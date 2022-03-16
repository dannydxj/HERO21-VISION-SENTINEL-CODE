cmake_minimum_required(VERSION 3.5.1)
project(HERORM2021)

set(CMAKE_CXX_STANDARD 11)

# 设置第三方库文件夹位置
set(OPENCV_DIR /usr/local/share/OpenCV)

# 搜索库
find_package(OpenCV REQUIRED)

# 添加源文件
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/armor_detect/armordetector.cpp
        src/armor_detect/armor/armor.cpp
        src/armor_detect/classifier/classifier.cpp
        src/camera/mvcamera/mvcamera.cpp
        src/camera/dhcamera/dhcamera.cpp
        src/communication/serialport.cpp
        src/communication/cannode.cpp
        src/target_solve/anglesolver.cpp
        src/target_solve/targetsolver.cpp
        src/util/timer/timer.cpp
        src/util/debugger/debugger.cpp
        src/util/util.cpp
        src/energy/energy.cpp
        src/workspace.cpp)

# 添加头文件目录
include_directories(
        ./src/
        ./src/armor_detect
        ./src/armor_detect/armor
        ./src/armor_detect/classifier
        ./src/armor_detect/classifier/darknet/include
        ./src/camera/
        ./src/camera/dhcamera
        ./src/camera/mvcamera
        ./src/communication
        ./src/target_solve
        ./src/util
        ./src/util/timer
        ./src/util/debugger
        ./src/energy
        ${OpenCV_INCLUDE_DIRS})

# 添加需要链接的库文件目录
link_directories(./src/armor_detect/classifier/darknet/)

# 添加链接库
target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBRARIES}
        libdarknet.so
        -pthread
        -lMVSDK
        -lgxiapi
        -fopenmp
        /lib/libMVSDK.so)

