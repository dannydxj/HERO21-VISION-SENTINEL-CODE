#include <omp.h>
#include <util.h>
#include "timer.h"
#include "armor.h"

using namespace cv;
using namespace std;

double Armor::GAMMA_C = 2.2;
double Armor::GAMMA_G = 1.0;
int Armor::GAMMA_THRES = 30;

Armor::Armor() : is_valid(false), priority(0) {
}

Armor::Armor(const cv::Mat &src,
             const cv::RotatedRect &rotated_rect,
             const int color,
             double score, cv::Point2f vertices[4]) : rotated_rect(rotated_rect),
                                                      color(color),
                                                      score(score),
                                                      number(0),
                                                      priority(0),
                                                      is_valid(true) {

    // 扩大旋转矩形高度, 尽可能保留数字的全部特征
    //Armor::rotated_rect.size.width *= ROTATEDRECT_WIDTH_RATE;
    //Armor::rotated_rect.size.height *= ROTATEDRECT_HEIGHT_RATE;

    float width = Armor::rotated_rect.size.width;
    float height = Armor::rotated_rect.size.height;

    // 为透视变换做准备
    // 装甲板角点排列顺序: 左上角, 右上角, 右下角, 左下角
    cv::Point2f P[4];
    Armor::rotated_rect.points(P);

    // 源图像中的角点排列
    //cv::Point2f src_rect[4];
    //src_rect[0] = P[1];
    //src_rect[1] = P[2];
    //src_rect[2] = P[3];
    //src_rect[3] = P[0];

    // 变换后的角点排列
    cv::Point2f dst_rect[4];
    dst_rect[0] = cv::Point2f(0, 0);
    dst_rect[1] = cv::Point2f(width, 0);
    dst_rect[2] = cv::Point2f(width, height);
    dst_rect[3] = cv::Point2f(0, height);

    // 应用透视变换,将装甲板区域矫正成规则矩形
    cv::Mat transform = cv::getPerspectiveTransform(vertices, dst_rect);
    cv::Mat perspect_img;
    cv::warpPerspective(src, perspect_img, transform, src.size());

    // 防止超出边界
    width = src.cols < width ? src.cols : width;
    height = src.rows < height ? src.rows : height;

    // 提取数字编号区域
    // 提取数字编号区域
    //imshow("per", perspect_img);
    number_img = perspect_img(cv::Rect(0, 0, width, height));
    //number_img = number_img(cv::Rect(0.15 * width, 0, 0.8 * width, height));
    //cv::imshow("la", number_img);
    //number_img = perspect_img(cv::Rect(0, 0, width, height));
    cv::resize(number_img, number_img, cv::Size(28, 28));

    // 伽马矫正
    // here！！！

    //cvtColor(number_img, number_img, COLOR_BGR2GRAY);
    //imshow("number_img", number_img);

    //imshow("gray", number_img);
    //gammaCorrect(number_img, number_img);
    //imshow("gamma", number_img);

    // 按阈值筛选
    //threshold(number_img, number_img, 31, 255, THRESH_BINARY);
    //imshow("gamma_final", number_img);
    //++count;
    cv::imwrite("../save/1.jpg", number_img);
    //waitKey();
    // 展示图像
    // cv::imshow("number_img", number_img);
    //number_img = Util::convertTo3Channels(number_img);
    // 大小还原
    //Armor::rotated_rect.size.width /= ROTATEDRECT_WIDTH_RATE;
    //Armor::rotated_rect.size.height /= ROTATEDRECT_HEIGHT_RATE;

}

Armor::~Armor() = default;

int Armor::getNumber() const {
    return number;
}

void Armor::setNumber(int number) {
    this->number = number;
    is_valid = true;
    // 根据数字编号生成打击优先级
    switch (number) {
        case 1:
            priority = 4;
            break;
        case 3:
        case 4:
            priority = 3;
            break;
        case 7:
            priority = 2;
            break;
        case 2:
            priority = 1;
            break;
        case 5:
        case 6:
        default:
            priority = 0;
    }
}

bool Armor::scoreComparator(const Armor &a, const Armor &b) {
    return a.score < b.score;
}

bool Armor::priorityComparator(const Armor &a, const Armor &b) {
    return a.priority > b.priority;
}

Armor &Armor::operator=(const Armor &armor) {
    this->number_img = armor.number_img;
    this->number = armor.number;
    this->rotated_rect = armor.rotated_rect;
    //this->rect = armor.rect;
    this->color = armor.color;
    this->score = armor.score;
    this->priority = armor.priority;
    this->is_valid = armor.is_valid;
    return *this;
}

cv::Rect Armor::rect() const {
    return rotated_rect.boundingRect();
}

void Armor::gammaCorrect(Mat &src, Mat &dst, double gammaG, double gammaC) {
    int rows = src.rows;
    int cols = src.cols;
    uchar *p_src, *p_dst;
    double val;

    // 直接原地修改
    // openmp并行化, 加速效果待测试
    if (&src == &dst) {
#pragma omp parallel for num_threads(4) default(none) shared(src, rows, cols, gammaG) private(val, p_src, i, j) \
    schedule(static, 1)
        for (int i = 0; i < rows; i++) {
            // 获取图像每一行的首地址
            p_src = src.ptr<uchar>(i);
            for (int j = 0; j < cols; j++) {
                // 每次迭代获取图像列的地址
                uchar &pix_src = *p_src++;
                // 归一化处理
                val = static_cast<double>(pix_src) / 255;
                // 按公式计算并逆归一化
                pix_src = static_cast<uchar>((pow(val / gammaC, 1 / gammaG) * 255));
            }
        }
    } else {
        Mat _dst = Mat(rows, cols, src.type());

#pragma omp parallel for num_threads(4) default(none) shared(src, dst, rows, cols, gammaG) private(val, p_src, p_dst, i, j) \
    schedule(static, 1)
        for (int i = 0; i < rows; i++) {
            // 获取图像每一行的首地址
            p_src = src.ptr<uchar>(i);
            p_dst = _dst.ptr<uchar>(i);
            for (int j = 0; j < cols; j++) {
                // 每次迭代获取图像列的地址
                uchar &pix_src = *p_src++;
                uchar &pix_dst = *p_dst++;
                // 归一化处理
                val = static_cast<double>(pix_src) / 255;
                // 按公式计算并逆归一化
                pix_dst = static_cast<uchar>((pow(val / gammaC, 1 / gammaG) * 255));
            }
        }
        dst = _dst;
    }
}
