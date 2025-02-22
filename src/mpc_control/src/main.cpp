#include "mpc_controller.h"

// Input
VehicleState vehicle_state_;

using namespace std;

bool first_record_ = false;
double V_set_ = 10.0;

double wheelbase_ = 1.580;
double car_length_ = 2.875;

double PointDistance(const TrajectoryPoint &point, const double x,
                     const double y)
{
  const double dx = point.x - x;
  const double dy = point.y - y;
  return sqrt(dx * dx + dy * dy);
}

void odomCallback(const nav_msgs::Odometry::ConstPtr &msg)
{
  if (!first_record_)
    first_record_ = true;
  // ROS_ERROR("I heard: [%f]", msg->pose.pose.position.x);

  vehicle_state_.vx = msg->twist.twist.linear.x;
  vehicle_state_.vy = msg->twist.twist.linear.y;

  // 将orientation(四元数)转换为欧拉角(roll, pitch, yaw)
  tf::Quaternion q;
  tf::quaternionMsgToTF(msg->pose.pose.orientation, q);
  tf::Matrix3x3(q).getRPY(vehicle_state_.roll, vehicle_state_.pitch, vehicle_state_.yaw);

  vehicle_state_.heading = vehicle_state_.yaw; // pose.orientation是四元数
  // cout << "vehicle_state_.heading: " << vehicle_state_.heading << endl;

  // 将位置转移到前车轮的中心点
  //vehicle_state_.x = msg->pose.pose.position.x + std::cos(vehicle_state_.heading) * 0.5 * car_length_;
  //vehicle_state_.y = msg->pose.pose.position.y + std::sin(vehicle_state_.heading) * 0.5 * wheelbase_;
  

  vehicle_state_.x = msg->pose.pose.position.x; //
  vehicle_state_.y = msg->pose.pose.position.y;

  // cout << "vehicle_state_.heading: " << vehicle_state_.heading << endl;
  vehicle_state_.velocity =
      std::sqrt(msg->twist.twist.linear.x * msg->twist.twist.linear.x +
                msg->twist.twist.linear.y * msg->twist.twist.linear.y);
  vehicle_state_.cur_v_err = vehicle_state_.velocity - V_set_;
  vehicle_state_.angular_velocity =
      std::sqrt(msg->twist.twist.angular.x * msg->twist.twist.angular.x +
                msg->twist.twist.angular.y * msg->twist.twist.angular.y);
  vehicle_state_.acceleration = 0.0;
}

