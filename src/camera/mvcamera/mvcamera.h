/**
 * @file mvcamera.h
 * @brief MindVision相机驱动类头文件
 * @details 实现对工业摄像头的基本参数设定和初始化，并将当前帧的数据转化为opencv的Mat类型.
            具体可参考MindVision的官方linux SDK，下附SDK下载链接
 * @see http://www.mindvision.com.cn/rjxz/list_12.aspx?lcid=138
 * @author 王筱琰
 * @version 2021 Season
 * @update 李昊天
 * @email lcyxlihaotian@126.com
 * @date 2021-12-31
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */

#ifndef MVCAMERA_H
#define MVCAMERA_H

#include <cstring>
#include <exception>
#include <opencv2/core/core.hpp>

#include "camera.h"
#include "CameraApi.h"
#include "util/base.h"

/**
 * @brief MindVision相机驱动类
 * 实现对工业摄像头的基本参数设定和初始化，并将当前帧的数据转化为opencv的Mat类型
 */
class MVCamera : public Camera
{
private:
    /// 处理后图像输出的缓冲区
    unsigned char *g_pRgbBuffer;

    /// 相机的句柄
    int hCamera;

    /// 工业摄像头设备列表数组
    tSdkCameraDevInfo tCameraEnumList;

    /// 相机特性描述的结构体
    tSdkCameraCapbility tCapability;

    /// 图像的帧头信息
    tSdkFrameHead sFrameInfo;

    /// 指向图像的数据的缓冲区
    unsigned char *pbyBuffer;

    /// 相机工作状态
    bool is_open;

public:
    /**
     * @brief 构造函数
     */
    MVCamera();

    /**
     * @brief 析构函数，关闭相机
     */
    ~MVCamera();

    /**
     * @brief 打开相机, MV-SUA33GC-T的最大分辨率为640*480
     *
     * @param frame_width 图像宽度
     * @param frame_height 图像高度
     * @param exposure_time 曝光时间, 范围: [2, 20700]
     * @param frame_speed 帧率,<br>0: low <br> 1: normal <br> 2: high
     * @param gamma 伽马值, 保留暗部细节, 默认100
     * @param contrast 对比度, 默认100
     */
    void open(int frame_width = 1280,
              int frame_height = 1024,
              int exposure_time = 100,
              double frame_speed = 210,
              double gamma = 1, int contrast = 100);

    /**
     * @brief 返回相机是否已开启
     *
     * @return bool值，表示相机是否开启
     *     @retval true 相机正常开启
     *     @retval false 相机未开启
     */
    bool isOpen();

    /**
     * @brief 通过相机获取单张Mat类型图片
     * @overload
     * 
     * @param image的引用，保存获取到的图像
     */
    void getImage(cv::Mat &image);

    /**
     * @brief 关闭相机
     */
    void close();
};

#endif // MVCAMERA_H
