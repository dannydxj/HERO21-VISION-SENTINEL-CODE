#include "serialport.h"
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

SerialPort::SerialPort()
{
    fd = -1;
    is_open = false;
}

SerialPort::SerialPort(const string &port_name,
                       long baud_rate,
                       int byte_size,
                       int parity,
                       int stop_bit,
                       int flow_controconst) : port_name(port_name), baud_rate(baud_rate),
                                               byte_size(byte_size),
                                               parity(parity),
                                               stop_bit(stop_bit),
                                               flow_control(flow_control),
                                               is_open(false)
{
}

SerialPort::~SerialPort()
{
    close();
}

void SerialPort::open(cv::FileStorage &file_storage)
{

    DEBUG_INFO = file_storage["DEBUG_INFO"];

    if (port_name.empty())
    {
        throw SerialException("Open port failed. Port name is empty.");
    }

    if (is_open)
    {
        throw SerialException("Open port failed. Port is already opened.");
    }

    fd = ::open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd == -1)
    {
        throw SerialException("Open port failed. Bad file description.");
    }

    reconfigurePort();
    is_open = true;
}

void SerialPort::open(const string &port_name,const cv::FileStorage &file_storage,
                      long baud_rate,
                      int byte_size,
                      int parity,
                      int stop_bit,
                      int flow_control)
{
    DEBUG_INFO = file_storage["DEBUG_INFO"];
    this->port_name = port_name;
    this->baud_rate = baud_rate;
    this->byte_size = byte_size;
    this->parity = parity;
    this->stop_bit = stop_bit;
    this->flow_control = flow_control;

    if (port_name.empty())
    {
        throw SerialException("Open port failed. Port name is empty.");
    }

    if (is_open)
    {
        throw SerialException("Open port failed. Port is already opened.");
    }

    fd = ::open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd == -1)
    {
        throw SerialException("Open port failed. Bad file description.");
    }

    reconfigurePort();
    is_open = true;
}

bool SerialPort::isOpen()
{
    return is_open;
}

void SerialPort::close()
{
    if (is_open)
    {
        if (fd < 0)
        {
            throw SerialException("Close port failed. Bad file(open) description.");
        }
        else
        {
            int ret = ::close(fd);
            if (ret == 0)
            {
                fd = -1;
                is_open = false;
            }
            else
            {
                throw SerialException("Close port failed. Bad file(close) description.");
            }
        }
    }
    else
    {
        throw SerialException("Close port failed. Port is not opened.");
    }
}

void SerialPort::setBaudRate(long baud_rate)
{
    if (is_open)
    {
        baud_rate = baud_rate;
        reconfigurePort();
    }
    else
    {
        throw SerialException("Set baud rate failed. Port is not opened.");
    }
}

long SerialPort::getBaudRate()
{
    return baud_rate;
}

void SerialPort::setByteSize(int byte_size)
{
    if (is_open)
    {
        byte_size = byte_size;
        reconfigurePort();
    }
    else
    {
        throw SerialException("Set byte size failed. Port is not opened.");
    }
}

int SerialPort::getByteSize()
{
    return byte_size;
}

void SerialPort::setParity(int parity)
{
    if (is_open)
    {
        parity = parity;
        reconfigurePort();
    }
    else
    {
        throw SerialException("Set parity failed. Port is not opened.");
    }
}

int SerialPort::getParity()
{
    return parity;
}

void SerialPort::setStopBit(int stop_bit)
{
    if (is_open)
    {
        stop_bit = stop_bit;
        reconfigurePort();
    }
    else
    {
        throw SerialException("Set stop bit failed. Port is not opened.");
    }
}

int SerialPort::getStopBit()
{
    return stop_bit;
}

void SerialPort::setFlowControl(int flow_control)
{
    if (is_open)
    {
        flow_control = flow_control;
        reconfigurePort();
    }
    else
    {
        throw SerialException("Set flow control failed. Port is not opened.");
    }
}

int SerialPort::getFlowControl()
{
    return flow_control;
}

