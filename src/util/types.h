/**
 * @file types.h
 * @brief 自定义类型集合
 * @details 存放各种自定义类型, 包括通信协议数据帧, 机器人工作模式等
 * @authors 陆展
 * @version 2021 Season
 * @update 赵泊媛
 * @email 819646960@qq.com
 * @date 2020-09-09
 * @license 2021 HITWH HERO-Robomaster Group
 */

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <iostream>

/**
 * @brief 枚举敌方装甲板颜色
 * 调试的时候可能会指定颜色，比赛的时候电控发给我们颜色
 */
enum EnemyColor {
    /// 默认由电控控制
    COLOR_AUTO=0,
    /// 调试时固定红色
    COLOR_RED,
    /// 调式时固定蓝色
    COLOR_BLUE,



    /// 默认错误颜色
    COLOR_DEFAULT
};

/**
 * @brief 枚举模式
 * 比赛场上依据不同的需求所设定的模式
 */
enum Mode {
    /// 由电控控制
    MODE_AUTO=0,

    /// 一代自瞄
    MODE_ARMOR1,

    /// 二代自瞄
    MODE_ARMOR2,

    /// 小能量机关
    MODE_SMALLRUNE,

    /// 大能量机关
    MODE_BIGRUNE,

    /// 英雄吊射
    MODE_HERO,

    /// 工程取弹
    MODE_ENGINEER,

    /// 默认错误模式
    MODE_DEFAULT
};

/** 
 * @brief 云台相应坐标
 * 云台坐标系中装甲板中心的三个坐标
 */
typedef struct Target {
    /// x轴坐标
    double x;

    /// y轴坐标
    double y;

    /// z轴坐标
    double z;

    /**
     * @brief 重载流输出运算符
     *
     * @param out 标准输出流
     * @param target Target 结构体
     * @return 标准输出流
     */
    friend std::ostream &operator<< (std::ostream &out, const Target &target) {
        out << "target: " <<  "[" << target.x << ", " << target.y << ", " << target.z << "]\n";
        return out;
    }
} Target;

/**
 * @brief 发送数据包
 * 发送与电控通讯间的相应所需数据
 */
typedef struct SendPack {
    /// 工作模式
    int mode;

    /// 一代自瞄参数
    /// 预测yaw
    double pred_yaw;

    /// 预测pitcn
    double pred_pitch;

    /// 二代自瞄参数
    /// 解算的x
    double x;

    /// 解算的y
    double y;

    /// 解算的z
    double z;

    /// 延迟时间
    double time_delay;

    /**
     * @brief 构造函数，初始化成员变量
     */
    SendPack() : mode(MODE_DEFAULT),
                 pred_yaw(0.0),
                 pred_pitch(0.0),
                 x(0.0), y(0.0), z(0.0),
                 time_delay(0.0) {}
    
    /**
     * @brief 从 `target` 中读取 x, y, z
     *
     * @param target Target 结构体变量
     */
    void set(const Target& target) {
        x = target.x;
        y = target.y;
        z = target.z;
    }

    /**
     * @brief 将 x, y, z, yaw, pitch 清零
     */
    void clear() {
        x = y = z = pred_yaw = pred_pitch = 0.0;
    }

    /**
     * @brief 重载流输出运算符
     *
     * @param out 标准输出流
     * @param send_pack SendPack 结构体
     * @return 标准输出流
     */    
    friend std::ostream &operator<< (std::ostream &out, const SendPack &send_pack) {
        // x, y, z 已经在 Target 里面打印了，就不需要在这里打印了
        out << "work_mode: " << send_pack.mode << std::endl
            << "pred_yaw: " << send_pack.pred_yaw << std::endl
            << "pred_pitch: " << send_pack.pred_pitch << std::endl
            << "time_delay: " << send_pack.time_delay << std::endl;        
        return out;
    }
} SendPack;

/**
 * @brief 接收数据包
 * 从电控接收到视觉这边所需要的信息
 */
typedef struct ReadPack{
    /// 工作模式
    int mode;

    /// 敌方颜色代号
    int enemy_color;

    /// yaw偏角
    double ptz_yaw;

    /// pitch偏角
    double ptz_pitch;

    /// 子弹速度
    double bullet_speed;

    /**
     * @brief 构造函数，初始化成员变量
     */
    ReadPack() : mode(MODE_DEFAULT),
                 enemy_color(COLOR_DEFAULT),
                 ptz_yaw(0),
                 ptz_pitch(0),
                 bullet_speed(15) {}
    
    /**
     * @brief 重载流输出运算符
     *
     * @param out 标准输出流
     * @param read_pack ReadPack 结构体
     * @return 标准输出流
     */
    friend std::ostream &operator<< (std::ostream &out, const ReadPack& read_pack) {
        // `mode` 已经在 SendPack 中打印了，这里无需打印
        out << "enemy_color: " << read_pack.enemy_color << std::endl
            << "ptz_yaw: " << read_pack.ptz_yaw << std::endl
            << "ptz_pitch: " << read_pack.ptz_pitch << std::endl
            << "bullet_speed: " << read_pack.bullet_speed << std::endl;
        return out;
    }
} ReadPack;

#endif  // TYPES_H
