/**
 * @file classifier.h
 * @brief 数字分类器
 * @details 以28*28的cv::Mat图像为输入, 进行分类返回装甲板数字
 * @author 陆展
 * @version 2021 Season
 * @email 965105951@qq.com
 * @date 2020-10-03
 * @license Copyright© 2021 HITwh HERO-RoboMaster Group
 */
#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <darknet.h>

class Classifier {
public:
    std::vector<std::string> labels;
private:
    network *net;
    constexpr static int TOP = 1;
    float *input;
public:
    Classifier(char *cfg_file, char *weight_file, const char *name_list);

    int predict(const cv::Mat &src);

private:
    void imgConvert(const cv::Mat &img, float *dst);
};


#endif //CLASSIFIER_H
