
#include "armordetector.h"

#include <cmath>

#include "timer.h"
#include "util.h"

using namespace cv;
using namespace std;

ArmorDetector::ArmorDetector() : classifier((char *) "../src/armor_detect/classifier/darknet/cfg/mnist_cifar10.cfg",
                                            (char *) "../src/armor_detect/classifier/darknet/backup/mnist_cifar10_4.weights",
                                            (char *) "../src/armor_detect/classifier/darknet/data/names.list") {}

ArmorDetector::~ArmorDetector() = default;

void ArmorDetector::init(const FileStorage &file_storage) {
    roi_rect = Rect();
    FileNode arm_detect = file_storage["armor_detect"];
    // 是否使用 ROI
    ROI_ENABLE = arm_detect["ROI_ENABLE"];
    // 图像预处理相关参数传入
    GREY_THRES = arm_detect["GREY_THRES"];
    SUBTRACT_THRES = arm_detect["SUBTRACT_THRES"];
    KERNEL_SIZE = arm_detect["KERNEL_SIZE"];

    FRAME_WIDTH = file_storage["FRAME_WIDTH"];
    FRAME_HEIGHT = file_storage["FRAME_HEIGHT"];

    // 最大预选数量相关参数传入
    MAX_CANDIDATE_NUM = arm_detect["MAX_CANDIDATE_NUM"];
    // 装甲板筛选限定条件相关参数传入
    MIN_LIGHTBAR_AREA = arm_detect["MIN_LIGHTBAR_AREA"];
    MIN_ASPECT_RATIO = arm_detect["MIN_ASPECT_RATIO"];
    MAX_ASPECT_RATIO = arm_detect["MAX_ASPECT_RATIO"];
    MIN_LENGTH_RATIO = arm_detect["MIN_LENGTH_RATIO"];
    MAX_LENGTH_RATIO = arm_detect["MAX_LENGTH_RATIO"];
    MAX_LIGHTBAR_DELTA = arm_detect["MAX_LIGHTBAR_DELTA"];
    MAX_ARMOR_ANGLE = arm_detect["MAX_ARMOR_ANGLE"];
    MAX_ARMOR_LIGHTBAR_DELTA = arm_detect["MAX_ARMOR_LIGHTBAR_DELTA"];

#ifdef DISTORTION_CORRECT
    file_storage["camera_matrix"] >> camera_matrix;
    file_storage["distortion_coeff"] >> distortion_coeff;
#endif // DISTORTION_CORRECT

}

bool ArmorDetector::run(const Mat &src, const int enemy_color, Armor &target_armor) {
    Timer timer;
    timer.start();
    Preprocess(src, enemy_color);
    vector<Armor> vec_armors;
    findTarget(enemy_color, vec_armors);
    selectTarget(vec_armors);

    if (!vec_armors.empty()) {
        target_armor = vec_armors.at(0);
        if (!roi_rect.empty()) {
            target_armor.rotated_rect.center.x += roi_rect.x;
            target_armor.rotated_rect.center.y += roi_rect.y;
        }
        if (ROI_ENABLE) {
            setRoiRect(target_armor.rect());
        }
        return true;
    } else {
        roi_rect = Rect();
        return false;
    }
}

