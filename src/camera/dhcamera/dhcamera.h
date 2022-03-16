/**
 * @file dhcamera.h
 * @brief 大恒相机驱动
 * @details 相机驱动文件及配置，设置相机白平衡,帧率曝光等属性及图像特征
 * @authors 李昊天
 * @version 2021 Season
 * @update 王灿
 * @email 1178272817@qq.com
 * @date 2020-12-31
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */

#ifndef DHCAMERA_H
#define DHCAMERA_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <exception>
#include <mutex>

#include "camera.h"
#include "GxIAPI.h"
#include "DxImageProc.h"

/**
 * @brief 大恒相机类
　*/
class DHCamera : public Camera
{
private:
    /// 设备句柄
    GX_DEV_HANDLE m_hDevice = NULL;

    /// RGB图像数据，转为opencv的Mat形式
    uchar *m_pBufferRGB;

    ///　获取的加载图像大小
    int64_t m_nPayLoadSize;
    int64_t m_nPixelColorFilter;

    /// 相机状态
    bool is_open;

    /// 线程锁
    // std::mutex image_buffer_mutex;

public:
    /**
     * @brief　默认构造函数
     */
    DHCamera();

    /**
     * @brief　默认析构函数
     */
    ~DHCamera();

    /**
     * @brief　摄像头打开函数
     * @param frame_width 图像长度
     * @param frame_height 图像宽度
     * @param exposure_time 曝光时间
     * @param frame_speed 帧率
     * @param gamma gamma值
     */
    void open(int frame_width = 1280,
              int frame_height = 1024,
              int exposure_time = 100,
              double frame_speed = 210,
              double gamma = 1, int contrast = 100);

    /**
     * @brief　图像获取函数
     * @param image 图像
     */
    void getImage(cv::Mat &image);

    /**
     * @brief　设备开启函数，判断
     * @return true/false状态
     */
    bool isOpen();

    /**
     * @brief　设备关闭函数
     */
    void close();
};

#endif // DHCAMERA_H
