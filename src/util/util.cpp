#include "util.h"

#include <cmath>

using namespace cv;
using namespace std;

/* 云台坐标系的x，y，z轴随着云台旋转位置在不断改变，装甲板的云台坐标也在不断变换，
   绝对坐标系即被指定为初始状态下的云台坐标系，它不随云台旋转而改变，
   根据从单片机实时读到的yaw/pitch值做相应计算得到 */
void Util::coordinate_transformation(double &x, double &y, double &z,
                                     double ptz_pitch, double ptz_yaw) {
    double absolute_x = 0;
    double absolute_y = 0;
    double absolute_z = 0;
    // 转换为弧度制
    double read_yaw = ptz_yaw * PI / 180;
    double read_pitch = ptz_pitch * PI / 180;
    absolute_x = x * cos(read_yaw) + y * sin(read_pitch) * sin(read_yaw) -
                 z * cos(read_pitch) * sin(read_yaw);
    absolute_y = y * cos(read_pitch) + z * sin(read_pitch);
    absolute_z = x * sin(read_yaw) - y * sin(read_pitch) * cos(read_yaw) +
                 z * cos(read_pitch) * cos(read_yaw);
    x = absolute_x;
    y = absolute_y;
    z = absolute_z;
}

// 与上一个函数的意义完全相反
void Util::anti_coordinate_transformation(double &x, double &y, double &z,
                                          double ptz_pitch,
                                          double ptz_yaw) {
    double objective_x = 0;
    double objective_y = 0;
    double objective_z = 0;
    // 将偏角转换为弧度制
    double read_yaw = ptz_yaw * PI / 180;
    double read_pitch = ptz_pitch * PI / 180;
    objective_x = x * cos(read_yaw) + z * sin(read_yaw);
    objective_y = x * sin(read_yaw) * sin(read_pitch) + y * cos(read_pitch) -
                  z * cos(read_yaw) * sin(read_pitch);
    objective_z = -x * sin(read_yaw) * cos(read_pitch) + y * sin(read_pitch) +
                  z * cos(read_yaw) * cos(read_pitch);
    x = objective_x;
    y = objective_y;
    z = objective_z;
}

// 空间坐标系下点间距离
double Util::distance(const Point3d &point1, const Point3d &point2) {
    double distance = sqrt((point2.x - point1.x) * (point2.x - point1.x) +
                           (point2.y - point1.y) * (point2.y - point1.y) +
                           (point2.z - point1.z) * (point2.z - point1.z));
    return distance;
}

// 图像坐标系下点间距离
double Util::distance(const Point2d &point1, const Point2d &point2) {
    double distance = sqrt((point2.x - point1.x) * (point2.x - point1.x) +
                           (point2.y - point1.y) * (point2.y - point1.y));
    return distance;
}

double Util::get_flight_time(double x, double y, double v, double ptz_pitch) {
    double time_square;
    double delta_angle;
    double x_bar;
    double y_bar;
    delta_angle = ptz_pitch * PI / 180;
    // x_bar为坐标系旋转后x、z影响下组成的新轴
    x_bar = x * cos(delta_angle) + y * sin(delta_angle);
    // 原y轴
    y_bar = -x * sin(delta_angle) + y * cos(delta_angle);
    // 以当前云台pitch轴发射子弹打到装甲板中心所需的子弹飞行时间
    time_square = 2.0 *
                  ((y_bar * G + v * v) - sqrt(pow(G * y_bar + v * v, 2.0) - (x_bar * x_bar + y_bar * y_bar) * G * G)) /
                  (G * G);
    double flight_time = sqrt(time_square) + 0.05;
    if (isnan(flight_time) || isinf(flight_time)) {
        flight_time = 0.05;
    }
    return flight_time;
}

cv::Mat Util::convertTo3Channels(const Mat &binImg) {
    Mat three_channel = Mat::zeros(binImg.rows, binImg.cols, CV_8UC3);
    vector<Mat> channels;
    for (int i = 0; i < 3; i++) {
        channels.push_back(binImg);
    }
    merge(channels, three_channel);
    return three_channel;
}

bool Util::equalZero(double x) {
    return abs(x) <= EXP;
}