#include "dhcamera.h"

DHCamera::DHCamera()
{
    is_open = false;
}

DHCamera::~DHCamera()
{
    if (m_pBufferRGB)
    {
        free(m_pBufferRGB);
        m_pBufferRGB = NULL;
    }
    if (m_hDevice)
    {
        GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
        GXCloseDevice(m_hDevice);
        GXCloseLib();
    }
}

void DHCamera::open(int frame_width,
                    int frame_height,
                    int exposure_time,
                    double frame_speed,
                    double gamma,
                    int contrast)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_OPEN_PARAM openParam;
    uint32_t nDeviceNum = 0;
    openParam.accessMode = GX_ACCESS_EXCLUSIVE;
    openParam.openMode = GX_OPEN_INDEX;
    openParam.pszContent = (char *)"1";

    /// 初始化lib
    emStatus = GXInitLib();
    if (emStatus != GX_STATUS_SUCCESS)
    {
        return;
    }
    /// 枚举相机设备
    emStatus = GXUpdateDeviceList(&nDeviceNum, 1000);
    if ((emStatus != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
    {
        // return;
        throw CameraException("Enumerate camera failed.");
    }
    /// 打开设备
    emStatus = GXOpenDevice(&openParam, &m_hDevice);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        // return;
        throw CameraException("Enumerate camera failed.");
    }

    /// 设置自动白平衡（OFF）、帧率模式设置
    GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_FRAME_RATE_MODE, GX_ACQUISITION_FRAME_RATE_MODE_ON);
    GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);
    // GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS);

    /// 设置图像大小、曝光时间
    GXSetInt(m_hDevice, GX_INT_WIDTH, frame_width);
    GXSetInt(m_hDevice, GX_INT_HEIGHT, frame_height);
    GXSetFloat(m_hDevice, GX_FLOAT_ACQUISITION_FRAME_RATE, frame_speed);

    /// 设置自动曝光（OFF）并添加手动曝光
    GXSetEnum(m_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
    GXSetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, exposure_time);

    /// 获取图像大小
    emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_nPayLoadSize);

    /// 获取所设置的gamma值
    emStatus = GXGetFloat(m_hDevice, GX_FLOAT_GAMMA_PARAM, &gamma);

    if ((emStatus != GX_STATUS_SUCCESS) || (m_nPayLoadSize <= 0))
    {
        return;
    }

    /// 判断是否支持
    bool m_bColorFilter;
    emStatus = GXIsImplemented(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_bColorFilter);

    if (m_bColorFilter)
    {
        GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_nPixelColorFilter);
    }

    m_pBufferRGB = (uchar *)malloc((size_t)(frame_width * frame_height * 3));
    if (!m_pBufferRGB)
    {
        return;
    }

    /// 发送获取开始命令
    emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        return;
    }

    is_open = true;
}

bool DHCamera::isOpen()
{
    return is_open;
}

void DHCamera::getImage(cv::Mat &image)
{
    if (!is_open)
    {
        return;
    }

    // 输入获取的图像参数
    GX_FRAME_DATA frame_data;

    /// 申请内存
    frame_data.pImgBuf = (uchar *)malloc((size_t)m_nPayLoadSize);
    if (!frame_data.pImgBuf)
    {
        return;
    }

    /// 获取一帧图像，异常睡眠
    while (GXGetImage(m_hDevice, &frame_data, 100) != GX_STATUS_SUCCESS)
    {
        sleep(1);
    }

    /// 成功获取相机图像，转换图像类型并变为Mat形式
    if (frame_data.nStatus == GX_FRAME_STATUS_SUCCESS)
    {
        DxRaw8toRGB24(frame_data.pImgBuf,
                      m_pBufferRGB,
                      (VxUint32)(frame_data.nWidth),
                      (VxUint32)(frame_data.nHeight),
                      RAW2RGB_NEIGHBOUR,
                      DX_PIXEL_COLOR_FILTER(m_nPixelColorFilter),
                      false);
        image = cv::Mat(cv::Size(frame_data.nWidth, frame_data.nHeight), CV_8UC3, m_pBufferRGB);
        // image_buffer_mutex.lock();
        // cv::cvtColor(image, image, CV_RGB2BGR);
        // image_buffer_mutex.unlock();
        /// 释放内存
        free(frame_data.pImgBuf);
    }
}

void DHCamera::close()
{
    if (m_pBufferRGB)
    {
        delete[] m_pBufferRGB;
        m_pBufferRGB = NULL;
    }
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
    emStatus = GXCloseDevice(m_hDevice);
    emStatus = GXCloseLib();
    m_hDevice = NULL;

    is_open = false;
}

// cv::Mat DHCamera::getImage()
// {
//     if (!is_open_)
//     {
//         return cv::Mat();
//     }
//     cv::Mat image;
//     GX_FRAME_DATA frame_data;
//     frame_data.pImgBuf = (uchar *)malloc((size_t)m_nPayLoadSize);
//     if (!frame_data.pImgBuf)
//     {
//         return cv::Mat();
//     }

//     while (GXGetImage(m_hDevice, &frame_data, 100) != GX_STATUS_SUCCESS)
//     {
//         sleep(1);
//     }
//     if (frame_data.nStatus == GX_FRAME_STATUS_SUCCESS)
//     {
//         DxRaw8toRGB24(frame_data.pImgBuf,
//                       m_pBufferRGB,
//                       (VxUint32)(frame_data.nWidth),
//                       (VxUint32)(frame_data.nHeight),
//                       RAW2RGB_NEIGHBOUR,
//                       DX_PIXEL_COLOR_FILTER(m_nPixelColorFilter),
//                       false);
//         image = cv::Mat(cv::Size(frame_data.nWidth, frame_data.nHeight), CV_8UC3, m_pBufferRGB);
//         cv::cvtColor(image, image, CV_RGB2BGR);
//         free(frame_data.pImgBuf);
//     }

//     return image;
// }
