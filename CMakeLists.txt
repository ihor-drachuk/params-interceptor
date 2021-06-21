cmake_minimum_required(VERSION 3.16)

project(interceptor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(interceptor src/main.cpp)
set_property(TARGET interceptor PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(interceptor PRIVATE Qt5::Core)

add_executable(testee src/main-testee.cpp)
set_property(TARGET testee PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET testee POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE_DIR:testee>/$<TARGET_FILE_NAME:testee>
    $<TARGET_FILE_DIR:interceptor>/$<TARGET_FILE_NAME:interceptor>_orig
    )
