#include "workspace.h"

void init();

Workspace workspace;

int main()
{
    init();
    workspace.run();
    return 0;
}

/**
* @brief 初始化函数, 读入配置文件
*/
void init()
{
    // workspace参数传入
    cv::FileStorage file_storage(PARAM_PATH, cv::FileStorage::READ);
    cv::FileNode workspace_node = file_storage["workspace"];
    workspace.SHOW_IMAGE = workspace_node["SHOW_IMAGE"];
    workspace.TRACKBAR = workspace_node["TRACKBAR"];
    workspace.RUNNING_TIME = workspace_node["RUNNING_TIME"];
    workspace.ENEMY_COLOR = workspace_node["ENEMY_COLOR"];
    workspace.MODE = workspace_node["MODE"];
    workspace.USE_CAMERA = workspace_node["USE_CAMERA"];
    workspace.SAVE_VIDEO = workspace_node["SAVE_VIDEO"];
    workspace.DEBUG_INFO = file_storage["DEBUG_INFO"];
    workspace.FRAME_WIDTH = file_storage["FRAME_WIDTH"];
    workspace.FRAME_HEIGHT = file_storage["FRAME_HEIGHT"];
    workspace.EXPOSURE_TIME = file_storage["EXPOSURE_TIME"];
    workspace.USE_SERIAL = file_storage["USE_SERIAL"];
    workspace.USE_CAN = file_storage["USE_CAN"];
    workspace.VIDEO_PATH = static_cast<std::string>(workspace_node["VIDEO_PATH"]);
    workspace.VIDEO_SAVED_PATH = static_cast<std::string>(workspace_node["VIDEO_SAVED_PATH"]);

    // 保存视频必须使用相机
    if (workspace.USE_CAMERA == 0 && workspace.SAVE_VIDEO == 1)
    {
        Debugger::error("wrong mode, SAVE_VIDEO need USE_CAMERA.",
                        __FILE__, __FUNCTION__, __LINE__);
        exit(1);
    }
    // 使用TRACKBAR必须展示图像
    if (workspace.SHOW_IMAGE == 0 && workspace.TRACKBAR == 1)
    {
        Debugger::error("wrong mode, TRACKBAR need SHOW_IMAGE.",
                        __FILE__, __FUNCTION__, __LINE__);
    }

    // 其它文件参数传入及初始化
    cv::FileNode arm_detect = workspace_node["arm_detect"];
    Armor::GAMMA_C = arm_detect["GAMMA_C"];
    Armor::GAMMA_G = arm_detect["GAMMA_G"];
    Armor::GAMMA_THRES = arm_detect["GAMMA_THRES"];
    CanNode::USE_CAN = file_storage["USE_CAN"];
    CanNode::DEBUG_INFO = file_storage["DEBUG_INFO"];
    workspace.armor_detector.init(file_storage);
    workspace.target_solver.init(file_storage);
    workspace.energy.init(file_storage);
    if (workspace.USE_SERIAL)
    {
        workspace.openSerialPort();
    }
    else if (workspace.USE_CAN != 2)
    {
        workspace.can_node.init();
    }
}