void SerialPort::reconfigurePort() const
{
    if (fd == -1)
    {
        throw SerialException("Configure port failed. Bad file description.");
    }

    struct termios options;

    if (::tcgetattr(fd, &options) < 0)
    {
        throw SerialException("tcgetattr error.");
    }

    options.c_cflag |= (tcflag_t)(CLOCAL | CREAD);
    options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    options.c_oflag &= (tcflag_t) ~(OPOST);
    options.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK | ISTRIP | INPCK);

    // setup baud rate
    speed_t baud;
    switch (baud_rate)
    {
    case 0:
        baud = B0;
        break;
    case 50:
        baud = B50;
        break;
    case 75:
        baud = B75;
        break;
    case 110:
        baud = B110;
        break;
    case 134:
        baud = B134;
        break;
    case 150:
        baud = B150;
        break;
    case 200:
        baud = B200;
        break;
    case 300:
        baud = B300;
        break;
    case 600:
        baud = B600;
        break;
    case 1200:
        baud = B1200;
        break;
    case 1800:
        baud = B1800;
        break;
    case 2400:
        baud = B2400;
        break;
    case 4800:
        baud = B4800;
        break;
    case 9600:
        baud = B9600;
        break;
    case 19200:
        baud = B19200;
        break;
    case 38400:
        baud = B38400;
        break;
    case 57600:
        baud = B57600;
        break;
    case 115200:
        baud = B115200;
        break;
    case 230400:
        baud = B230400;
        break;
    case 460800:
        baud = B460800;
        break;
    case 576000:
        baud = B576000;
        break;
    case 921600:
        baud = B921600;
        break;
    default:
        printf("Invalid baud rate. Input baud rate is %ld.\n", baud_rate);
        throw SerialException("Set baud rate failed.");
        break;
    }
    ::cfsetispeed(&options, baud);
    ::cfsetospeed(&options, baud);

    // setup byte size
    options.c_cflag &= (tcflag_t)~CSIZE;
    switch (byte_size)
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        printf("Invalid byte size. Input byte size is %d.\n", byte_size);
        throw SerialException("Set byte size failed.");
        break;
    }

    // setup parity
    switch (parity)
    {
    case 0:
        options.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
        break;
    case 1:
        options.c_cflag |= (PARENB | PARODD);
        break;
    case 2:
        options.c_cflag &= (tcflag_t) ~(PARODD);
        options.c_cflag |= PARENB;
        break;
    default:
        printf("Invalid parity. Input parity is %d.\n", parity);
        throw SerialException("Set parity failed.");
        break;
    }

    // setup stop bit
    switch (stop_bit)
    {
    case 1:
        options.c_cflag &= (tcflag_t)~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    case 3:
        options.c_cflag |= CSTOPB;
        break; // one point five
    default:
        printf("Invalid stop bit. Input stop bit is %d.\n", stop_bit);
        throw SerialException("Set stop bit failed.");
        break;
    }

    // setup flow control
    options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY);
    options.c_cflag &= (tcflag_t)~CRTSCTS;
    switch (flow_control)
    {
    case 0:
        break;
    case 1:
        options.c_iflag |= (IXON | IXOFF);
        break;
    case 2:
        options.c_cflag |= CRTSCTS;
        break;
    default:
        printf("Invalid flow control. Input flow control is %d.\n", flow_control);
        throw SerialException("Set flow control failed.");
        break;
    }

    if (::tcsetattr(fd, TCSANOW, &options) != 0)
    {
        throw SerialException("Set port failed.");
    }
}

void SerialPort::sendData(const SendPack &send_pack)
{
    if (!is_open)
    {
        throw SerialException("Send data failed. Port is not opened.");
    }

    /** communication protocol 
     * first generation of auto aiming
     * 0: yaw high
     * 1: yaw low
     * 2: pitch high
     * 3: pitch low
     * 4: 0
     * 5: 0
     * 6: 0
     * 7: checksum
     *
     * second generation of auto aiming
     * 0: x high
     * 1: x low
     * 2: y high
     * 3: y low
     * 4: z high
     * 5: z low
     * 6: time delay
     * 7: checksum
     */

    uint8_t send_bytes[8] = {0};
    if (send_pack.mode == Mode::MODE_ARMOR1)
    { // first generation of auto aiming
        // pack yaw & pitch
        // yaw
        send_bytes[0] = static_cast<uint16_t>(send_pack.pred_yaw * 100) >> 8;
        send_bytes[1] = static_cast<uint8_t>(send_pack.pred_yaw * 100);
        // pitch
        send_bytes[2] = static_cast<uint16_t>(send_pack.pred_pitch * 100) >> 8;
        send_bytes[3] = static_cast<uint8_t>(send_pack.pred_pitch * 100);
    }
    else if (send_pack.mode == Mode::MODE_ARMOR2)
    { // second generation of auto aiming
        // pack x, y, z, delay time
        // x
        send_bytes[0] = static_cast<uint8_t>(send_pack.x * 1000) >> 8;
        send_bytes[1] = static_cast<uint8_t>(send_pack.x * 1000);
        // y
        send_bytes[2] = static_cast<uint8_t>(send_pack.y * 1000) >> 8;
        send_bytes[3] = static_cast<uint8_t>(send_pack.y * 1000);
        // z
        send_bytes[4] = static_cast<uint8_t>(send_pack.z * 1000) >> 8;
        send_bytes[5] = static_cast<uint8_t>(send_pack.z * 1000);
        // delay time
        send_bytes[6] = static_cast<uint8_t>(send_pack.time_delay);
    }

    // checksum
    for (int i = 0; i <= 6; ++i)
    {
        send_bytes[7] += send_bytes[i];
    }

    if (::write(fd, send_bytes, 8) == 8)
    {
        if (DEBUG_INFO)
        {
            printf("Send successfully.\n");
            for (int i = 0; i < 8; ++i)
                printf("%x\n", send_bytes[i]);
        }
    }
    else
    {
        throw SerialException("Send data failed.");
    }
}

