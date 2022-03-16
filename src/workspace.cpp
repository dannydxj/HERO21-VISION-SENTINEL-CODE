#include "workspace.h"
#include "timer.h"
#include "util.h"

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

using namespace cv;
using namespace std;

Workspace::Workspace() = default;

Workspace::~Workspace()
{
    if (camera != nullptr)
        delete camera;
}

void Workspace::run()
{
    thread image_receiving_thread(&Workspace::imageReceivingFunc, this);
    thread image_processing_thread(&Workspace::imageProcessingFunc, this);
    thread message_communicating_thread(&Workspace::messageCommunicatingFunc, this);

    image_receiving_thread.join();
    image_processing_thread.join();
    message_communicating_thread.join();
}

void Workspace::imageReceivingFunc()
{
    VideoWriter writer(VIDEO_SAVED_PATH, VideoWriter::fourcc('M', 'J', 'P', 'G'),
                       30, Size(FRAME_WIDTH, FRAME_HEIGHT));
    VideoCapture cap(VIDEO_PATH);
    try
    {
        if (USE_CAMERA)
        {
            // autoCamera();
            camera->open(FRAME_WIDTH, FRAME_HEIGHT, EXPOSURE_TIME);

        }
    }
    catch (CameraException &e1)
    {
        cerr << "Camera error.\n";
        camera->close();
        sleep(1);
        for (int i = 0; i < 5; ++i)
        {
            try
            {
                camera->open(FRAME_WIDTH, FRAME_HEIGHT, EXPOSURE_TIME);
                if (camera->isOpen())
                {
                    break;
                }
            }
            catch (CameraException &e2)
            {
                cerr << "Try to open camera error." << endl;
                sleep(1);
            }
        }
        if (!camera->isOpen())
        {
            exit(1);
        }
    }

    while (true)
    {
        static Timer timer;
        timer.start();
        Mat image;
        if (USE_CAMERA)
        {
            cv::cvtColor(image, image, CV_RGB2BGR);
            if (image_buffer.size() < MAX_IMAGE_BUFFER_SIZE)
            {
                camera->getImage(image);
                image_buffer.emplace_back(image);
                if (SAVE_VIDEO == 1)
                {
                    writer.write(image);
                }
            }
        }
        else
        {
            cap >> image;
            if (image.empty())
            {
                cerr << "视频为空\n";
                exit(0);
            }
            if (image_buffer.size() < MAX_IMAGE_BUFFER_SIZE)
            {
                image_buffer.emplace_back(image);
            }
        }
        if (RUNNING_TIME)
        {
            timer.printTime("图像接收");
        }
        timer.stop();
    }
}

void Workspace::imageProcessingFunc()
{
    ostringstream ostr;
    Timer timer;
    while (true)
    {
        try
        {
            timer.start();

            if (image_buffer.empty())
            {
                continue;
            }

            image_buffer_mutex.lock();
            image_original = image_buffer.back();
            image_buffer.clear();
            image_buffer_mutex.unlock();

            setModeAndColor();

            //TODO RUNNING_TIME for each module.
            switch (read_pack.mode)
            {
            case Mode::MODE_ARMOR1:
            case Mode::MODE_ARMOR2:
            {
                bool has_target = armor_detector.run(image_original, read_pack.enemy_color, target_armor);
                if (has_target)
                {
                    // 解算成世界坐标
                    target_solver.run(target_armor, target);
                    send_pack.set(target);
                    // 解算云台角度
                    AngleSolver::run(target, 20, read_pack.ptz_pitch, send_pack.pred_yaw, send_pack.pred_pitch);
                }
                else
                {
                    send_pack.clear();
                }
                break;
            }
            case Mode::MODE_SMALLRUNE:
            {
                if (energy.run(image_original, read_pack.enemy_color, target))
                {
                    send_pack.set(target);
                    AngleSolver::run(target, 30, read_pack.ptz_pitch, send_pack.pred_yaw, send_pack.pred_pitch);
                    cout << "test rune here\n";
                    cout << energy.isCalibrated << '\n';
                    cout << send_pack;
                    if (!energy.isCalibrated &&
                        Util::equalZero(send_pack.pred_yaw) &&
                        Util::equalZero(send_pack.pred_pitch))
                    {
                        energy.isCalibrated = true;
                        energy.setOriginPtzPitch(read_pack.ptz_pitch);
                        energy.setOriginPtzYaw(read_pack.ptz_yaw);
                    }
                }
                else
                {
                    if (energy.isCalibrated)
                    {
                        send_pack.pred_pitch = energy.getOriginPtzPitch() - read_pack.ptz_pitch;
                        send_pack.pred_yaw = energy.getOriginPtzYaw() - read_pack.ptz_yaw;
                    }
                    else
                    {
                        send_pack.clear();
                        //send_pack.pred_pitch = 0.0;
                        //send_pack.pred_yaw = 0.0;
                    }
                }
                break;
            }
            default:
                send_pack.x = 0.0;
                send_pack.y = 0.0;
                send_pack.z = 0.0;
                send_pack.pred_yaw = 0.0;
                send_pack.pred_pitch = 0.0;
            }

            //if ((read_pack.mode == Mode::MODE_ARMOR1) || (read_pack.mode == Mode::MODE_ARMOR2)) {
            //    bool has_target = armor_detector.run(image_original, read_pack.enemy_color, target_armor);
            //    if (has_target) {
            //        // 解算成世界坐标
            //        target_solver.run(target_armor, target);
            //
            //        send_pack.set(target);
            //        // 解算云台角度
            //        AngleSolver::run(target, 20, read_pack.ptz_pitch, send_pack.pred_yaw, send_pack.pred_pitch);
            //    } else {
            //        send_pack.clear();
            //    }
            //}

            send_pack.time_delay = timer.getTime();
            if (RUNNING_TIME)
            {
                timer.printTime("图像预处理");
            }
            timer.stop();

            if (USE_SERIAL)
            {
                serial_port.sendData(send_pack);
            }
            else if (USE_CAN != 2)
            {
                can_node.send(send_pack);
            }

            if (DEBUG_INFO)
            {
                cout << target << read_pack << send_pack;
            }
            if (SHOW_IMAGE)
            {
                showImage();
            }
        }
        catch (Exception &e)
        {
            Debugger::warning(e.what(), __FILE__, __FUNCTION__, __LINE__);
        }
    }
}

