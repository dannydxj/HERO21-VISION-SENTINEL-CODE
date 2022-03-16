#include "mvcamera.h"

using namespace cv;
using namespace std;

MVCamera::MVCamera()
{
    is_open = false;
}

MVCamera::~MVCamera()
{
    close();
}

void MVCamera::open(int frame_width,
                    int frame_height,
                    int exposure_time,
                    double frame_speed,
                    double gamma,
                    int contrast)
{
    int status = -1;
    int cameraCounts = 1;
    int channel = 3;

    CameraSdkInit(0);

    // 枚举设备，并建立设备列表
    status = CameraEnumerateDevice(&tCameraEnumList, &cameraCounts);
    if (cameraCounts == 0)
    {
        throw CameraException("Enumerate camera failed.");
    }

    // 相机初始化。初始化成功后，才能调用任何其他相机相关的操作接口
    status = CameraInit(&tCameraEnumList, -1, -1, &hCamera);
    if (status != CAMERA_STATUS_SUCCESS)
    {
        throw CameraException("Init camera failed.");
    }

    // 获得相机的特性描述结构体。该结构体中包含了相机可设置的各种参数的范围信息。决定了相关函数的参数
    CameraGetCapability(hCamera, &tCapability);
    g_pRgbBuffer = (unsigned char *)malloc(
        tCapability.sResolutionRange.iWidthMax * tCapability.sResolutionRange.iHeightMax * 3);
    /* 让SDK进入工作模式，开始接收来自相机发送的图像数据。
       如果当前相机是触发模式，则需要接收到触发帧以后才会更新图像 */
    CameraPlay(hCamera);

    tSdkImageResolution imageResolution;
    CameraGetImageResolution(hCamera, &imageResolution);
    imageResolution.iIndex = 0XFF;
    imageResolution.iWidth = frame_width;
    imageResolution.iHeight = frame_height;
    // 设置图像的分辨率
    CameraSetImageResolution(hCamera, &imageResolution);
    // 设置为手动曝光
    CameraSetAeState(hCamera, 0);
    // 设置曝光
    CameraSetExposureTime(hCamera, exposure_time);
    // 设置帧率
    CameraSetFrameSpeed(hCamera, frame_speed);
    // 设置gamma
    CameraSetGamma(hCamera, gamma);
    // 设置对比度
    CameraSetContrast(hCamera, contrast);

    // 判断相机的通道个数
    if (tCapability.sIspCapacity.bMonoSensor)
    {
        channel = 1;
        // 8位单通道
        CameraSetIspOutFormat(hCamera, CAMERA_MEDIA_TYPE_MONO8);
    }
    else
    {
        channel = 3;
        // 8位单通道
        CameraSetIspOutFormat(hCamera, CAMERA_MEDIA_TYPE_BGR8);
    }

    is_open = true;
}

bool MVCamera::isOpen()
{
    return is_open;
}

void MVCamera::getImage(Mat &image)
{
    if (!is_open)
    {
        throw CameraException("Get image error. Camera is not opened.");
    }

    // 从相机的缓冲区中接受一幅图像
    if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 500) == CAMERA_STATUS_SUCCESS)
    {
        // 将图像转换为RGB图像
        CameraImageProcess(hCamera, pbyBuffer, g_pRgbBuffer, &sFrameInfo);
        // 使用原图像生成OpenCV中的Mat
        image = Mat(Size(sFrameInfo.iWidth, sFrameInfo.iHeight),
                    CV_8UC3,
                    g_pRgbBuffer);

        /* 在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer
           否则再次调用CameraGetImageBuffer时，程序将被挂起一直阻塞，
           直到其他线程中调用CameraReleaseImageBuffer来释放了buffer */
        CameraReleaseImageBuffer(hCamera, pbyBuffer);
    }
    else
    {
        throw CameraException("Get image failed. Camera is not opened.");
    }
}

void MVCamera::close()
{
    CameraUnInit(hCamera);
    free(g_pRgbBuffer);
    is_open = false;
}

// Mat MVCamera::getImage() {
//     if (!is_open) {
//         throw MVCameraException("Get image error. Camera is not opened.");
//     }

//     cv::Mat image;
//     // 从相机的缓冲区中接受一幅图像
//     if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 500) == CAMERA_STATUS_SUCCESS) {
//         // 将图像转换为RGB图像
//         CameraImageProcess(hCamera, pbyBuffer, g_pRgbBuffer, &sFrameInfo);
//         // 使用原图像生成OpenCV中的Mat
//         image = cv::Mat(Size(sFrameInfo.iWidth, sFrameInfo.iHeight),
//                         CV_8UC3,
//                         g_pRgbBuffer);

//         /* 在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer
//            否则再次调用CameraGetImageBuffer时，程序将被挂起一直阻塞，
//            直到其他线程中调用CameraReleaseImageBuffer来释放了buffer */
//         CameraReleaseImageBuffer(hCamera, pbyBuffer);
//     } else {
//         throw MVCameraException("Get image failed. Camera is not opened.");
//     }

// return image;
// }
