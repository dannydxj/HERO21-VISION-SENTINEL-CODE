# HERO21-VISION-SENTINEL-CODE

HERO-RoboMaster 团队 2021 赛季哨兵视觉代码。

项目地址：https://github.com/dannydxj/HERO21-VISION-SENTINEL-CODE

负责人：

- 董行健  [![img](https://img.shields.io/badge/github-dannydxj-green.svg?logo=github)](https://github.com/dannydxj)
- 朱纹轩  [![img](https://img.shields.io/badge/github-skywalker--dell-green.svg?logo=github)](https://github.com/skywalker-dell)

> 整体代码框架项目地址：https://github.com/skywalker-dell/HERO21-VISION-CODE

## 环境依赖

- Ubuntu 16.04 及以上
- OpenCV 3.4 & OpenCV Contrib 3.4
- CMake 3.5.1 及以上

## 编译运行

```sh
git clone ${this project}
cd HERO21-VISION-CODE
mkdir build
cd build
cmake ..
make
./HERORM2021
```

## 项目结构说明

```
.
├── CMakeLists.txt
├── README.md
├── monitor.sh
├── param
│   └── param.xml
└── src
    ├── armor_detect
    │   ├── armor
    │   │   ├── armor.cpp
    │   │   └── armor.h
    │   ├── armordetector.cpp
    │   ├── armordetector.h
    │   └── classifier
    │       ├── classifier.cpp
    │       ├── classifier.h
    │       └── darknet
    ├── camera
    │   ├── camera.h
    │   ├── dhcamera
    │   └── mvcamera
    ├── communication
    │   ├── cannode.cpp
    │   ├── cannode.h
    │   ├── serialport.cpp
    │   └── serialport.h
    ├── energy
    │   ├── energy.cpp
    │   └── energy.h
    ├── main.cpp
    ├── target_solve
    │   ├── anglesolver.cpp
    │   ├── anglesolver.h
    │   ├── targetsolver.cpp
    │   └── targetsolver.h
    ├── util
    │   ├── base.h
    │   ├── debugger
    │   │   ├── debugger.cpp
    │   │   └── debugger.h
    │   ├── timer
    │   │   ├── timer.cpp
    │   │   └── timer.h
    │   ├── types.h
    │   ├── util.cpp
    │   └── util.h
    ├── workspace.cpp
    └── workspace.h
```

# 模块介绍

## `param`

配置文件。

## `armor_detect`

自瞄系统，包括装甲板类、装甲板检测、数字识别、运动预测、数字识别等多个模块。

## `camera`

对迈德威视相机 API 的再封装。

## `communication`

can 通信封装。

## `energy`

能量机关模块，在哨兵上不使用。

## `target_solve`

坐标解算模块，用于计算装甲板的三维坐标。

## `util`

常用的工具代码。

## `workspace`

主程序流程控制模块，其中包含对三个线程的调度：

- 通信线程
- 图像接收线程
- 图像处理线程

