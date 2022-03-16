#ifndef HERORM2020_CANNODE_HPP
#define HERORM2020_CANNODE_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "base.h"
#include "types.h"

#define ID_NUM 1


// #if USE_CAN == 0
// #define UP "ip link set up can0"                                                                     // open device
// #define DOWN "ip link set down can0"                                                                 // close device
// #define COMMAND "ip link set can0 type can bitrate 1000000 dbitrate 2000000 berr-reporting on fd on" // enable and set bitrate
// #elif USE_CAN == 1
// #define UP "ip link set up can1"
// #define DOWN "ip link set down can1"
// #define COMMAND "ip link set can1 type can bitrate 1000000 dbitrate 2000000 berr-reporting on fd on"
// #endif


class CanNode
{
public:
    typedef struct sockaddr_can SockAddr_Can;
    typedef struct ifreq Ifreq;
    typedef struct can_filter Filter;
    typedef struct can_frame Frame; // send frame

private:
    SockAddr_Can addr_;
    Ifreq ifr_;
    Filter cfilter_[ID_NUM];
    int skt_; // socket
    std::string dev_name;

    // remained for multiple ids
    unsigned int id_rcv_[ID_NUM] = {0x301};
    unsigned int id_snd_[ID_NUM] = {0x302};


public:
    CanNode();
    ~CanNode();
    static int USE_CAN;
    static int DEBUG_INFO;
    bool init();
    bool send(const SendPack &send_pack);
    bool receive(ReadPack &read_pack);

private:
    bool unpack(const Frame &frame, ReadPack &read_pack);
};

#endif // HERORM2020_CANNODE_HPP
