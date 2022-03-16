/**
 * @file debugger.cpp
 * @brief Debugger class.
 * @details Output debug info and show images.
 * @author Haotian Li
 * @version 2020 Season
 * @update Li Haotian
 * @email lcyxlihaotian@126.com
 * @date 2020-12-13
 * @license Copyright© 2020 HITwh HERO-RoboMaster Group
 */

#include "base.h"
#include "debugger.h"

#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;
using namespace cv;

void Debugger::warning(const std::string& description,
                       const std::string& file_name,
                       const std::string& func_name,
                       int line) {
    cerr << "[WARNING] In function '" << func_name << "':\n"
         << file_name << ":" << line << ": " << description << endl;
}

void Debugger::error(const std::string& description,
                     const std::string& file_name,
                     const std::string& func_name,
                     int line) {
    cerr << "[ERROR] In function '" << func_name << "':\n"
         << file_name << ":" << line << ": " << description << endl;
    exit(1);
}

void Debugger::trackbar(ArmorDetector& detector,
                        const string& window_name_target,
                        const string& window_name_proc) {
    /* Trackbar for function `Preprocess()`. */
    createTrackbar("gray_thres", window_name_proc, &detector.GREY_THRES, 60, 0, 0);
    createTrackbar("subtract_thres", window_name_proc, &detector.SUBTRACT_THRES, 60, 0, 0);
    createTrackbar("kernel_size", window_name_proc, &detector.KERNEL_SIZE, 15, 0, 0);

#ifdef COMPILE_WITH_CUDA
    detector.kernel = cv::cuda::createMorphologyFilter(MORPH_CLOSE, CV_8U,
                                    getStructuringElement(MORPH_RECT, 
                                        Size(detector.KERNEL_SIZE, detector.KERNEL_SIZE)));
#else
    detector.kernel = getStructuringElement(MORPH_RECT, 
                          Size(detector.KERNEL_SIZE, detector.KERNEL_SIZE));
#endif

    /* Trackbar for function `selectTarget()`. */
    static int min_asp_ratio = static_cast<int>(detector.MIN_ASPECT_RATIO * 100);
    createTrackbar("min_aspect_ratio", window_name_target, &min_asp_ratio, 250, 0, 0);
    detector.MIN_ASPECT_RATIO = static_cast<double>(min_asp_ratio) / 100.0;

    static int max_asp_ratio = static_cast<int>(detector.MAX_ASPECT_RATIO * 100);
    createTrackbar("max_aspect_ration", window_name_target, &max_asp_ratio, 900, 0, 0);
    detector.MAX_ASPECT_RATIO = static_cast<double>(max_asp_ratio) / 100.0;

    static int min_len_ratio = static_cast<int>(detector.MIN_LENGTH_RATIO * 100);
    createTrackbar("min_length_ration", window_name_target, &min_len_ratio, 100, 0, 0);
    detector.MIN_LENGTH_RATIO = static_cast<double>(min_len_ratio) / 100.0;

    static int max_len_ratio = static_cast<int>(detector.MAX_LENGTH_RATIO * 100);
    createTrackbar("max_length_ratio", window_name_target, &max_len_ratio, 400, 0, 0);
    detector.MAX_LENGTH_RATIO = static_cast<double>(max_len_ratio) / 100.0;

    static int max_lb_delta = static_cast<int>(detector.MAX_LIGHTBAR_DELTA * 10);
    createTrackbar("max_lightbar_delta", window_name_target, &max_lb_delta, 500, 0, 0);
    detector.MAX_LIGHTBAR_DELTA = static_cast<double>(max_lb_delta) / 10.0;

    static int max_am_angle = static_cast<int>(detector.MAX_ARMOR_ANGLE * 10);
    createTrackbar("max_armor_angle", window_name_target, &max_am_angle, 500, 0, 0);
    detector.MAX_ARMOR_ANGLE = static_cast<double>(max_am_angle) / 10.0;

    static int max_delta = static_cast<int>(detector.MAX_ARMOR_LIGHTBAR_DELTA * 10);
    createTrackbar("max_armor_lightbar_delta", window_name_target, &max_delta, 500, 0, 0);
    detector.MAX_ARMOR_LIGHTBAR_DELTA = static_cast<double>(max_delta) / 10.0;
}

template <class T>
void Debugger::drawValue(const T& value,
                         string name,
                         Point point,
                         Mat& image) {
    static stringstream sstr;
    sstr << name << ": " << value;
    putText(image, sstr.str(), point, QT_FONT_NORMAL, 1, Scalar(0, 255, 0));
    sstr.str("");
}

void Debugger::drawTypeValue(Target& target, cv::Mat& image) {
    drawValue(target.x, "x", Point(20, 30), image);
    drawValue(target.y, "y", Point(20, 60), image);
    drawValue(target.z, "z", Point(20, 90), image);
}

void Debugger::drawTypeValue(ReadPack& read_pack, cv::Mat& image) {
    drawValue(read_pack.ptz_yaw, "read_yaw", Point(20, 120), image);
    drawValue(read_pack.ptz_pitch, "read_pitch", Point(20, 150), image);
}

void Debugger::drawTypeValue(SendPack& send_pack, cv::Mat& image) {
    drawValue(send_pack.pred_yaw, "pred_yaw", Point(20, 180), image);
    drawValue(send_pack.pred_pitch, "pred_pitch", Point(20, 210), image);
}

void Debugger::drawArmor(const Armor& armor, Mat& image) {
    drawRotatedRect(image, armor.rotated_rect, cv::Scalar(255, 0, 0));
    rectangle(image, armor.rect(), Scalar(0, 0, 255));
}

void Debugger::drawRotatedRect(cv::Mat& image,
                               const cv::RotatedRect& rect,
                               const cv::Scalar& scalar,
                               int thickness) {
    Point2f pt[4];
    rect.points(pt);
    for (int i = 0; i < 4; i++) {
        line(image, pt[i % 4], pt[(i + 1) % 4], scalar, thickness);
    }
}
