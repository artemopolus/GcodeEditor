cmake_minimum_required(VERSION 3.0)
project(GcodeEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS PrintSupport REQUIRED)

include_directories(
    "src"
    "qcustomplot"
)

add_executable(gcodeeditor
    src/main.cpp
    src/mainwindow.cpp
    src/gcodeanalyzator.cpp
    src/captureparams.cpp
    src/mainwindow.ui
    src/captureparams.ui
    qcustomplot/qcustomplot.cpp
)

# https://www.qcustomplot.com/index.php/download

target_link_libraries(gcodeeditor PRIVATE Qt6::Widgets Qt6::PrintSupport)
