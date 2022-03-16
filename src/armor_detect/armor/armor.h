/**
 * @file armor.h
 * @brief 装甲板类头文件
 * @details 定义存储装甲板对象各种信息的装甲板类
 * @author 陆展
 * @version 2021 Season
 * @update 李昊天、王灿
 * @email 1178272817@qq.com
 * @date 2020-12-31
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */

#ifndef ARMOR_H
#define ARMOR_H

#include <opencv2/opencv.hpp>
#include "base.h"
#include "types.h"

/**
 * @brief 装甲板类
 * 用于存储装甲板对象的各种参数并对其进行操作，部分属性不进行封装, 降低getter、setter带来的开销
 */
class Armor {
public:

    ///// 伽马矫正常系数
    static double GAMMA_C;
    //
    ///// 伽马矫正变量
    static double GAMMA_G;
    //
    ///// 伽马矫正灰度阈值
    static int GAMMA_THRES;

    /// 指示装甲板对象是否有效的布尔变量: 
    /// 只经过默认构造函数操作的装甲板对象是无效的, 经过自定义构造函数操作的装甲板对象是有效的
    bool is_valid;

    /// 装甲板打击优先级, 优先级越高越优先击打
    int priority;

    /// 装甲板数字编号的图像
    cv::Mat number_img;

    /// 装甲板颜色
    int color;

    /// 贴合装甲板区域的旋转矩形
    cv::RotatedRect rotated_rect;

    /// 包围装甲板区域的正常矩形, 用于跟踪
    //cv::Rect2d rect;

    /// 误差得分, 越低越好
    double score;

private:
    /// 装甲板数字编号
    int number;

    /// 旋转矩形宽度放大倍率, 尽可能保证截取数字区域的全部特征
    constexpr static double ROTATEDRECT_WIDTH_RATE = 1.0;

    /// 旋转矩形高度放大倍率, 尽可能保证截取数字区域的全部特征
    constexpr static double ROTATEDRECT_HEIGHT_RATE = 2.0;

public:
    /**
     * @brief 默认构造函数
     */
    Armor();

    /**
     * @brief 构造函数
     * @detail 对旋转矩形框出的装甲板图像进行透视变换，提取数字图像
     * 
     * @param src 源图像
     * @param rotated_rect 贴合装甲板的旋转矩形
     * @param color 敌方颜色
     * @param score 误差得分
     */
    Armor(const cv::Mat &src,
          const cv::RotatedRect &rotated_rect,
          const int color,
          double score, cv::Point2f vertices[4]);


    /**
     * @brief 默认析构函数
     */
    ~Armor();

    cv::Rect rect() const;

    /**
     * @brief 获取装甲板数字编号
     *
     * @return 装甲板数字编号(常量）
     */
    int getNumber() const;

    /**
     * @brief 设置装甲板数字编号, 同时更新优先级
     *
     * @param number 装甲板数字编号
     */
    void setNumber(int number);

    /**
     * @brief 自定义排序函数, 按误差得分升序排列装甲板对象
     *
     * @param a 装甲板对象1（在该函数中不可修改）
     * @param b 装甲板对象2（在该函数中不可修改）
     * @return 返回表示排序结果的布尔值
     *   @retval true 表示a排在前面
     *   @retval false 表示b排在前面
     */
    static bool scoreComparator(const Armor &a, const Armor &b);

    /**
     * @brief 自定义排序函数, 按打击优先级得分降序排列装甲板对象
     *
     * @param a 装甲板对象1（在该函数中不可修改）
     * @param b 装甲板对象2（在该函数中不可修改）
     * @return 返回表示排序结果的布尔值
     *   @retval true 表示a排在前面
     *   @retval false 表示b排在前面
     */
    static bool priorityComparator(const Armor &a, const Armor &b);

    /**
     * @brief 赋值运算符重载函数
     * @detail 因为定义了两个const成员变量, 重载赋值运算符，
     * 将传入的已有装甲板对象的各项参数赋给另一个装甲板对象，使两个装甲板对象完全相同
     * 
     * @param armor 将要赋值的对象
     * @return 赋值后的对象
     */
    Armor &operator=(const Armor &armor);

private:

    /**
     * @brief Gamma矫正增强数字显示效果, 伽马矫正公式:https://www.latexlive.com/#SV8lN0JvdXQlN0Q9KCU1Q2ZyYWMlN0JJXyU3QmluJTdEJTdEJTdCYyU3RCklNUUlN0IlNUNmcmFjJTdCMSU3RCU3QmclN0QlN0Q=
     *
     * @param src 源图像
     * @param dst 矫正后的图像
     * @param gammaG 矫正变量, 大于1以后, 越大则对低灰度值的增强越明显
     * @param gammaC 常系数, 通常为1
     */
    static void gammaCorrect(cv::Mat &src, cv::Mat &dst, double gammaG = 2.2, double gammaC = 1.0);
};

#endif // ARMOR_H
