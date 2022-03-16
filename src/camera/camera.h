/**
 * @file camera.h
 * @brief 相机驱动
 * @details 相机驱动文件及配置，设置相机白平衡,帧率曝光等属性及图像特征
 * @authors 王灿
 * @version 2021 Season
 * @update 
 * @email 1178272817@qq.com
 * @date 2020-12-31
 * @license 2021 HITWH HERO-Robomaster Group
 */


#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <exception>
#include <mutex>

/**
 * @brief 相机类
　*/
class Camera
{
private:
public:
    /**
     * @brief　默认构造函数
     */
    Camera() {}

    /**
     * @brief　默认析构函数
     */
    ~Camera() {}

    /**
     * @brief　摄像头打开函数
     * @param frame_width 图像长度
     * @param frame_height 图像宽度
     * @param exposure_time 曝光时间
     * @param frame_speed 帧率
     * @param gamma gamma值
     * @param contrast 对比度,
     */
    virtual void open(int frame_width,
                      int frame_height,
                      int exposure_time,
                      double frame_speed = 210,
                      double gamma = 1, int contrast = 100) = 0;

    /**
     * @brief 返回相机是否已开启
     *
     * @return bool值，表示相机是否开启
     *     @retval true 相机正常开启
     *     @retval false 相机未开启
     */
    virtual bool isOpen() = 0;

    /**
     * @brief 通过相机获取单张Mat类型图片
     * @param image的引用，保存获取到的图像
     */
    virtual void getImage(cv::Mat &image) = 0;

    /**
     * @brief　关闭相机函数
     */
    virtual void close() = 0;
};

// Camera::Camera(){}
// Camera::~Camera(){}

/**
 * @brief 相机异常类
 * 相机在比赛中可能出现连接不稳定等异常，写一异常类帮助相机重新连接；
 * 同时防止因为相机异常造成的程序崩溃
 */
class CameraException : public std::exception
{
private:
    /// 异常信息字符串
    std::string e_what;

public:
    CameraException() = default;

    /**
     * @brief 自定义构造函数, 需要给出异常信息
     *
     * @param error 异常描述信息
     */
    CameraException(const std::string &error) : e_what(error) {}

    ~CameraException() throw() {}

    /**
     * @brief 异常规格说明：不抛出异常
     *
     * @return 异常信息字符串
     * @note 该函数为std::exception类中的覆盖
     */
    virtual const char *what() const throw()
    {
        return e_what.c_str();
    }
};

#endif // CAMERA_H
