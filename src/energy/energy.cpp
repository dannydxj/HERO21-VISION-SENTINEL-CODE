#include "energy.h"
#include "timer.h"
#include "util.h"
#include "debugger.h"

using namespace cv;
using namespace std;

Energy::Energy() {
    isFindCenterR = false;
    isCalibrated = false;
    energy_centers.reserve(50);
    fan_centers.reserve(50);
    angle_array.reserve(50);
    direction = DIR_DEFAULT;
    object_Points.reserve(5);
}

void Energy::init(const FileStorage &file_storage) {
    file_storage["camera_matrix"] >> CAMERA_MATRIX;
    file_storage["distortion_coeff"] >> DISTCOEFFS;
    FileNode energy_node = file_storage["energy"];
    MIN_ENERGY_AREA = energy_node["MIN_ENERGY_AREA"];
    MAX_ENERGY_AREA = energy_node["MAX_ENERGY_AREA"];
    MIN_ENERGY_RATIO = energy_node["MIN_ENERGY_RATIO"];
    MAX_ENERGY_RATIO = energy_node["MAX_ENERGY_RATIO"];
    MIN_FAN_AREA = energy_node["MIN_FAN_AREA"];
    MAX_FAN_AREA = energy_node["MAX_FAN_AREA"];
    MIN_FAN_RATIO = energy_node["MIN_FAN_RATIO"];
    MAX_FAN_RATIO = energy_node["MAX_FAN_RATIO"];
    MIN_R_AREA = energy_node["MIN_R_AREA"];
    MAX_R_AREA = energy_node["MAX_R_AREA"];
    ENERGY_HALF_LENGTH = energy_node["ENERGY_HALF_LENGTH"];
    ENERGY_HALF_WIDTH = energy_node["ENERGY_HALF_WIDTH"];
    R_HALF_LENGTH = energy_node["R_HALF_LENGTH"];
    R_HALF_WIDTH = energy_node["R_HALF_WIDTH"];
    EXP = energy_node["EXP"];
    RADIUS = energy_node["RADIUS"];
    REAL_ENERGY_SIZE = make_pair(ENERGY_HALF_LENGTH, ENERGY_HALF_WIDTH);
    REAL_R_SIZE = make_pair(R_HALF_LENGTH, R_HALF_WIDTH);
}

Energy::~Energy() {
}

bool Energy::run(Mat &current_frame, int color, Target &target) {
    current_frame.copyTo(src);
    isFindEnergy = false;
    isFindFan = false;
    preprocess(color);

    isFindEnergy = findEnergy();
    //找能量板
    if (isFindEnergy) {
        //clog<<"find energy"<<endl;
        circle(current_frame, target_energy.center, 1, Scalar(0, 255, 0), 1);
        Debugger::drawRotatedRect(current_frame, target_energy, Scalar(0, 255, 0));
    }

    isFindFan = findFan();
    //找箭头
    if (isFindFan) {
        //clog<<"find fan"<<endl;
        circle(current_frame, target_fan.center, 1, Scalar(0, 255, 0), 1);
        Debugger::drawRotatedRect(current_frame, target_fan, Scalar(0, 255, 0));
    }
    imshow("energy with R", current_frame);
    //能量板,箭头任一没有找到,视为丢失目标
    //通常由于仰角过大造成,需要将角度复位至标定的原点
    if ((!isFindEnergy) && (!isFindFan)) {
        cerr << "丢失目标!\n";
        return false;
    }

    //imshow("energy", src);

    estimateRPosition();

    if (findR() && !isCalibrated) {
        automaticCentering();
    }
    circle(current_frame, current_center, 1, Scalar(0, 255, 0), 2);


    solveRealPostiton(predicted_energy, REAL_ENERGY_SIZE);


    target.x = translation_matrix.at<double>(0, 0) / 1000;
    target.y = (translation_matrix.at<double>(1, 0) - 49.19) / 1000;
    target.z = (translation_matrix.at<double>(2, 0) + 115.62) / 1000;
    return true;
}

void Energy::preprocess(int color) {
    vector<Mat> channels(3);
    split(src, channels);

    if (color == EnemyColor::COLOR_RED) {
        threshold(channels[2] - channels[0], bin, 50, 255, THRESH_BINARY_INV);
    } else {
        threshold(channels[0] - channels[2], bin, 90, 255, THRESH_BINARY_INV);
    }

    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    //dilate(temp_bin, temp_bin, element, Point(-1, -1), 2);
    morphologyEx(bin, bin, MORPH_OPEN, element);
}

