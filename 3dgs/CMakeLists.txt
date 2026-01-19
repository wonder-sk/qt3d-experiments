cmake_minimum_required(VERSION 3.16)
project(qt3d-splat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets 3DCore 3DRender 3DInput 3DExtras)

add_executable(qt3d-splat
    main.cpp
    gaussiansplat.h
    splatrenderer.h
)

target_link_libraries(qt3d-splat PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DInput
    Qt6::3DExtras
)
