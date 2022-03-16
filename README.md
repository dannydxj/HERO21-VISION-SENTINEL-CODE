# HERO21-VISION-SENTINEL-CODE

HERO-RoboMaster 团队 2021 赛季哨兵视觉代码。

项目地址：

负责人：

- 董行健  [![img](https://img.shields.io/badge/github-dannydxj-green.svg?logo=github)](https://github.com/dannydxj)
- 朱纹轩  [![img](https://img.shields.io/badge/github-skywalker--dell-green.svg?logo=github)](https://github.com/skywalker-dell)

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
├── Config								# 配置文件
└── src
    ├── aimbot							# 自瞄
    │   ├── antigyro					# 反陀螺
    │   ├── armor						# 装甲板类
    │   ├── armor_detect				# 装甲板检测
    │   │   ├── findlightbars
    │   │   ├── grade_armors
    │   │   ├── image_process
    │   │   └── match_armors
    │   ├── armor_predict				# 运动预测
    │   ├── classifier					# 数字识别
    │   │   ├── classifier_dk
    │   │   │   └── darknet
    │   │   └── classifier_sj
    │   │       └── para
    │   ├── others
    │   │   ├── init
    │   │   └── roi
    │   └── pick_target					# 装甲板选择模式
    │       ├── search_armor
    │       └── track_armor
    ├── camera							# 相机
    │   └── mvcamera
    ├── communication					# 通信
    │   ├── can
    │   └── can_to_debug
    ├── hit_sentinel					# 攻击哨兵模式
    ├── runesolver						# 能量机关模式
    │   ├── fit_rune_motion
    │   ├── runedetector
    │   └── runepredictor
    ├── target_solve					# 坐标解算
    ├── tools							# 工具
    │   ├── debugger
    │   ├── macros
    │   ├── queues
    │   ├── timer
    │   └── util_func
    └── workspace						# 主程序流程控制模块
        ├── commu_thread				# 通信线程
        │   ├── receive_thread
        │   └── send_thread
        ├── get_img_thread				# 图像接收线程
        ├── img_process_thread			# 图像处理线程
        │   ├── armor_func
        │   ├── rune_func
        │   └── sentinel_func
        └── others
```

# 模块介绍

## `Config`

配置文件说明

## `aimbot`

自瞄系统，包括装甲板类、装甲板检测、数字识别、运动预测、数字识别等多个模块。

## `camera`

对迈德威视相机 API 的再封装。

## `communication`

can 通信封装。

## `hit_sentinel`

该模块用于定点击打轨道上运动的哨兵。

## `runesolver`

能量机关模块，包括椭圆拟合、能量机关装甲板识别和能量机关运动预测等多个模块。

## `target_solve`

坐标解算模块，用于计算装甲板的三维坐标。

## `workspace`

主程序流程控制模块，其中包含对三个线程的调度：

- 通信线程
- 图像接收线程
- 图像处理线程

## `tools`

常用的工具代码。
