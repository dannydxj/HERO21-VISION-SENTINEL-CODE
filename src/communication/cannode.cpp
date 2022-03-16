#include "cannode.h"
int CanNode::USE_CAN = 2;
int CanNode::DEBUG_INFO = 0;

CanNode::CanNode()
{
}

CanNode::~CanNode()
{
    if (skt_)
    {
        close(skt_);
    }
}

bool CanNode::init()
{

    bool status = true;

    if (USE_CAN == 0)
    {
        dev_name = "can0";
        system("ip link set up can0");
        system("ip link set down can0");
        system("ip link set can0 type can bitrate 1000000 dbitrate 2000000 berr-reporting on fd on");
    }
    else if (USE_CAN == 1)
    {
        dev_name = "can1";
        system("ip link set up can1");
        system("ip link set down can1");
        system("ip link set can1 type can bitrate 1000000 dbitrate 2000000 berr-reporting on fd on");
    }

    if ((skt_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        std::cerr << dev_name << " socket error.\n";
        status = false;
    }
    strcpy(ifr_.ifr_name, dev_name.c_str());

    if (ioctl(skt_, SIOCGIFINDEX, &ifr_) < 0)
    {
        std::cerr << dev_name << " ioctl error.\n";
        status = false;
    }

    addr_.can_family = AF_CAN;
    addr_.can_ifindex = ifr_.ifr_ifindex;

    if (bind(skt_, (struct sockaddr *)&addr_, sizeof(addr_)) != 0) // bind socket and device can
    {
        std::cout << dev_name << "bind error.\n";
        status = false;
    }

    for (int i = 0; i < ID_NUM; ++i)
    {
        cfilter_[i].can_id = id_rcv_[i];     // receive frame id
        cfilter_[i].can_mask = CAN_SFF_MASK; // succeed when (reveived_id) & mask == (can_id) & mask
    }
    if (setsockopt(skt_, SOL_CAN_RAW, CAN_RAW_FILTER, &cfilter_, sizeof(cfilter_)) != 0)
    {

        std::cout << dev_name << " sockopt error.\n";
        status = false;
    }
    //    mode_ = 2;

    return status;
}

bool CanNode::send(const SendPack &send_pack)
{
    bool res = false;
    Frame frame{0};

    frame.can_dlc = 8;
    frame.can_id = id_snd_[0];

    /* first generation of auto aiming 
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

    /* first generation of auto aiming  */
    if (send_pack.mode == Mode::MODE_ARMOR1)
    {
        frame.data[0] = static_cast<int16_t>(send_pack.pred_yaw * 100) >> 8;
        frame.data[1] = static_cast<int16_t>(send_pack.pred_yaw * 100);
        frame.data[2] = static_cast<int16_t>(send_pack.pred_pitch * 100) >> 8;
        frame.data[3] = static_cast<int16_t>(send_pack.pred_pitch * 100);
    }
    /* second generation of auto aiming  */
    else if (send_pack.mode == Mode::MODE_ARMOR2)
    {
        frame.data[0] = static_cast<int16_t>(send_pack.x * 1000) >> 8;
        frame.data[1] = static_cast<int16_t>(send_pack.x * 1000);
        frame.data[2] = static_cast<int16_t>(send_pack.y * 1000) >> 8;
        frame.data[3] = static_cast<int16_t>(send_pack.y * 1000);
        frame.data[4] = static_cast<int16_t>(send_pack.z * 1000) >> 8;
        frame.data[5] = static_cast<int16_t>(send_pack.z * 1000);
        frame.data[6] = (uint8_t)send_pack.time_delay; // time delay
    }

    // checksum
    for (int i = 0; i <= 6; ++i)
        frame.data[7] += frame.data[i];

    int nbytes = write(skt_, &frame, sizeof(Frame));
    if (nbytes < 0)
    {
        std::cerr << "can raw socket write.\n";
        res = false;
    }
    else if (nbytes < sizeof(Frame))
    {
        std::cerr << "write: incomplete frame.\n";
        res = false;
    }
    else if (nbytes == sizeof(Frame))
    {
        if (DEBUG_INFO)
        {
            for (int i = 0; i < 8; ++i)
                printf("SEND data[%d]: %x\n", i, frame.data[i]);
        }

        res = true;
    }

    return res;
}

bool CanNode::receive(ReadPack &read_pack)
{
    bool res = false;

    Frame frame;

    int nbytes = read(skt_, &frame, sizeof(Frame));

    if (nbytes < 0)
    {
        std::cerr << "can raw socket read.\n";
        res = false;
    }
    else if (nbytes < sizeof(Frame))
    {
        std::cerr << "read: incomplete frame.\n";
        res = false;
    }
    else if (nbytes == sizeof(Frame))
    {
        unpack(frame, read_pack);
        res = true;
    }

    return res;
}

bool CanNode::unpack(const Frame &frame, ReadPack &read_pack)
{
    /* first generation of auto aiming 
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

    std::vector<uint8_t> frame_heads{0xA1, 0xB1, 0xA2, 0xB2, 0xA3, 0xB3, 0xA6, 0xB6};
    auto head_iter = std::find(frame_heads.begin(), frame_heads.end(), frame.data[0]);
    if (head_iter == frame_heads.end())
    {
        if (DEBUG_INFO)
        {
            printf("CAN read head invalid: %X", frame.data[0]);
        }
        return false;
    }

    if (DEBUG_INFO)
    {
        for (int i = 0; i < 8; ++i)
        {
            printf("READ data[%d]: %X\n", i, frame.data[i]);
        }
    }

    // checksum
    uint8_t checksum = 0;
    for (int i = 1; i <= 6; ++i)
        checksum += frame.data[i];
    if (frame.data[7] != checksum)
    {
        if (DEBUG_INFO)
        {
            printf("READ data checksum error: %X\n", checksum);
        }
        return false;
    }

    switch (frame.data[0])
    {
    /* first generation of auto aiming */
    case 0xA1:
        read_pack.mode = Mode::MODE_ARMOR1;
        read_pack.enemy_color = 0; // red color
        break;
    case 0xB1:
        read_pack.mode = Mode::MODE_ARMOR1;
        read_pack.enemy_color = 1; // blue color
        break;
    /* first generation of auto aiming */
    case 0xA2:
        read_pack.mode = Mode::MODE_SMALLRUNE;
        read_pack.enemy_color = 0; // red color;
        break;
    case 0xB2:
        read_pack.mode = Mode::MODE_SMALLRUNE;
        read_pack.enemy_color = 1; // blue color;
        break;
    case 0xA3:
        read_pack.mode = Mode::MODE_BIGRUNE;
        read_pack.enemy_color = 0; // red color;
        break;
    case 0xB3:
        read_pack.mode = Mode::MODE_BIGRUNE;
        read_pack.enemy_color = 0; // blue color;
        break;
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

    if (read_pack.mode == Mode::MODE_ARMOR1)
    {
        int16_t temp_yaw = (static_cast<int16_t>(frame.data[1]) << 8) +
                           static_cast<int16_t>(frame.data[2]);
        int16_t temp_pitch = (static_cast<int16_t>(frame.data[3]) << 8) +
                             static_cast<int16_t>(frame.data[4]);
        read_pack.ptz_pitch = static_cast<double>(temp_pitch) * 0.01;
        read_pack.ptz_yaw = static_cast<double>(temp_yaw) * 0.01;
        read_pack.bullet_speed = static_cast<double>(frame.data[5]) * 0.1 + 5;
        if (DEBUG_INFO)
        {
            printf("READ ptz_pitch:%lf ptz_yaw:%lf\n", read_pack.ptz_pitch, read_pack.ptz_yaw);
            printf("READ mode:%d color:%d\n", read_pack.mode, read_pack.enemy_color);
            printf("READ speed:%lf\n", read_pack.bullet_speed);
        }
    }
    else if (read_pack.mode == Mode::MODE_ARMOR2)
    {
        // TODO
    }

    return true;
}