void Workspace::messageCommunicatingFunc()
{
    if ((!USE_SERIAL) && (USE_CAN == 2))
        return;
    Timer timer;
    timer.start();
    while (true)
    {
        try
        {
            if (USE_SERIAL)
            {
                serial_port.readData(read_pack);
            }
            else
            {
                can_node.receive(read_pack);
            }

            if (RUNNING_TIME)
            {
                timer.printTime("Package receive");
            }
            timer.stop();
        }
        catch (SerialException &e1)
        {
            if (serial_port.isOpen())
                serial_port.close();
            sleep(1);
            for (int i = 0; i < 5; ++i)
            {
                try
                {
                    openSerialPort();
                    if (serial_port.isOpen())
                        break;
                }
                catch (SerialException &e2)
                {
                    Debugger::warning("Try to open serial port error.",
                                      __FILE__, __FUNCTION__, __LINE__);
                    sleep(1);
                }
            }
            if (!serial_port.isOpen())
            {
                Debugger::error("Re-open serial port failed.",
                                __FILE__, __FUNCTION__, __LINE__);
            }
        }
    }
}

void Workspace::openSerialPort()
{
    FileStorage file_storage(PARAM_PATH, FileStorage::READ);
    int count = 0;
    string port_name;

    while (count < 3)
    {
        try
        {
            port_name = "/dev/ttyUSB" + to_string(count++);
            serial_port.open(port_name, file_storage);
            if (serial_port.isOpen())
            {
                cout << "Open serial successfully in " << port_name << "." << endl;
                return;
            }
        }
        catch (SerialException &e)
        {
            cout << "Open " << port_name << " failed." << endl;
        }
    }
    throw SerialException("Open serial failed. Port is not in /dev/ttyUSB0-2");
}

void Workspace::setModeAndColor()
{
    // 设置工作模式
    switch (MODE)
    {
    case MODE_AUTO:
        break;
    case MODE_ARMOR1:
    case MODE_ARMOR2:
    case MODE_SMALLRUNE:
    case MODE_BIGRUNE:
    case MODE_HERO:
    case MODE_ENGINEER:
        send_pack.mode = read_pack.mode = MODE;
        break;
    case MODE_DEFAULT:
        Debugger::warning("default MODE.", __FILE__, __FUNCTION__, __LINE__);
        break;
    default:
        Debugger::error("error MODE.", __FILE__, __FUNCTION__, __LINE__);
        break;
    }

    // 设置敌方装甲板颜色
    switch (ENEMY_COLOR)
    {
    case COLOR_AUTO:
        break;
    case COLOR_RED:
    case COLOR_BLUE:
        read_pack.enemy_color = ENEMY_COLOR;
        break;
    case COLOR_DEFAULT:
        Debugger::warning("default COLOR", __FILE__, __FUNCTION__, __LINE__);
        break;
    default:
        Debugger::error("error COLOR", __FILE__, __FUNCTION__, __LINE__);
        break;
    }
}

void Workspace::showImage()
{
    string window_name_target = "with target";
    string window_name_proc = "processed binary";
    namedWindow(window_name_target, 1);
    namedWindow(window_name_proc, 1);

    copyMakeBorder(armor_detector.processed_image,
                   armor_detector.processed_image, 0,
                   FRAME_HEIGHT - armor_detector.processed_image.rows, 0,
                   FRAME_WIDTH - armor_detector.processed_image.cols,
                   BORDER_CONSTANT, Scalar(255, 255, 255));
    if (TRACKBAR)
        Debugger::trackbar(this->armor_detector, window_name_target, window_name_proc);

    Mat image_draw = image_original.clone();
    // cv::cvtColor(image_draw, image_draw, CV_RGB2BGR);
    Debugger::drawTypeValue(target, image_draw);
    Debugger::drawTypeValue(read_pack, image_draw);
    Debugger::drawTypeValue(send_pack, image_draw);
    Debugger::drawArmor(target_armor, image_draw);

    imshow(window_name_target, image_draw);
    imshow(window_name_proc, armor_detector.processed_image);
    // cv::cvtColor(image_original, image_original, CV_RGB2BGR);
    imshow("original", image_original);
    waitKey(1);
}

void Workspace::autoCamera()
{
    system("touch camera.txt");
    system("ls /dev/*Camera > camera.txt");
    fstream file("camera.txt", ios::in);
    char buf[100];
    file.getline(buf, 100);

    if (!strncmp(buf, "/dev/DHCamera", 13))
    {
        camera = new DHCamera();
    }
    else if (!strncmp(buf, "/dev/MVCamera", 13))
    {
        camera = new MVCamera();
    }
    else
    {
        Debugger::error("Match no camera！", __FILE__, __FUNCTION__, __LINE__);
    }
}
