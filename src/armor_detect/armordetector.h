/**
 * @file armordetector.h
 * @brief 装甲板检测类头文件
 * @details 从图像中找出装甲板
 * @author 侯典泳
 * @version 2021 Season
 * @update 李昊天、王灿
 * @email 1178272817@qq.com
 * @date 2020-12-31
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */

#ifndef ARMORDETECTOR_H
#define ARMORDETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

#include "armor/armor.h"
#include "base.h"
#include "classifier/classifier.h"

#ifdef COMPILE_WITH_CUDA
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#endif

class Debugger;

/**
 * @brief 装甲板检测类
 */
class ArmorDetector
{
public:
    /// 预处理得到的灰度图像, 用于寻找灯条
    cv::Mat processed_image;

    /// 调试时用于输出调试信息的友元类
    friend class Debugger;

    const cv::Rect &getRoiRect() const;

    void setRoiRect(const cv::Rect &roiRect);

private:

    /// 是否使用ROI
    int ROI_ENABLE=1;

    /// ROI区域
    cv::Rect roi_rect;

    /// 图像帧宽度
    int FRAME_WIDTH;

    /// 图像帧高度
    int FRAME_HEIGHT;

private:
    constexpr static double ratio = 1.5;
    /// ROI图片
    cv::Mat roi_image;

    /// 轮廓被视为灯条的最小面积
    double MIN_LIGHTBAR_AREA;

    /// 装甲板宽高比下限
    double MIN_ASPECT_RATIO;

    /// 装甲板宽高比上限
    double MAX_ASPECT_RATIO;

    /// 灯条长度比下限
    double MIN_LENGTH_RATIO;

    /// 灯条长度比上限
    double MAX_LENGTH_RATIO;

    /// 灯条倾斜度之差上限
    double MAX_LIGHTBAR_DELTA;

    /// 装甲板倾斜度上限
    double MAX_ARMOR_ANGLE;

    /// 装甲板和灯条倾斜度之差上限
    double MAX_ARMOR_LIGHTBAR_DELTA;

    /// 每帧送进分类器的候选装甲板的最大数量
    int MAX_CANDIDATE_NUM = 1;

    /// 源图像灰度阈值
    int GREY_THRES;

    /// 通道相减图像灰度阈值
    int SUBTRACT_THRES;

    /// 算子核大小
    int KERNEL_SIZE;

#ifdef DISTORTION_CORRECT
    Mat camera_matrix;
    Mat distortion_coeff;
#endif // DISTORTION_CORRECT

#ifndef COMPILE_WITH_CUDA
    cv::Mat kernel;
#else
    cv::Ptr<cv::cuda::Filter> kernel;
#endif // COMPILE_WITH_CUDA

    /// 数字分类器
    Classifier classifier;

public:
    /**
     * @brief 默认构造函数
     */
    ArmorDetector();

    /**
     * @brief 默认析构函数
     */
    ~ArmorDetector();

    /**
     * @brief 初始化函数
     *
     * @param file_storage 参数配置文件
     */
    void init(const cv::FileStorage &file_storage);

    /**
     * @brief 执行装甲板检测的核心函数
     *
     * @param src 源图像
     * @param enemy_color 敌方颜色
     * @param target_armor 存储最终找到的目标装甲板
     * @return 是否找到装甲板
     *   @retval true 找到装甲板 
     *   @retval false 没有找到装甲板
     */
    bool run(const cv::Mat &src, const int enemy_color, Armor &target_armor);

private:
    /**
     * @brief 根据对方装甲板颜色, 将图像预处理成二值图像
     *
     * @param src 源图像
     * @param enemy_color 敌方颜色
     */
    void Preprocess(const cv::Mat &src, const int enemy_color);

    /**
     * @brief 找出所有灯条, 匹配成装甲板
     *
     * @param enemy_color 敌方颜色
     * @param armors 存放找到的候选装甲板
     */
    void findTarget(const int enemy_color, std::vector<Armor> &armors);

    /**
     * @brief 对灯条进行两两匹配, 根据一系列标准选出候选装甲板
     *
     * @param src 源图像
     * @param lightbars 灯条数组
     * @param enemy_color 敌方颜色
     * @param armors 存储找到的候选装甲板
     */
    void findArmors(std::vector<cv::RotatedRect> &lightbars, const int enemy_color,
                    std::vector<Armor> &armors);

    /**
     * @brief 从候选装甲板中选出最合适的目标装甲板, 最终按照优先级从高到低排列
     *
     * @param armors 候选装甲板
     */
    void selectTarget(std::vector<Armor> &armors);

    /**
     * @brief 统一灯条的旋转矩形的样式, height>width
     * 
     * @param rect 源旋转矩形
     */
    static void adjustLightBar(cv::RotatedRect &rect);

    /**
     * 统一装甲板的旋转矩形的样式, width>height
     * @param rect 源旋转矩形
     */
    static void adjustArmor(cv::RotatedRect &rect);

    void preventROIExceed(int &x, int &y, int &width, int &height);
};

#endif /// ARMORDETECTOR_H
