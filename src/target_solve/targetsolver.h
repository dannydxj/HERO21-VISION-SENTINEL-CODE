/**
 * @file targetsolver.h
 * @brief PnP解算
 * @details 得到相机坐标系和云台坐标系下的坐标
 * @authors 侯典泳，陆展
 * @version 2021 Season
 * @update 赵泊媛
 * @email 819646960@qq.com
 * @date 2020-09-09
 * @license 2021 HITWH HERO-Robomaster Group
 */

#ifndef TARGETSOLVER_H
#define TARGETSOLVER_H

#include <opencv2/opencv.hpp>
#include "armor_detect/armor/armor.h"

/**
 * @brief 目标姿态解算类
 * 解算出目标装甲板在云台坐标系中的x,y,z坐标
 */
class TargetSolver
{
private:
    /// 大装甲板实际宽度的一半, 单位为毫米, 请以实际测量为准
    constexpr static double HALF_BIG_ARMOR_WIDTH = 115.04;

    /// 大装甲板实际高度的一半, 单位为毫米, 请以实际测量为准
    constexpr static double HALF_BIG_ARMOR_HEIGHT = 55.08;

    /// 小装甲板实际宽度的一半, 单位为毫米, 请以实际测量为准
    constexpr static double HALF_SMALL_ARMOR_WIDTH = 70.30;

    /// 小装甲板实际高度的一半, 单位为毫米, 请以实际测量为准
    constexpr static double HALF_SMALL_ARMOR_HEIGHT = 27.30;

    /// x轴补偿值
    constexpr static double X_OFFSET = 0.0;

    /// y轴补偿值, 云台向上
    constexpr static double Y_OFFSET = -51.4469;

    /// z轴补偿值
    constexpr static double Z_OFFSET = 140.7033;

    /// 设置解算模式，计算速度: P3P > EPNP > Iterative
    constexpr static int ALGORITHM = cv::SOLVEPNP_EPNP;

    /// 相机内参矩阵, 常量
    cv::Mat CAMERA_MATRIX;

    /// 相机畸变系数, 常量
    cv::Mat DISTORTION_COEFF;

    /// PNP解算结果旋转矩阵
    cv::Mat rotate_mat;

    /// PNP解算平移矩阵
    cv::Mat trans_mat;

public:
    /**
     * @brief 默认构造函数
     */
    TargetSolver();

    /**
     * @brief 默认析构函数
     */
    ~TargetSolver();

    /**
     * @brief 初始化函数
     * 
     * @param file_storage 配置文件
     */
    void init(const cv::FileStorage &file_storage);

    /**
     * @brief 弹道解算核心函数
     * 
     * @param armor 目标装甲板
     * @param target 云台坐标
     */
    void run(const Armor &armor, Target &target);

private:
    /**
     * @brief 相机坐标解算
     * 
     * @param rect 目标装甲板旋转矩形
     * @param is_big_armor 是否是大装甲板, 大小装甲板拥有不同的实际尺寸
     */
    void solvePnP4Points(const cv::RotatedRect &rect, bool is_big_armor);

    /**
     * @brief 相机坐标系转换成云台坐标系
     * 
     * @param camera_position 输入相机坐标
     * @param ptz_position 输出云台坐标
     */
    static void camera2ptzTransform(const cv::Mat &camera_position, Target &ptz_position);
};

#endif // TARGETSOLVER_H
