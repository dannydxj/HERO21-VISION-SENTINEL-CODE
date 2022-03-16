/**
 * @file anglesolver.h
 * @brief 弹道解算头文件
 * @details 根据目标三维坐标计算云台所需角度增量：
            先将目标在云台坐标系（z轴与摄像机光轴重合，指向镜头外侧；y轴与z轴在同一竖直平面内并垂直z轴向下；
            x轴垂直yoz平面向右；该坐标系为右手坐标系）下三维坐标转换为水平坐标系（z轴水平向前；y轴竖直向下；
            x轴水平向右）下坐标；再利用物体平抛运动模型计算得云台要击中目标需要转动到的pitch轴相对地面绝对角度，
            减去云台原来相对地面的pitch轴绝对角度即可得到pitch轴所需角度增量；
            yaw轴所需角度增量可直接由转换后坐标计算得。
 * @author 侯典泳
 * @version 2021 Season
 * @update 曾婧（添加注释）
 * @email 1905161936@qq.com
 * @date 2021-09-07
 * @license Copyright© 2020 HITwh HERO-RoboMaster Group
 */

#ifndef ANGLESOLVER_H
#define ANGLESOLVER_H

#include "util/types.h"

/**
 * @brief 弹道解算类
 * 运用物体平抛运动模型，根据目标在云台坐标系下的三维坐标，计算云台要打击到目标所需转动角度（pitch轴，yaw轴）
 */
class AngleSolver {
private:
    /// yaw轴补偿角度
    constexpr static double YAW_OFFSET = 0.0;

    /// pitch轴补偿角度
    constexpr static double PITCH_OFFSET = 0.0;

public:
    /**
     * @brief Anglesolver默认构造函数
     */
    AngleSolver();

    /**
     * @brief Anglesolver默认析构函数
     */
    ~AngleSolver();

    /**
     * @brief Anglesolver初始化函数
     */
    void init();

    /**
     * @brief 角度解算核心函数, 输入目标在云台坐标系下的三维坐标、子弹速度
              和当前云台相对地面pitch轴角度, 输出要打击到目标云台yaw和pitch所需角度增量
     * @detail 先通过坐标系旋转将目标在云台坐标系下三维坐标转换为水平坐标系下三维坐标，
               再根据平抛运动公式计算出pitch轴角度增量，根据坐标直接计算出yaw轴角度增量
     *
     * @param target 云台三维坐标
     * @param v 子弹速度
     * @param ptz_pitch 当前云台相对地面pitch轴角度
     * @param yaw 解算得到的yaw轴角度增量
     * @param pitch 解算得到的pitch轴角度增量
     * @note 该代码正确性已得到多次验证，后续绝对不允许修改！！！
     * @attention 参数不可为NULL
     */
    static void run(const Target &target, double v, double ptz_pitch, double &yaw, double &pitch);

    /**
     * @brief 飞行延迟解算函数（主要用于预测）, 输入目标在云台坐标系下的三维坐标、子弹速度
               和当前云台相对地面pitch轴角度, 返回弹丸从射出枪口到击中目标所需飞行延迟时间
     * @detail 步骤类似角度解算，先通过坐标系旋转将目标在云台坐标系下三维坐标转换为
               水平坐标系下三维坐标，再根据平抛运动公式计算出弹丸从出射到击中目标的飞行延迟
     *
     * @param target 云台三维坐标
     * @param v 子弹速度
     * @param ptz_pitch 当前云台相对地面pitch轴角度
     * @return 返回double类型浮点数：弹丸从射出枪口到击中目标所需飞行延迟时间
     * @attention 参数不可为NULL！
     */
    static double get_flight_time(const Target &target, double v, double ptz_pitch);

private:
    /**
     * @brief 抛物线运动解算函数，输入子弹发射点与目标的水平距离、竖直距离、子弹速度、
              当前云台相对地面pitch轴绝对角度，输出云台所需pitch轴角度增量，
              返回表示解算结果是否有效的布尔值
     * @detail 在子弹出射点与目标点所在的竖直平面内根据平抛运动公式计算pitch轴角度增量
     *
     * @param x 子弹发射点与目标的水平距离（x轴距离）
     * @param y 子弹发射点与目标的竖直距离（y轴距离）
     * @param v 子弹速度
     * @param ptz_pitch 当前云台相对地面pitch轴角度
     * @param theta pitch轴角度增量（解算结果）
     * @return 返回表示解算结果是否有效的布尔值
     *   @retval true 表示解算结果有效
     *   @retval false 表示解算结果无效
     * @note 该代码正确性已被多次验证，不允许后续再修改！
     * @attention 参数不可为NULL！
     */
    static bool parabolaSolve(double x, double y, double v, double ptz_pitch, double &theta);
};

#endif // ANGLESOLVER_H
