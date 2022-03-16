#include "classifier.h"

using namespace cv;
using namespace std;

Classifier::Classifier(char *cfg_file, char *weight_file, const char *name_file) {
    net = load_network(cfg_file, weight_file, 0);
    set_batch_network(net, 1);
    srand(2222222);

    // 标签文件
    ifstream f_label(name_file);
    if (f_label.is_open()) {
        string label;
        while (getline(f_label, label)) {
            labels.emplace_back(label);
        }
    }

    size_t srcSize = 28 * 28 * 3 * sizeof(float);
    input = (float *) malloc(srcSize);
}

int Classifier::predict(const cv::Mat &src) {
    // 将图像转为yolo形式
    imgConvert(src, input);

    // 网络推理
    float *predictions = network_predict(net, input);

    if (net->hierarchy) {
        hierarchy_predictions(predictions, net->outputs, net->hierarchy, 1, 1);
    }
    int *indexes = (int *) calloc(TOP, sizeof(int));
    top_k(predictions, net->outputs, TOP, indexes);
    for (int i = 0; i < TOP; ++i) {
        int index = indexes[i];
        printf("%5.2f%%: %s\n", predictions[index] * 100, labels[index].c_str());
    }
    return indexes[0];
}

void Classifier::imgConvert(const cv::Mat &img, float *dst) {
    uchar *data = img.data;
    int h = img.rows;
    int w = img.cols;
    int c = img.channels();

    for (int k = 0; k < c; ++k) {
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                dst[k * w * h + i * w + j] = data[(i * w + j) * c + k] / 255.;
            }
        }
    }
}