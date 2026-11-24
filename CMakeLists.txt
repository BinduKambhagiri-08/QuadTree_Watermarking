cmake_minimum_required(VERSION 3.15)
project(QuadTree_Watermarking VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
)

# Optional System OpenCV 4.x check
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS ">>> Found System OpenCV ${OpenCV_VERSION}")
    add_definitions(-DUSE_SYSTEM_OPENCV)
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(STATUS ">>> Using High-Performance Built-in OpenCV 4.x Compatibility Engine (No external OpenCV install required)")
endif()

# Core sources
set(CORE_SOURCES
    src/ImageProcessor.cpp
    src/QuadTree.cpp
    src/Watermark.cpp
    src/Metrics.cpp
    src/stb_impl.cpp
)

# Main Server Executable
add_executable(QuadTreeServer
    src/main.cpp
    src/ApiServer.cpp
    ${CORE_SOURCES}
)

if(WIN32)
    target_link_libraries(QuadTreeServer PRIVATE ws2_32 crypt32)
endif()

if(OpenCV_FOUND)
    target_link_libraries(QuadTreeServer PRIVATE ${OpenCV_LIBS})
endif()

# Tests
enable_testing()

add_executable(test_quadtree
    tests/test_quadtree.cpp
    ${CORE_SOURCES}
)
if(WIN32)
    target_link_libraries(test_quadtree PRIVATE ws2_32 crypt32)
endif()
if(OpenCV_FOUND)
    target_link_libraries(test_quadtree PRIVATE ${OpenCV_LIBS})
endif()
add_test(NAME TestQuadTree COMMAND test_quadtree)

add_executable(test_watermark
    tests/test_watermark.cpp
    ${CORE_SOURCES}
)
if(WIN32)
    target_link_libraries(test_watermark PRIVATE ws2_32 crypt32)
endif()
if(OpenCV_FOUND)
    target_link_libraries(test_watermark PRIVATE ${OpenCV_LIBS})
endif()
add_test(NAME TestWatermark COMMAND test_watermark)

add_executable(test_metrics
    tests/test_metrics.cpp
    ${CORE_SOURCES}
)
if(WIN32)
    target_link_libraries(test_metrics PRIVATE ws2_32 crypt32)
endif()
if(OpenCV_FOUND)
    target_link_libraries(test_metrics PRIVATE ${OpenCV_LIBS})
endif()
add_test(NAME TestMetrics COMMAND test_metrics)
