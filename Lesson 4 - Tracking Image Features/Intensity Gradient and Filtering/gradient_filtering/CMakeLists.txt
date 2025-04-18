cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_FLAGS "-Wall" "-pedantic")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")

project(camera_fusion)

find_package(OpenCV 4.1 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

# Executables for exercises
add_executable (gaussian_smoothing src/gaussian_smoothing.cpp)
target_link_libraries (gaussian_smoothing ${OpenCV_LIBRARIES})

add_executable (gradient_sobel src/gradient_sobel.cpp)
target_link_libraries (gradient_sobel ${OpenCV_LIBRARIES})

add_executable (magnitude_sobel src/magnitude_sobel.cpp)
target_link_libraries (magnitude_sobel ${OpenCV_LIBRARIES})
