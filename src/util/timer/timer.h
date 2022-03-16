/**
 * @file timer.h
 * @brief 计时器
 * @details 通过C内置的计时函数gettimeofday()实现一个简易的计时器,
 * 确定程序中某一模块的运行时间。对于 Linux 系统来说该方法更加精确。
 * @authors 周宇新，陆展
 * @version 2021 Season
 * @update 董行健
 * @email dannydxj@icloud.com
 * @date 2021-02-27
 * @license 2021 HITWH HERO-Robomaster Group
 */

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#include <string>

/**
 * @brief 计时器类
 * 用于获取打印各模块的时间，得到算法效率信息
 */
class Timer {
   private:
    /// 计时器启动时的日历时间，Linux下为从1970年1月1日0时整到此时的时间，用结构体timeval存储
    timeval time_start;

    /// 计时器工作状态, true表示正在工作, false表示未工作
    bool is_open;

   public:
    /**
     * @brief 默认构造函数
     */
    Timer();

    /**
     * @brief 默认析构函数
     */
    ~Timer();

    /**
     * @brief 启动计时器
     */
    void start();

    /**
     * @brief 停止计时器
     */
    void stop();

    /**
     * @brief 获取计时器工作时间, 计时器必须处于工作状态
     *
     * @return 工作时间, 单位为毫秒
     */
    float getTime() const;

    /**
     * @brief 打印计时器工作时间, 计时器必须处于工作状态
     *
     * @param message 需要打印运行时间的程序段描述信息
     */
    void printTime(const std::string &message) const;
};

#endif  // TIMER_H