bool SerialPort::readData(ReadPack &read_pack)
{
    if (!is_open)
    {
        throw SerialException("Read data failed. Port is not opened.");
    }

    /** communication protocol 
     * first generation of auto aiming
     * 0: frame head --- 0xA1 or 0xB1
     * 1: yaw high
     * 2: yaw low
     * 3: pitch high
     * 4: pitch low
     * 5: bullet speed
     * 6: 0
     * 7: checksum
     *
     * second generation of auto aiming
     * 0: frame head --- 0xA6 or 0xB6
     * 1-7: all 0
     */
    uint8_t read_bytes[8] = {0};

    // 所有候选帧头
    std::vector<uint8_t> frame_heads{0xA1, 0xB1, 0xA6, 0xB6};
    auto head_iter = std::find(frame_heads.begin(), frame_heads.end(), read_bytes[0]);
    // while frame head not found
    while (frame_heads.end() == head_iter)
    {
        // 所有数据位前移
        for (int i = 0; i < 7; ++i)
            read_bytes[i] = read_bytes[i + 1];
        // 新读取一位数据
        if (::read(fd, &read_bytes[7], 1) == 1)
        {
            // printf("Not read package. New byte: %x\n", send_bytes[8]);
        }
        else
        {
            //throw SerialException("Read data failed.");
        }
        head_iter = std::find(frame_heads.begin(), frame_heads.end(), read_bytes[0]);
    }

    if (DEBUG_INFO)
    {
        for (int i = 0; i < 8; ++i)
        {
            printf("READ data[%d]: %X\n", i, read_bytes[i]);
        }
    }

    // checksum
    uint8_t checksum = 0;
    for (int i = 1; i <= 6; ++i)
        checksum += read_bytes[i];
    if (read_bytes[7] != checksum)
    {
        printf("SERIAL READ checksum error: %X\n", checksum);
        return false;
    }

    // exrtact color & mode from frame head
    switch (read_bytes[0])
    {
    // first generation of auto aiming
    case 0xA1:
        read_pack.mode = Mode::MODE_ARMOR1;
        read_pack.enemy_color = 0; // red color
        break;
    case 0xB1:
        read_pack.mode = Mode::MODE_ARMOR1;
        read_pack.enemy_color = 1; // blue color
        break;
        // second generation of auto aiming
    case 0xA6:
        read_pack.mode = Mode::MODE_ARMOR2;
        read_pack.enemy_color = 0; // red color
        break;
    case 0xB6:
        read_pack.mode = Mode::MODE_ARMOR2;
        read_pack.enemy_color = 1; // blue color
        break;
    default:
        return false;
    }

    // extract yaw, pitch, bullet speed
    if (read_pack.mode == Mode::MODE_ARMOR1)
    {
        uint16_t temp_yaw = (static_cast<uint16_t>(read_bytes[1]) << 8) +
                            static_cast<uint16_t>(read_bytes[2]);
        uint16_t temp_pitch = (static_cast<uint16_t>(read_bytes[3]) << 8) +
                              static_cast<uint16_t>(read_bytes[4]);
        read_pack.ptz_pitch = static_cast<double>(temp_pitch) * 0.01;
        read_pack.ptz_yaw = static_cast<double>(temp_yaw) * 0.01;
        read_pack.bullet_speed = static_cast<double>(read_bytes[5]) * 0.1 + 5;

        if (DEBUG_INFO)
        {
            printf("SERIAL READ ptz_pitch:%lf ptz_yaw:%lf\n", read_pack.ptz_pitch, read_pack.ptz_yaw);
            printf("SERIAL READ mode:%d color:%d\n", read_pack.mode, read_pack.enemy_color);
            printf("SERIAL READ speed:%lf\n", read_pack.bullet_speed);
            cout << "Read successfully!\n";
        }
    }
    else if (read_pack.mode = Mode::MODE_ARMOR2)
    {
    }

    return true;
}