void ArmorDetector::Preprocess(const Mat &src, const int enemy_color) {
    if (ROI_ENABLE && !roi_rect.empty()) {
        src(roi_rect).copyTo(roi_image);
    } else {
        src.copyTo(roi_image);
    }
    // imshow("roi", roi_image);

#ifndef COMPILE_WITH_CUDA
// 畸变矫正
#ifdef DISTORTION_CORRECT
    static Mat map1, map2;
    initUndistortRectifyMap(camera_matrix, distortion_coeff, Mat(),
                            getOptimalNewCameraMatrix(camera_matrix, distortion_coeff, roi_image.size(), 1, roi_image.size(), 0),
                            roi_image.size(), CV_16SC2, map1, map2);
    remap(roi_image, roi_image, map1, map2, INTER_LINEAR);
#endif // DISTORTION_CORRECT

    Mat gray_image;
    Mat subtract_image;
    vector<Mat> channels;

    // 转灰度图
    cvtColor(roi_image, gray_image, COLOR_BGR2GRAY);

    // 按阈值筛选
    threshold(gray_image, gray_image, GREY_THRES, 255, THRESH_BINARY);

    // 通道分离相减
    split(roi_image, channels);
    if (enemy_color == COLOR_BLUE) {
        subtract(channels[0], channels[2], subtract_image);
    } else {
        subtract(channels[2], channels[0], subtract_image);
    }

    threshold(subtract_image, subtract_image, SUBTRACT_THRES, 255, THRESH_BINARY);
    // 取图像交集
    processed_image = gray_image & subtract_image;
    // 开运算
    morphologyEx(processed_image, processed_image, MORPH_CLOSE, kernel);
#else
    // 畸变矫正
#ifdef DISTORTION_CORRECT
    static Mat map1, map2;
    static cv::cuda::GpuMat gpu_map1, gpu_map2;
    initUndistortRectifyMap(camera_matrix, distortion_coeff, Mat(),
                            getOptimalNewCameraMatrix(camera_matrix, distortion_coeff, roi_image.size(), 1, roi_image.size(), 0),
                            roi_image.size(), CV_16SC2, map1, map2);
    gpu_map1.upload(map1);
    gpu_map2.upload(map2);
    cv::cuda::remap(roi_image, roi_image, gpu_map1, gpu_map2, INTER_LINEAR);
#endif // DISTORTION_CORRECT
    static cv::cuda::GpuMat gpu_src, gpu_dst;
    gpu_src.upload(roi_image);
    // 与CPU处理步骤相同
    static cv::cuda::GpuMat gpu_gray, gpu_subtract;
    static vector<cv::cuda::GpuMat> gpu_channels;

    cv::cuda::cvtColor(gpu_src, gpu_gray, COLOR_BGR2GRAY);
    cv::cuda::threshold(gpu_gray, gpu_gray, GREY_THRES, 255, THRESH_BINARY);

    cv::cuda::split(gpu_src, gpu_channels);
    if (enemy_color)
    {
        cv::cuda::subtract(gpu_channels[0], gpu_channels[2], gpu_subtract);
    }
    else
    {
        cv::cuda::subtract(gpu_channels[2], gpu_channels[0], gpu_subtract);
    }
    cv::cuda::threshold(gpu_subtract, gpu_subtract, SUBTRACT_THRES, 255, THRESH_BINARY);

    cv::cuda::bitwise_and(gpu_gray, gpu_subtract, gpu_dst);

    kernel->apply(gpu_dst, gpu_dst);
    gpu_dst.download(processed_image);
#endif // COMPILE_WITH_CUDA
}

