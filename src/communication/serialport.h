/**
 * @file serialport.h
 * @brief 串口驱动类头文件
 * @details 实现串口相关参数的设定和初始化, 并以协调好的通信协议进行数据的收发
 *          通信协议见如下链接
 * @see https://hitwhlc.yuque.com/hero-rm/fxhbfd/hxzphp
 * @author 王筱琰，李昊天
 * @version 2020 Season
 * @update 李昊天
 * @email lcyxlihaotian@126.com
 * @date 2020-12-31
 * @license Copyright© 2020 HITwh HERO-RoboMaster Group
 */
#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>
#include <exception>
#include <opencv2/opencv.hpp>
#include "util/base.h"
#include "util/types.h"

/**
 * @brief 串口驱动类
 * 实现串口相关参数的设定和初始化, 并以协调好的通信协议进行数据的收发
 * 阅读时，只需要看收发函数的实现方式，其他皆为固定配置
 */
class SerialPort
{
private:
    /// 串口设备名称
    std::string port_name;

    /// 波特率, 默认115200
    long baud_rate;

    /// 数据位长度, <br>5: 5bits, <br>6: 6bits, <br>7: 7bits, <br>8: 8bits
    int byte_size;

    /// 校验位, <br>0: 无校验, <br>1: 奇校验, <br>2: 偶校验
    int parity;

    /// 停止位, <br>1: 1bit, <br>2: 2bits, <br>3: 1.5bits
    int stop_bit;

    /// 流控方式, <br>0: 无控制, <br>1: 软件控制, <br>2: 硬件控制
    int flow_control;

    /// 文件描述符
    int fd;

    /// 是否打印发送信息，是1否0
    int DEBUG_INFO;

    /// 串口工作状态, true表示已经打开, false表示未打开
    bool is_open;

public:
    /**
     * @brief 默认构造函数
     */
    SerialPort();

    /**
     * @brief 自定义构造函数, 进行串口配置
     *
     * @param port_name 串口设备名称
     * @param baud_rate 波特率
     * @param byte_size 数据位长度
     * @param parity 校验位
     * @param stop_bit 停止位
     * @param flow_control 流控方式 
     */
    SerialPort(const std::string &port_name,
               long baud_rate = 115200,
               int byte_size = 8,
               int parity = 0,
               int stop_bit = 1,
               int flow_control = 0);

    /**
     * @brief 默认析构函数, 关闭串口
     */
    ~SerialPort();

    /**
     * @brief 打开串口
     */
    void open(cv::FileStorage &file_storage);

    /**
     * @brief 配置串口并打开串口
     * 
     * @param port_name 串口设备名称
     * @param baud_rate 波特率
     * @param byte_size 数据位长度
     * @param parity 校验位
     * @param stop_bit 停止位
     * @param flow_control 流控方式
     */
    void open(const std::string &port_name, const cv::FileStorage &file_storage,
              long baud_rate = 115200,
              int byte_size = 8,
              int parity = 0,
              int stop_bit = 1,
              int flow_control = 0);

    /**
     * @brief 串口工作状态
     */
    bool isOpen();

    /**
     * @brief 关闭串口
     */
    void close();

    /**
     * @brief 设置波特率
     * 
     * @param baud_rate 设定值
     */
    void setBaudRate(long baud_rate);

    /**
     * @brief 获取波特率
     * 
     * @return 当前波特率
     */
    long getBaudRate();

    /**
     * @brief 设置数据位长度
     * 
     * @param byte_size 数据位长度设定值
     */
    void setByteSize(int byte_size);

    /**
     * @brief 获取数据位长度
     * 
     * @return 当前数据位长度
     */
    int getByteSize();

    /**
     * @brief 设置校验方式
     * 
     * @param parity 校验方式设定值
     */
    void setParity(int parity);

    /**
     * @brief 获取校验方式
     * 
     * @return 当前校验方式
     */
    int getParity();

    /**
     * @brief 设置停止位
     * 
     * @param stop_bit 停止位设定值
     */
    void setStopBit(int stop_bit);

    /**
     * @brief 获取停止位
     * 
     * @return 当前停止位
     */
    int getStopBit();

    /**
     * @brief 设定流控方式
     * 
     * @param flow_control 流控方式设定值
     */
    void setFlowControl(int flow_control);

    /**
     * @brief 获取流控方式
     * 
     * @return 当前流控方式
     */
    int getFlowControl();

    /**
     * @brief 串口数据打包并向MCU发送
     * 
     * @param send_pack 发送数据包
     */
    void sendData(const SendPack &send_pack);

    /**
     * @brief 从MCU接收串口数据包并解包
     * 
     * @param read_pack 接收数据包
     * @return 是否接收并解包成功
     *     @retval true 解包成功
     *     @retval false 解包失败
     */
    bool readData(ReadPack &read_pack);

private:
    /**
     * @brief 重新配置串口并开启
     */
    void reconfigurePort() const;
};

/**
 * @brief 串口异常类
 * 串口在比赛中可能出现连接不稳定等异常，写一异常类帮助相机重新连接
 * 同时防止因为相机异常造成的程序崩溃
 */
class SerialException : public std::exception
{
private:
    /// 异常信息字符串
    std::string e_what_;

public:
    /**
     * @brief 默认构造函数
     */
    SerialException() {}

    /**
     * @brief 自定义构造函数, 需要给出异常信息
     * 
     * @param error 异常描述信息
     */
    SerialException(const std::string &error) : e_what_(error) {}

    /**
     * @brief 默认析构函数
     */
    virtual ~SerialException() throw() {}

    /**
     * @brief 异常规格说明: 不抛出异常
     * 
     * @return 异常信息字符串
     * @note 该函数为std::exception类中的覆盖
     */
    virtual const char *what() const throw()
    {
        return e_what_.c_str();
    }
};

#endif // SERIALPORT_H
