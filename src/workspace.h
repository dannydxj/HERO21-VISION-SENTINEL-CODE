/**
 * @file workspace.h
 * @brief 工作类头文件
 * @details 机器人核心工作类, 多线程工作, 完成图像接收, 图像处理, 通信功能
 * @author 王筱琰
 * @version 2021 Season
 * @update 李昊天、王灿
 * @email lcyxlihaotian@126.com
 * @date 2020-12-31
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <mutex>

#include "anglesolver.h"
#include "armor.h"
#include "armordetector.h"
#include "debugger.h"
#include "camera.h"
#include "dhcamera.h"
#include "mvcamera.h"
#include "serialport.h"
#include "cannode.h"
#include "targetsolver.h"
#include "energy.h"

/// 配置文件路径<br>
/// 开自启时需改为绝对路径
const static std::string PARAM_PATH = "../param/param.xml";

/**
 * @brief 工作类
 * 完成多线程, 图像接收, 图像处理, 通信功能
 */
class Workspace {
private:
    /// 图片缓冲队列大小
    constexpr static int MAX_IMAGE_BUFFER_SIZE = 10;

    /// 线程锁
    std::mutex image_buffer_mutex;

    /// 文件输出流
    std::ofstream outfile;

    /// 装甲板检测类对象
    ArmorDetector armor_detector;

    /// 能量机关类
    Energy energy;

    /// 坐标解算类对象
    TargetSolver target_solver;

    /// 角度解算类对象
    AngleSolver angle_solver;

    /// 相机对象
    Camera *camera = new DHCamera();
    // Camera *camera = nullptr;

    /// 串口和 CAN 对象, 和 MCU 通信
    SerialPort serial_port;
    CanNode can_node;

    ///图像缓冲区
    std::vector<cv::Mat> image_buffer;

    /// 当前图像
    cv::Mat image_original;

    ///目标装甲板
    Armor target_armor;

    /// 目标三维坐标
    Target target;

    /// MCU通信发送数据包
    SendPack send_pack;

    /// MCU通信接收数据包
    ReadPack read_pack;

    /// 是否显示图像
    int SHOW_IMAGE = 0;

    /// 是否显示滚动条<br>
    /// 必须在 SHOW_IMAGE=1 的情况下才可开启！
    int TRACKBAR = 0;

    /// 是否将调式信息打印在终端上
    int DEBUG_INFO = 0;

    /// 是否显示运行时间
    int RUNNING_TIME = 0;

    /// 敌方颜色：0代表从电控读；1是红色，2是蓝色
    int ENEMY_COLOR = COLOR_DEFAULT;

    /// 机器人工作模式：
    /// 0:从电控读<br>
    /// 1:一代自瞄<br>
    /// 2:二代自瞄<br>
    /// 3:小能量机关<br>
    /// 4:大能量机关<br>
    /// 5:英雄吊射<br>
    /// 6:工程取弹<br>
    int MODE = MODE_DEFAULT;

    /// 是否使用相机，是1否0
    /// ***注意：不使用相机时默认为使用视频传入***
    int USE_CAMERA = 1;

    /// 是否使用串口，1 是, 0 否
    int USE_SERIAL = 0;

    /// 是否使用串口，0 使用 CAN0, 1 使用 CAN1, 2 不使用 CAN
    int USE_CAN = 2;

    /// 是否在**运行代码的同时**保存视频，0否1是
    int SAVE_VIDEO = 1;

    /// 图像帧宽度
    int FRAME_WIDTH = 640;

    /// 图像帧高度
    int FRAME_HEIGHT = 480;

    /// 相机曝光值，不同厂家相机该参数不同
    int EXPOSURE_TIME = 400;

    /// 当前帧图片运算时间, 子弹发射延迟中要考虑进去
    float delay_time;

    /// 测试视频输入路径
    std::string VIDEO_PATH;

    /// 视频保存路径
    std::string VIDEO_SAVED_PATH;

public:
    /**
     * @brief 默认构造函数
     */
    Workspace();

    /**
     * @brief 默认析构函数
     */
    ~Workspace();

    /**
     * @brief 开启多线程
     */
    void run();

    friend void init();

private:
    /**
     * @brief 图像接收线程, 可以从图片和视频文件中接收, 也可以从工业相机中接收
     */
    void imageReceivingFunc();

    /**
     * @brief 图像处理线程, 完成对目标的检测, 跟踪, 预测, 角度解算, 向MCU发送数据包
     */
    void imageProcessingFunc();

    /**
     * @brief 通信线程, 读取MCU发送的数据包
     */
    void messageCommunicatingFunc();

    /**
     * @brief 打开串口
     */
    void openSerialPort();

    /**
     * @brief 设置 `read_pack` 和 `send_pack` 中的 `mode` 和 `enemy_color`
     * 
     */
    void setModeAndColor();

    /**
     * @brief 调试时显示图像 & 创建滚动条
     */
    void showImage();

    /**
     * @brief 自动匹配相机类型
     */
    void autoCamera();
};

#endif /// WORKSPACE_H
