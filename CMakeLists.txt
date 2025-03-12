cmake_minimum_required(VERSION 3.16)
project(my_project VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(ONNX_WRAPPER_DIR "${CMAKE_SOURCE_DIR}/onnx_inference_wrapper")
set(ONNXRUNTIME_ROOT "/home/wsfyy/onnxruntime-1.15.1")
set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT}/include")
set(ONNXRUNTIME_LIBRARY "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so.1.15.1")
set(MODEL_DIR "${CMAKE_SOURCE_DIR}/models")
set(MODEL_FILE "RealESRGAN_x4plus.onnx")

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Gui OpenGL)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui OpenGL)

file(GLOB_RECURSE CPP_SOURCES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.ui"
    "${ONNX_WRAPPER_DIR}/*.cpp"

)
file(GLOB_RECURSE HEADER_FILES
    "${INCLUDE_DIR}/*.hpp"
    "${INCLUDE_DIR}/*.h"
    "${ONNX_WRAPPER_DIR}/*.hpp"
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(my_project MANUAL_FINALIZATION
    ${CPP_SOURCES}
    ${HEADER_FILES}
)
else()
    add_executable(my_project
    ${CPP_SOURCES}
    ${HEADER_FILES}
)
endif()

add_custom_command(TARGET my_project PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/models"
    COMMENT "Creating model directory"
)
add_custom_command(TARGET my_project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${MODEL_DIR}/${MODEL_FILE}"
        "${CMAKE_BINARY_DIR}/models/${MODEL_FILE}"
    COMMENT "Copying model file"
)

target_include_directories(my_project PRIVATE
    ${SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${INCLUDE_DIR}
    ${ONNX_WRAPPER_DIR}
    ${OPENGL_INCLUDE_DIR}
    "${ONNXRUNTIME_INCLUDE_DIR}"
)

target_link_libraries(my_project PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::OpenGL
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARIES}
    "${ONNXRUNTIME_LIBRARY}"
    -Wl,--as-needed
    -lstdc++
    -Wl,--no-as-needed
)

install(TARGETS my_project
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(my_project)
endif()