void ArmorDetector::findTarget(const int enemy_color, vector<Armor> &armors) {
    // 找出所有轮廓
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    RotatedRect temp_rect;
    findContours(processed_image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // 按照面积初步筛选出灯条
    vector<RotatedRect> lightbars;
    for (auto &contour : contours) {
        temp_rect = minAreaRect(contour);
        if (temp_rect.size.width * temp_rect.size.height > MIN_LIGHTBAR_AREA) {
            adjustLightBar(temp_rect);
            lightbars.emplace_back(temp_rect);
        }
    }
    // 寻找装甲板
    findArmors(lightbars, enemy_color, armors);
}

void ArmorDetector::findArmors(vector<RotatedRect> &lightbars, const int enemy_color,
                               vector<Armor> &armors) {
    if (lightbars.empty() || lightbars.size() == 1)
        return;

    /// 左右灯条中心点
    Point2f left_center, right_center;

    /// 装甲宽，高，宽高比，灯条长度比
    double armor_width, armor_height, aspect_ratio, length_ratio;

    /// 灯条倾斜度差值，装甲板倾斜角度，灯甲倾斜度之差
    double lightbar_delta, armor_angle, armor_lightbar_delta;

    /// 得分情况
    double score;

    /// 寻找结果
    bool result;

    Point2f left_vertices[4], right_vertices[4];
    vector<Point2f> armor_vertices;
    RotatedRect temp_rect;

    for (int i = 0; i < lightbars.size() - 1; ++i) {
        left_center = lightbars[i].center;

        for (int j = i + 1; j < lightbars.size(); ++j) {
            right_center = lightbars[j].center;

            // 计算装甲板宽高比
            armor_width = sqrt((right_center.x - left_center.x) *
                               (right_center.x - left_center.x) +
                               (right_center.y - left_center.y) *
                               (right_center.y - left_center.y));
            armor_height = (lightbars[i].size.height + lightbars[j].size.height) / 2;

            aspect_ratio = armor_width / armor_height;

            // 计算两灯条长度比
            length_ratio = lightbars[i].size.height / lightbars[j].size.height;

            // 计算灯条倾斜度之差
            lightbar_delta = fabs(lightbars[i].angle - lightbars[j].angle);

            // 计算装甲板倾斜度
            armor_angle = atan((right_center.y - left_center.y) / (right_center.x - left_center.x)) * 180 / Util::PI;

            // 计算灯甲倾斜度之差
            armor_lightbar_delta = fabs((lightbars[i].angle + lightbars[j].angle) / 2.0 - armor_angle);

            // 判断
            result = 1;
            score = 0;

            // 长宽比在范围内
            result = result && aspect_ratio > MIN_ASPECT_RATIO && aspect_ratio < MAX_ASPECT_RATIO;
            score += fabs(aspect_ratio - 3.31) * 10.0 / 3.31;

            // 两灯条长度相差不过大
            result = result && length_ratio > MIN_LENGTH_RATIO && length_ratio < MAX_LENGTH_RATIO;
            if (length_ratio > 1.0) {
                score += (length_ratio - 1.0) * 10.0 / (MAX_LENGTH_RATIO - 1.0);
            } else {
                score += (1.0 - length_ratio) * 10.0 / (1.0 - MIN_LENGTH_RATIO);
            }

            // 两灯条倾斜度之差不过大
            result = result && lightbar_delta < MAX_LIGHTBAR_DELTA;
            score += lightbar_delta * 10.0 / MAX_LIGHTBAR_DELTA;

            // 装甲板倾斜度不过大
            result = result && fabs(armor_angle) < MAX_ARMOR_ANGLE;
            score += fabs(armor_angle) * 10.0 / MAX_ARMOR_ANGLE;

            // 灯甲倾斜度相差不过大
            result = result && armor_lightbar_delta < MAX_ARMOR_LIGHTBAR_DELTA;
            score += armor_lightbar_delta * 10.0 / MAX_ARMOR_LIGHTBAR_DELTA;

            if (result) {
                lightbars[i].points(left_vertices);
                lightbars[j].points(right_vertices);
                for (int k = 0; k < 4; ++k) {
                    armor_vertices.emplace_back(left_vertices[k]);
                    armor_vertices.emplace_back(right_vertices[k]);
                }

                //将灯条的四个顶点传入容器中
                // 为了截取出完整的数字，设定一个截取比例ratio
                Point2f perspect_vertices[4];
                Point2f left_up, left_down, right_up, right_down;
                left_up.x = left_center.x;
                left_up.y = left_center.y - ratio * lightbars[i].size.height;
                left_down.x = left_center.x;
                left_down.y = left_center.y + ratio * lightbars[i].size.height;
                right_up.x = right_center.x;
                right_up.y = right_center.y - ratio * lightbars[j].size.height;
                right_down.x = right_center.x;
                right_down.y = right_center.y + ratio * lightbars[j].size.height;
                perspect_vertices[0] = left_up;
                perspect_vertices[1] = right_up;
                perspect_vertices[2] = right_down;
                perspect_vertices[3] = left_down;


                // 得到装甲板
                temp_rect = minAreaRect(armor_vertices);
                adjustArmor(temp_rect);
                armors.emplace_back(Armor(roi_image, temp_rect, enemy_color, score, perspect_vertices));
                armor_vertices.clear();
            }
        }
    }
}

void ArmorDetector::selectTarget(vector<Armor> &armors) {
    // 无候选装甲板, 无需再挑选
    if (armors.empty()) {
        return;
    }

    // 先根据误差得分升序排列
    sort(armors.begin(), armors.end(), Armor::scoreComparator);

    // 已经进行分类的装甲板数量
    // 考虑的分类推理的耗时, 需要对数量进行限制
    int cnt = 0;

    // 根据数字识别结果筛选, 删除误识别的候选者
    for (auto iter = armors.begin(); (iter != armors.end()) && (cnt < MAX_CANDIDATE_NUM);) {
#ifdef USE_MODEL
        Timer timer1;
        timer1.start();
        imshow("img", iter->number_img);
        Mat img = imread("../image/23408.jpg");
        classifier.predict(img);
        //iter->setNumber(classifier.predict(iter->number_img));
        timer1.printTime("predict:");
#else
        iter->setNumber(1);
#endif
        if (iter->getNumber() == 0) {
            //iter = armors.erase(iter);
            ++iter;
            ++cnt;
        } else {
            ++iter;
            ++cnt;
        }
    }

    // 根据打击优先级降序排列
    sort(armors.begin(), armors.end(), Armor::priorityComparator);
}


void ArmorDetector::adjustLightBar(RotatedRect &rect) {
    if (rect.size.width > rect.size.height) {
        rect = RotatedRect(rect.center, Size2f(rect.size.height, rect.size.width), rect.angle + 90);
    }
}

void ArmorDetector::adjustArmor(RotatedRect &rect) {
    if (rect.size.width < rect.size.height) {
        rect = RotatedRect(rect.center, Size2f(rect.size.height, rect.size.width), rect.angle + 90);
    }
}

const Rect &ArmorDetector::getRoiRect() const {
    return roi_rect;
}

void ArmorDetector::setRoiRect(const Rect &roiRect) {
    int detect_x, detect_y;
    int detect_width, detect_height;

    detect_width = static_cast<int>(roiRect.width * 2.0);
    detect_height = static_cast<int>(roiRect.height * 2.0);
    detect_x = roiRect.x - (detect_width - roiRect.width) / 2;
    detect_y = roiRect.y - (detect_height - roiRect.height) / 2;

    preventROIExceed(detect_x, detect_y, detect_width, detect_height);
    roi_rect = Rect(detect_x, detect_y, detect_width, detect_height);
}

void ArmorDetector::preventROIExceed(int &x, int &y, int &width, int &height) {
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x + width > FRAME_WIDTH)
        width = FRAME_WIDTH - x;
    if (y + height > FRAME_HEIGHT)
        height = FRAME_HEIGHT - y;
}
