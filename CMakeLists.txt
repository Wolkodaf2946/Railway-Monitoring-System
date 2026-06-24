cmake_minimum_required(VERSION 3.21)

project(RailwayMonitoringSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(railway_monitor
    src/main.cpp
    src/core/DataManager.cpp
    src/core/DataManager.h
    src/models/Constants.h
    src/models/LogEntry.h
    src/ui/CollapsibleBox.cpp
    src/ui/CollapsibleBox.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/SensorTable.cpp
    src/ui/SensorTable.h
    src/ui/TrainDetailView.cpp
    src/ui/TrainDetailView.h
    src/ui/WagonView.cpp
    src/ui/WagonView.h
)

target_include_directories(railway_monitor PRIVATE src)

target_link_libraries(railway_monitor PRIVATE Qt6::Widgets)
