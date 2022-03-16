#include "anglesolver.h"

#include "base.h"
#include "timer.h"
#include "util.h"

using namespace std;


AngleSolver::AngleSolver() = default;


AngleSolver::~AngleSolver() = default;


void AngleSolver::init() {
}

/**
 * @brief 角度解算核心函数, 输入目标在云台坐标系下的三维坐标、子弹速度和当前云台相对地面pitch轴角度, 输出要打击到目标云台yaw和pitch所需角度增量
 * @detail 先通过坐标系旋转将目标在云台坐标系下三维坐标转换为水平坐标系下三维坐标，再根据平抛运动公式计算出pitch轴角度增量，根据坐标直接计算出yaw轴角度增量
 *
 * @param target 云台三维坐标
 * @param v 子弹速度
 * @param ptz_pitch 当前云台相对地面pitch轴角度
 * @param yaw 解算得到的yaw轴角度增量
 * @param pitch 解算得到的pitch轴角度增量
 * @note 该代码正确性已得到多次验证，后续绝对不允许修改！！！
 * @attention 参数不可为NULL
*/
void AngleSolver::run(const Target &target, double v, double ptz_pitch, double &yaw, double &pitch) {
    // 坐标系转换，将云台坐标系绕x轴向下旋转大小等于原云台绝对pitch轴角的角度，相当于将坐标向上旋转相同的角度
    double delta_angle;
    double x_bar, y_bar, z_bar;
    /* 在进行三角函数运算前，要把以°为单位的角度
       转换为以rad为单位的角度 */
    delta_angle = ptz_pitch * Util::PI / 180;
    x_bar = target.x;
    y_bar = target.y * cos(delta_angle) - target.z * sin(delta_angle);
    z_bar = target.y * sin(delta_angle) + target.z * cos(delta_angle);
    if (parabolaSolve(sqrt(x_bar * x_bar + z_bar * z_bar), y_bar, v, ptz_pitch, pitch)) {
        /// 由于电控建立的坐标系是y轴竖直向上、其他轴方向不变，因此计算所得pitch轴角度增量需要取反号
        pitch = -pitch;
        yaw = atan(x_bar / z_bar) / Util::PI * 180;
    } else {
        pitch = 0.0;
        yaw = 0.0;
    }
}

/**
 * @brief 飞行延迟解算函数（主要用于预测）, 输入目标在云台坐标系下的三维坐标、子弹速度和当前云台相对地面pitch轴角度, 返回弹丸从射出枪口到击中目标所需飞行延迟时间
 * @detail 步骤类似角度解算，先通过坐标系旋转将目标在云台坐标系下三维坐标转换为水平坐标系下三维坐标，再根据平抛运动公式计算出弹丸从出射到击中目标的飞行延迟
 *
 * @param target 云台三维坐标
 * @param v 子弹速度
 * @param ptz_pitch 当前云台相对地面pitch轴角度
 * @return 返回double类型浮点数：弹丸从射出枪口到击中目标所需飞行延迟时间
 * @attention 参数不可为NULL！
 * @warning 暂未发现
 * @exception 暂未发现
 */
double AngleSolver::get_flight_time(const Target &target, double v, double ptz_pitch) {
    double time;
    double delta_angle;
    double x_bar, y_bar, z_bar;
    delta_angle = ptz_pitch * Util::PI / 180;
    x_bar = target.x;
    y_bar = target.y * cos(delta_angle) - target.z * sin(delta_angle);
    z_bar = target.y * sin(delta_angle) + target.z * cos(delta_angle);
    // 建立新坐标系:子弹出射点与目标点所在的竖直平面坐标系
    double x_x = sqrt(x_bar * x_bar + z_bar * z_bar);
    double y_y = y_bar;
    time = 2.0 * ((y_y * Util::G + v * v) -
                  sqrt(pow(Util::G * y_y + v * v, 2.0) - (x_x * x_x + y_y * y_y) * Util::G * Util::G)) /
           (Util::G * Util::G);
    time = sqrt(time);
    if (isnan(time)) {
        time = 0;
    }
    return time;
}

/**
 * @brief 抛物线运动解算函数，输入子弹发射点与目标的水平距离、竖直距离、子弹速度、当前云台相对地面pitch轴绝对角度，输出云台所需pitch轴角度增量，返回表示解算结果是否有效的布尔值
 * @detail 在子弹出射点与目标点所在的竖直平面内根据平抛运动公式计算pitch轴角度增量
 *
 * @param x 子弹发射点与目标的水平距离（x轴距离）
 * @param y 子弹发射点与目标的竖直距离（y轴距离）
 * @param v 子弹速度
 * @param ptz_pitch 当前云台相对地面pitch轴角度
 * @param theta pitch轴角度增量（解算结果）
 * @return 返回表示解算结果是否有效的布尔值
 *   @retval 返回true，表示解算结果有效;返回false，表示解算结果无效
 * @note 该代码正确性已被多次验证，不允许后续再修改！
 * @attention 参数不可为NULL！
 */
bool AngleSolver::parabolaSolve(double x, double y, double v, double ptz_pitch, double &theta) {
    double time;
    // 计算中间变量
    double res, res_1;
    time = 2.0 *
           ((y * Util::G + v * v) - sqrt(pow(Util::G * y + v * v, 2.0) -
                                         (x * x + y * y) * Util::G * Util::G)) /
           (Util::G * Util::G);
    time = sqrt(time);
    res_1 = (y - 0.5 * Util::G * time * time) / v / time;
    res = asin(res_1);
    theta = res * 180 / Util::PI;
    /* 由于电控传给我们的云台角度是y轴向上为正，
       此处相加相当于将计算所得云台需要转到的
       pitch轴角度减去云台现有角度，得到的是
       pitch轴需要的角度增量（y轴向下为正）*/
    theta = theta + ptz_pitch;
    if (isnan(theta) || isinf(theta)) {
        theta = 0;
        return false;
    }
    return true;
}