int main(int argc, char **argv)
{
  // Read the reference_line txt
  std::ifstream infile;
  infile.open(
      "/media/q/Elenments/5ubuntu/carla/shenlan/src/stanley_control/data/reference_line.txt"); // 将文件流对象与文件连接起来
  assert(infile.is_open());                                                                    // 若失败,则输出错误消息,并终止程序运行

  std::vector<std::pair<double, double>> xy_points;
  std::string s;
  std::string x;
  std::string y;

  while (getline(infile, s))
  {
    std::stringstream word(s);
    word >> x;
    word >> y;
    double pt_x = std::atof(x.c_str());
    double pt_y = std::atof(y.c_str());
    xy_points.push_back(std::make_pair(pt_x, pt_y));
  }
  infile.close();

  // Construct the reference_line path profile
  std::vector<double> headings;
  std::vector<double> accumulated_s;
  std::vector<double> kappas;
  std::vector<double> dkappas;
  std::unique_ptr<shenlan::control::ReferenceLine> reference_line =
      std::make_unique<shenlan::control::ReferenceLine>(xy_points);
  reference_line->ComputePathProfile(&headings, &accumulated_s, &kappas,
                                     &dkappas);

  for (size_t i = 0; i < headings.size(); i++)
  {
    std::cout << "pt " << setw(3) << i
              << " heading: " << setw(10) << headings[i]
              << " acc_s: " << setw(7) << accumulated_s[i]
              << " kappa: " << setw(12) << kappas[i]
              << " dkappas: " << setw(8) << dkappas[i] << std::endl;
  }
  std::cout << "-------------------------------------" << std::endl;
  std::cout << std::endl;

  // Construct the planning trajectory
  TrajectoryData planning_published_trajectory;
  for (size_t i = 0; i < headings.size(); i++)
  {
    TrajectoryPoint trajectory_pt;
    trajectory_pt.x = xy_points[i].first;
    trajectory_pt.y = xy_points[i].second;
    trajectory_pt.v = V_set_;
    trajectory_pt.a = 0.0;
    trajectory_pt.heading = headings[i];
    trajectory_pt.kappa = kappas[i];

    planning_published_trajectory.trajectory_points.push_back(trajectory_pt);
  }

  TrajectoryPoint goal_point = planning_published_trajectory.trajectory_points.back();

  // Initialize ros node
  ros::init(argc, argv, "control_pub");
  ros::NodeHandle nh;
  ROS_ERROR("init !");

  ros::Subscriber sub = nh.subscribe("/carla/ego_vehicle/odometry", 10, odomCallback);
  ros::Publisher control_pub =
      nh.advertise<carla_msgs::CarlaEgoVehicleControl>("/carla/ego_vehicle/vehicle_control_cmd", 1000);
  ros::Publisher path_pub =
      nh.advertise<nav_msgs::Path>("Town02_refernce_path", 1000);

  nav_msgs::Path reference_path;

  reference_path.header.stamp = ros::Time::now();
  reference_path.header.frame_id = "map";

  for (size_t i = 0; i < headings.size(); i++)
  {
    geometry_msgs::PoseStamped refpath_pose;
    TrajectoryPoint trajectory_pt = planning_published_trajectory.trajectory_points[i];
    refpath_pose.pose.position.x = trajectory_pt.x;
    refpath_pose.pose.position.y = trajectory_pt.y;
    refpath_pose.pose.position.z = 0.0;

    geometry_msgs::Quaternion pt_quat = tf::createQuaternionMsgFromYaw(trajectory_pt.heading);
    refpath_pose.pose.orientation.w = pt_quat.w;
    refpath_pose.pose.orientation.x = pt_quat.x;
    refpath_pose.pose.orientation.y = pt_quat.y;
    refpath_pose.pose.orientation.z = pt_quat.z;

    refpath_pose.header.frame_id = "map";
    refpath_pose.header.stamp = ros::Time::now();

    reference_path.poses.push_back(refpath_pose);
  }

  // Stanley control part
  ControlCmd cmd;
  std::unique_ptr<shenlan::control::MPCController> stanley_controller = std::make_unique<shenlan::control::MPCController>();
  stanley_controller->Init();

  ros::Rate loop_rate(100);
  while (ros::ok())
  {
    if (first_record_)
    {
      // 距离终点0.5m停止
      if (PointDistance(goal_point, vehicle_state_.x, vehicle_state_.y) < 0.5)
      {
        V_set_ = 0;
      }
      stanley_controller->ComputeControlCommand(vehicle_state_, planning_published_trajectory, cmd);

      // control_cmd 的其他数据
      carla_msgs::CarlaEgoVehicleControl control_cmd;
      control_cmd.header.stamp = ros::Time::now();
      control_cmd.reverse = false;
      control_cmd.manual_gear_shift = false;
      control_cmd.hand_brake = false;
      control_cmd.gear = 0;

      double acc_cmd = cmd.acc;

      if (acc_cmd >= 0)
      {
        control_cmd.throttle = min(1.0, acc_cmd);
        control_cmd.brake = 0.0;
      }
      else
      {
        control_cmd.throttle = 0.0;
        control_cmd.brake = min(1.0, -acc_cmd);
      }

      if (V_set_ == 0)
      {
        control_cmd.throttle = 0.0;
      }

      cout << "vehical_state_.cur_v_err: " << vehicle_state_.cur_v_err << endl;
      control_cmd.steer = -cmd.steer_target;//!!要加上负号 ？？？？ 也没有做度数转换
      cout << "control_cmd.target_wheel_angle: " << control_cmd.steer << endl;
      control_pub.publish(control_cmd);
    }
    
    path_pub.publish(reference_path);
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}
