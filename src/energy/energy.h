/**
 * @file 
 * @brief lz记得加说明！
 * @details 
 * @author 
 * @version 2021 Season
 * @update 
 * @email 
 * @date 
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */

#ifndef ENERGY_H
#define ENERGY_H

#include <vector>
#include <queue>
#include <opencv2/opencv.hpp>
#include <utility>
#include "types.h"


typedef enum {
    DIR_DEFAULT = 0,
    DIR_CW,
    DIR_CCW,
    DIR_STATIC
} Direction;

//typedef enum {
//    MODE_DEFAULT = 0,
//    MODE_SMALL,
//    MODE_BIG,
//    MODE_RANDOM,
//} EnergyMode;

class Energy {
public:
    bool shoot;
    bool isLoseAllTargets;
    bool isCalibrated;
private:

    //常量定义
    float EXP;
    float RADIUS;

    //能量板范围
    int MIN_ENERGY_AREA;
    int MAX_ENERGY_AREA;
    float MIN_ENERGY_RATIO;
    float MAX_ENERGY_RATIO;
    //箭头面积范围
    int MIN_FAN_AREA;
    int MAX_FAN_AREA;
    float MIN_FAN_RATIO;
    float MAX_FAN_RATIO;

    /// 中心R的面积范围
    int MIN_R_AREA;
    int MAX_R_AREA;

    /// PNP解算 能量板实际尺寸
    std::pair<float, float> REAL_ENERGY_SIZE;
    std::pair<float, float> REAL_R_SIZE;
    std::pair<double, double> abs_angle;
    float ENERGY_HALF_LENGTH;
    float ENERGY_HALF_WIDTH;

    /// PNP解算 R的实际尺寸
    float R_HALF_LENGTH;
    float R_HALF_WIDTH;

    const int QUEUE_SIZE = 100;
    //变量部分
    std::vector<cv::Point3f> object_Points;     //世界坐标系下的坐标
    std::vector<cv::Point2f> image_Points;      //图像坐标系下的坐标
    cv::Mat rotated_vector;                     //旋转向量
    cv::Mat translation_matrix;                 //平移矩阵
    cv::Mat CAMERA_MATRIX;                      //相机内参矩阵
    cv::Mat DISTCOEFFS;                         //相机畸变参数
    std::pair<double, double> ptz_angle;
    cv::Mat src;
    cv::Mat bin;
    //EnergyMode mode;

    cv::Size_<float> CENTER_ROI = cv::Size_<float>(40.0f, 40.0f);

    cv::RotatedRect target_energy;                      //当前帧能量板的最小包围矩形
    cv::RotatedRect predicted_energy;                   //预测的能量板的最小包围矩形
    cv::RotatedRect target_fan;                       //当前帧扇叶
    cv::RotatedRect target_R;
    bool isFindEnergy;                                  //标志位：是否已找到当前帧中的能量板
    bool isFindFan;                                   //标志位：是否已找到当前帧中的箭头
    bool isFindCenterR;                                 //标志位：是否已找到风车中心R,每次激活能量机关只需要识别一次即可
    std::vector<cv::Point> energy_centers;              //箭头中心点坐标队列
    std::vector<cv::Point> fan_centers;               //箭头中心点坐标队列
    cv::Point circle_center;                            //拟合圆圆心坐标
    cv::Point raw_center;                           //当前帧圆心坐标
    cv::Point current_center;
    double circle_radius;                               //拟合圆的半径

    //预测
    std::vector<double> angle_array;
    const float ANGLE_OFFSET = 40.0f;
    Direction direction;

public:
    double getOriginPtzYaw() const;

    double getOriginPtzPitch() const;

private:
    /// pitch偏角
    double origin_ptz_pitch;
    /// yaw偏角
    double origin_ptz_yaw;
public:
    void setOriginPtzPitch(double originPtzPitch);

    void setOriginPtzYaw(double originPtzYaw);

public:
    Energy();

    ~Energy();

    void init(const cv::FileStorage &file_storage);

    bool run(cv::Mat &current_frame, int color, Target &target);


private:

    void preprocess(int color);

    bool findEnergy();

    bool findFan();

    bool findArrow();

    bool findR();

    bool match(const cv::RotatedRect &candidate_fan);

    void updateQueue();

    /**
     * 根据向量估计中心点R的位置
     */
    void estimateRPosition();

    void automaticCentering();


    bool calculate();

    float toPolarCoordinates(const cv::Point &temp_point, const cv::Point &origin_point);     //计算极坐标系下的角度
    void judgeDirection();                              //判断方向
    void predicting();                                  //预测位置
    bool solveDirection();

    void solveRealPostiton(const cv::RotatedRect &aim, const std::pair<float, float> &REAL_SIZE);
};

#endif // ENERGY_H