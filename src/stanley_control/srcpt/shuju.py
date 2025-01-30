#!/usr/bin/env python3
import rospy
from nav_msgs.msg import Path

# 打开文件用于保存数据
output_file = open("/media/q/Elenments/5ubuntu/carla/shenlan/src/stanley_control/data/1.txt", "w")
save_rate = 100  # 设置保存频率，单位为 Hz（每秒保存的次数）

def path_callback(data):
    rospy.loginfo(f"Received path with {len(data.poses)} waypoints.")
    for idx, pose in enumerate(data.poses):
        position = pose.pose.position
        # 写入文件
        output_file.write(f"{position.x}, {position.y}\n")
        rospy.loginfo(f"Waypoint {idx} saved: x={position.x}, y={position.y}")
    output_file.flush()  # 实时写入文件

if __name__ == "__main__":
    try:
        rospy.init_node("path_listener")
        rospy.Subscriber("/carla/ego_vehicle/waypoints", Path, path_callback)

        # 设置保存频率
        rate = rospy.Rate(save_rate)  # 设置为 1Hz，表示每秒保存一次
        while not rospy.is_shutdown():
            rate.sleep()  # 保持指定的频率
    except rospy.ROSInterruptException:
        rospy.loginfo("Node terminated.")
    finally:
        # 确保文件关闭
        output_file.close()