bool Energy::findEnergy() {
    isFindEnergy = false;
    double area;
    float ratio;
    RotatedRect temp_rect;
    Mat temp_bin;
    bin.copyTo(temp_bin);

    imshow("raw", temp_bin);
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(temp_bin, temp_bin, MORPH_OPEN, element);
    imshow("before", temp_bin);

    //dilate(temp_bin, temp_bin, element, Point(-1, -1), 2);
    //floodFill(temp_bin, Point(0, 0), Scalar(0), 0, FLOODFILL_FIXED_RANGE);

    floodFill(temp_bin, Point(0, 0), Scalar(0));
    imshow("after_fill", temp_bin);

    element = getStructuringElement(MORPH_RECT, Size(4, 4));
    morphologyEx(temp_bin, temp_bin, MORPH_CLOSE, element);
    dilate(temp_bin, temp_bin, element, Point(-1, -1), 2);
    imshow("recovery", temp_bin);

    vector<vector<Point>> contours;
    findContours(temp_bin, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    // 目标轮廓
    vector<Point> target_contour;

    //cout<<contours.size()<<'\n';
    //筛选目标的能量板区域
    int i = 0;
    for (auto &contour : contours) {
        drawContours(src, contours, i, Scalar(200, 100, 0), 1);
        ++i;
        area = contourArea(contour);
        //cout << "能量板面积:  " << area << '\n';
        //根据面积筛选
        if (area < MIN_ENERGY_AREA || area > MAX_ENERGY_AREA)
            continue;

        target_contour = contour;
        //if (target_contour.size() <= 5)
        //    continue;
        //temp_rect = fitEllipse(target_contour);
        temp_rect = minAreaRect(target_contour);
        //temp_rect.size.height *= 0.75;
        //temp_rect.size.width *= 0.75;

        //按宽高比筛选
        float width, height;
        width = max(temp_rect.size.width, temp_rect.size.height);
        height = min(temp_rect.size.width, temp_rect.size.height);
        ratio = width / height;
        //cout << "能量板比例：" << ratio << '\n';
        if (ratio < MIN_ENERGY_RATIO || ratio > MAX_ENERGY_RATIO)
            continue;

        //统一样式
        if (temp_rect.size.width > temp_rect.size.height) {
            temp_rect.size = Size2f(height, width);
            temp_rect.angle += 90;
        }
        predicted_energy = target_energy = temp_rect;
        isFindEnergy = true;
        break;
    }
    return isFindEnergy;
}

bool Energy::findFan() {

    isFindFan = false;
    double area;
    float ratio;
    RotatedRect temp_rect;
    Mat temp_bin;
    bin.copyTo(temp_bin);

    //Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    //morphologyEx(temp_bin, temp_bin, MORPH_OPEN, element);
    //imshow("扇叶预处理后", temp_bin);

    vector<vector<Point>> contours;
    vector<Point> target_contour;       //目标轮廓
    findContours(temp_bin, contours, RETR_TREE, CHAIN_APPROX_NONE);
    //cout<<"扇叶轮廓总数:  "<<contours.size()<<'\n';
    //筛选目标的箭头区域
    for (size_t i = 0; i < contours.size(); ++i) {
        drawContours(src, contours, i, Scalar(200, 100, 0), 1);
        area = contourArea(contours[i]);
        //cout << "扇叶面积: " << area << '\n';
        //根据面积筛选
        if (area < MIN_FAN_AREA || area > MAX_FAN_AREA)
            continue;

        target_contour = contours[i];
//        drawContours(src, contours, i, Scalar(200, 100, 0), 1);
        temp_rect = fitEllipse(target_contour);

        float width, height;
        width = max(temp_rect.size.width, temp_rect.size.height);
        height = min(temp_rect.size.width, temp_rect.size.height);
        ratio = width / height;

        //cout << "扇叶比例：" << ratio << '\n';
        if (ratio < MIN_FAN_RATIO || ratio > MAX_FAN_RATIO)
            continue;
        if (!match(temp_rect)) {
            continue;
        }

        //统一样式
        if (temp_rect.size.width > temp_rect.size.height) {
            temp_rect.size = Size2f(height, width);
            temp_rect.angle += 90;
        }
        target_fan = temp_rect;
        isFindFan = true;
        break;
    }
    imshow("扇叶", src);
    return isFindFan;
}

bool Energy::findR() {
    Rect_<float> center_region = Rect_<float>(raw_center.x - CENTER_ROI.width / 2, raw_center.y - CENTER_ROI.height / 2,
                                              CENTER_ROI.width, CENTER_ROI.height);
    Mat center_img = bin(center_region);


    vector<vector<Point>> contours;
    vector<Point> target_contour;       //目标轮廓
    findContours(center_img, contours, RETR_CCOMP, CHAIN_APPROX_NONE);
    //cout<<contours.size()<<'\n';
    int i = 0;
    for (auto &contour:contours) {

        drawContours(src, contours, i, Scalar(200, 100, 0), 1);
        ++i;
        double area = contourArea(contour);
        //cout << "R的面积" << contourArea(contour) << "\n";
        if ((area >= MIN_R_AREA) && (area <= MAX_R_AREA)) {
            target_R = minAreaRect(contour);
            target_R.center += center_region.tl();
            current_center = minAreaRect(contour).center + center_region.tl();
            return true;
        }
    }
    return false;
}

bool Energy::match(const RotatedRect &candidate_fan) {
    vector<Point2f> intersection;
    if (rotatedRectangleIntersection(target_energy, candidate_fan, intersection) > 0)
        return true;
    else
        return false;
}

void Energy::estimateRPosition() {
    // 单位向量
    Point2f unitVector;
    unitVector = (target_fan.center - target_energy.center);
    unitVector /= Util::distance(target_fan.center, target_energy.center);
    raw_center = unitVector * RADIUS + target_energy.center;
}

void Energy::solveRealPostiton(const cv::RotatedRect &aim, const std::pair<float, float> &REAL_SIZE) {
    int i;
    object_Points.clear();
    image_Points.clear();

    Point2f vertices[4];
    aim.points(vertices);
    float dis;
    Point2f temp;
    //按顺时针重新排列四个角点
    for (i = 0; i < 4; ++i)
        image_Points.emplace_back(vertices[i]);
    dis = sqrt(pow(image_Points[0].x - image_Points[1].x, 2) + pow(image_Points[0].y - image_Points[1].y, 2));
    if (abs(dis - aim.size.height) > EXP) {
        temp = image_Points[0];
        image_Points.erase(image_Points.begin());
        image_Points.emplace_back(temp);
    }
    if (image_Points[0].x > image_Points[1].x) {
        temp = image_Points[0];
        image_Points.erase(image_Points.begin());
        image_Points.emplace_back(temp);
        temp = image_Points[0];
        image_Points.erase(image_Points.begin());
        image_Points.emplace_back(temp);
    }
    String text;
    for (i = 0; i < 4; ++i) {
        text = to_string(i);
        putText(src, text, image_Points[i], FONT_HERSHEY_SIMPLEX, 0.35, Scalar(255, 255, 255));
    }

    object_Points.emplace_back(Point3f(-REAL_SIZE.first, -REAL_SIZE.second, 0));
    object_Points.emplace_back(Point3f(REAL_SIZE.first, -REAL_SIZE.second, 0));
    object_Points.emplace_back(Point3f(REAL_SIZE.first, REAL_SIZE.second, 0));
    object_Points.emplace_back(Point3f(-REAL_SIZE.first, REAL_SIZE.second, 0));

    solvePnP(object_Points, image_Points, CAMERA_MATRIX, DISTCOEFFS, rotated_vector, translation_matrix, false,
             SOLVEPNP_EPNP);
//    cout << rotated_vector << "\n" << translation_matrix << "\n";
}

void Energy::automaticCentering() {
    solveRealPostiton(target_R, REAL_R_SIZE);
}

double Energy::getOriginPtzYaw() const {
    return origin_ptz_yaw;
}

double Energy::getOriginPtzPitch() const {
    return origin_ptz_pitch;
}

void Energy::setOriginPtzPitch(double originPtzPitch) {
    origin_ptz_pitch = originPtzPitch;
}

void Energy::setOriginPtzYaw(double originPtzYaw) {
    origin_ptz_yaw = originPtzYaw;
}