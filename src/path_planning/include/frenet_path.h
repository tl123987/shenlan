/**
 * @Author: YunKai Xia
 * @Date:   2022-07-10 22:28:51
 * @Last Modified by:   YunKai Xia
 * @Last Modified time: 2022-07-10 22:37:25
 */

// Note: Thanks to TAI Lei for this data structure，Mail: ltai@ust.hk

#ifndef _FRENET_PATH_H
#define _FRENET_PATH_H

#include <array>
#include <iostream>
#include <string>
#include <vector>
// #include "common.h"
#include "cpprobotics_types.h"

namespace shenlan
{

  class FrenetPath
  {
  public:
    float cd = 0.0;
    float cv = 0.0;
    float cf = 0.0;

    Vec_f t;     // time
    Vec_f d;     // lateral offset
    Vec_f d_d;   // lateral speed
    Vec_f d_dd;  // 横向加速度
    Vec_f d_ddd; // 横向加加速度
    Vec_f s;     // s 沿样条线的位置
    Vec_f s_d;   // 速度
    Vec_f s_dd;  // s 加速度
    Vec_f s_ddd; // s jerk

    Vec_f x;   // x 位置
    Vec_f y;   // y position
    Vec_f yaw; // yaw in rad
    Vec_f ds;  // speed
    Vec_f c;   // curvature

    float max_speed;
    float max_accel;
    float max_curvature;
  };

  using Vec_Path = std::vector<FrenetPath>;

} // namespace shenlan
#endif
