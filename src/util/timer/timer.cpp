#include "timer.h"

#include <iostream>

Timer::Timer() {
    // 设置起始值为零
    time_start.tv_sec = 0.0;
    time_start.tv_usec = 0.0;
    // 计时器初始状态为未工作
    is_open = false;
}

Timer::~Timer() = default;

void Timer::start() {
    // 开始时间为当前时间
    gettimeofday(&time_start, nullptr);

    // 打开状态为true
    is_open = true;
}

void Timer::stop() {
    // 将开始时间初始化为0
    time_start.tv_sec = 0.0;
    time_start.tv_usec = 0.0;

    // 打开状态为false
    is_open = false;
}

float Timer::getTime() const {
    // 启动异常提示
    if (!is_open) {
        std::cerr << "计时器未工作!\n";
        return 0.0;
    }
    /* 返回模块所需时间，用当前时间减去设定的开始时间，最后转为毫秒级，
       CLOCKS_PER_SEC为一秒钟内CPU运行的周期数 */
    /// 此时的日历时间
    timeval time_current;
    gettimeofday(&time_current, nullptr);
    // 返回模块所需时间，tv_sec 和 tv_usec 都用当前时间减去设定的开始时间，并转为毫秒级
    // 注意1000“.0”！
    return static_cast<float>(((time_current.tv_sec - time_start.tv_sec) * 1000 + (time_current.tv_usec - time_start.tv_usec) / 1000.0));
}

void Timer::printTime(const std::string &message) const {
    // 启动异常提示
    if (!is_open) {
        std::cerr << "计时器未工作!\n";
        return;
    }
    /// 此时的日历时间
    timeval time_current;
    gettimeofday(&time_current, nullptr);
    /// 计时器的工作时间
    double delta_time = static_cast<double>(
        ((time_current.tv_sec - time_start.tv_sec) * 1000 +
         (time_current.tv_usec - time_start.tv_usec) / 1000.0));
    // 无需返回值，作为函数一部分，用则直接打印出来
    std::cout << message << " time costs: " << delta_time << "ms" << std::endl;
}
