#include "targetsolver.h"
#include "timer.h"

using namespace cv;
using namespace std;

TargetSolver::TargetSolver() = default;

TargetSolver::~TargetSolver() = default;

/**
 * @brief PnP解算初始化，引入内参矩阵和畸变矩阵
 */
void TargetSolver::init(const FileStorage &file_storage) {
    file_storage["camera_matrix"] >> CAMERA_MATRIX;
    file_storage["distortion_coeff"] >> DISTORTION_COEFF;
}

void TargetSolver::run(const Armor &armor, Target &target) {
    /* 如果判断装甲板无效，
       将各坐标值初始化为零 */
    if (!armor.is_valid) {
        target.x = 0;
        target.y = 0;
        target.z = 0;
        return;
    }
    // 通过宽高比判断装甲板类型，大or小
    double ratio = (armor.rotated_rect.size.width > armor.rotated_rect.size.height) ?
                   (armor.rotated_rect.size.width / armor.rotated_rect.size.height) :
                   (armor.rotated_rect.size.height / armor.rotated_rect.size.width);
    bool is_big_armor = ratio > 4.8;
    solvePnP4Points(armor.rotated_rect, is_big_armor);
    camera2ptzTransform(trans_mat, target);
}

// 空间坐标系（右手系）转换为相机坐标系下
void TargetSolver::solvePnP4Points(const RotatedRect &rect, const bool is_big_armor) {
    static Point2f vertices[4];
    Point2f left_up, left_down, right_up, right_down;
    rect.points(vertices);

    /* 四个点左上为起点顺时针进行排序，注意：y轴正方向向下
       将四个点的x坐标由小到大排序 */
    sort(vertices, vertices + 4, [](const Point2f &p1, const Point2f &p2) {
             return p1.x < p2.x;
         }
    );
    // 再根据四个点的y坐标确定相关位置
    if (vertices[0].y < vertices[1].y) {
        left_up = vertices[0];
        left_down = vertices[1];
    } else {
        left_up = vertices[1];
        left_down = vertices[0];
    }
    if (vertices[2].y < vertices[3].y) {
        right_up = vertices[2];
        right_down = vertices[3];
    } else {
        right_up = vertices[3];
        right_down = vertices[2];
    }

    // 将四个点按照顺序一一对应到变量名指定的位置上
    static vector<Point2f> points2d;
    points2d.clear();
    points2d.emplace_back(left_up);
    points2d.emplace_back(right_up);
    points2d.emplace_back(right_down);
    points2d.emplace_back(left_down);
  
    /* 根据装甲板类型的不同，
       将装甲板中心作为空间坐标系的原点，
       其四个顶点作为2D-3D的四对点 */
    static double half_w, half_h;
    if (is_big_armor) {
        half_w = HALF_BIG_ARMOR_WIDTH;
        half_h = HALF_BIG_ARMOR_HEIGHT;
    } else {
        half_w = HALF_SMALL_ARMOR_WIDTH;
        half_h = HALF_SMALL_ARMOR_HEIGHT;
    }

    // points3d中的点需和points2d中的点按顺序一一对应
    static vector<Point3f> points3d;
    points3d.clear();
    points3d.emplace_back(Point3f(-half_w, -half_h, 0));
    points3d.emplace_back(Point3f(half_w, -half_h, 0));
    points3d.emplace_back(Point3f(half_w, half_h, 0));
    points3d.emplace_back(Point3f(-half_w, half_h, 0));

    // 使用opencv内置函数PnP解算出相机坐标系 
    solvePnP(points3d, points2d, CAMERA_MATRIX, DISTORTION_COEFF, rotate_mat, trans_mat, false, ALGORITHM);

    // 暂时不需要使用旋转矩阵，需要用取消注释
    // Rodrigues(rotate_mat, rotate_mat);
}

// 以米作单位，offset补偿值源于机器人摄像头与云台所设定的原点间有物理距离
void TargetSolver::camera2ptzTransform(const Mat &camera_position, Target &ptz_position) {
    ptz_position.x = (camera_position.at<double>(0, 0) + X_OFFSET) / 1000;
    ptz_position.y = (camera_position.at<double>(1, 0) + Y_OFFSET) / 1000;
    ptz_position.z = (camera_position.at<double>(2, 0) + Z_OFFSET) / 1000;
}
