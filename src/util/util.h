/**
 * @file util.h
 * @brief 工具类
 * @details 存放一些通用函数和常量
 * @authors 侯典泳，陆展
 * @version 2021 Season
 * @update 赵泊媛
 * @email 819646960@qq.com
 * @date 2020-09-09
 * @license 2021 HITWH HERO-Robomaster Group
 */

#ifndef UTIL_H
#define UTIL_H

#include <opencv2/opencv.hpp>

/**
 * @brief 工具类
　* 存放常计算量，坐标转换工具，欧式距离计算工具等
 */
class Util {
public:
    /// 数学常量, π
    constexpr static double PI = 3.1415926;

    /// 物理常量, 重力加速度
    constexpr static double G = 9.7988;

    // 浮点数精度控制
    constexpr static double EXP = 0.1;
public:
    /**
     * @brief 将云台坐标系坐标转化为较绝对坐标系下的坐标
     * @param x 三维坐标x
     * @param y 三维坐标y
     * @param z 三维坐标z
     * @param ptz_pitch 云台绝对pitch轴角度
     * @param ptz_yaw 云台绝对yaw轴角度
     */
    static void coordinate_transformation(double &x, double &y, double &z,
                                          double ptz_pitch, double ptz_yaw);

    /**
     * @brief 将较绝对坐标系下的坐标转化为云台坐标系下的坐标
     * @param x 三维坐标x
     * @param y 三维坐标y
     * @param z 三维坐标z
     * @param ptz_pitch 云台绝对pitch轴角度
     * @param ptz_yaw 云台绝对yaw轴角度
     */
    static void anti_coordinate_transformation(double &x, double &y, double &z,
                                               double ptz_pitch, double ptz_yaw);

    /**
     * @brief 获取三维点的欧式距离
     * @param point1 点1
     * @param point2 点2
     * @return 两点欧氏距离
     */
    static double distance(const cv::Point3d &point1, const cv::Point3d &point2);

    /**
     * @brief 获取二维点的欧氏距离
     * @param point1 点1
     * @param point2 点2
     * @return 两点欧氏距离
     */
    static double distance(const cv::Point2d &point1, const cv::Point2d &point2);

    /**
     * @brief 获取子弹飞行时间
     * @param x 子弹出射点与目标点水平距离
     * @param y 子弹出射点与目标点垂直距离
     * @param v 子弹发射速度
     * @param ptz_pitch 云台绝对pitch轴角度
     * @return 子弹飞行时间
     */
    static double get_flight_time(double x, double y, double v, double ptz_pitch);

    static cv::Mat convertTo3Channels(const cv::Mat &binImg);

    /**
     * 判断浮点数是否约等于0
     * @param x 浮点数
     * @return 该浮点数是否约等于0
     */
    static bool equalZero(double x);
};

#endif  // UTIL_H
