cmake_minimum_required(VERSION 3.20)
project(AurelHouse LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(AurelHouse main.cpp)
target_compile_features(AurelHouse PRIVATE cxx_std_17)
target_link_libraries(AurelHouse PRIVATE Qt6::Widgets)