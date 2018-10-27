cmake_minimum_required (VERSION 3.0)
project(HAPI)

file(GLOB HAPI_SOURCES "*.cpp")

include_directories("${PROJECT_BINARY_DIR}")

add_executable(hapi ${HAPI_SOURCES})

target_link_libraries(hapi wiringPi spinnaker)