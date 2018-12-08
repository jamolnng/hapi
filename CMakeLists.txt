cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
project(hapi VERSION 0.1 LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Release)

function(get_include_dirs headers dirs)
    set (tlist "")
    foreach (_headerFile ${headers})
        get_filename_component(_dir ${_headerFile} PATH)
        list (APPEND tlist ${_dir})
    endforeach()
    set(${dirs} "${tlist}" PARENT_SCOPE)
endfunction()

macro(basic_program program_name src_dir)
    file(GLOB_RECURSE ${program_name}_SOURCES "src/*.cpp")
    file(GLOB_RECURSE ${program_name}_HEADERS "include/*.h")
    file(GLOB_RECURSE ${program_name}_SOURCES "${src_dir}src/*.cpp")
    file(GLOB_RECURSE ${program_name}_HEADERS "${src_dir}include/*.h")

    get_include_dirs("${${program_name}_HEADERS}" ${program_name}_INCLUDE_DIRS)

    add_executable(${program_name} ${${program_name}_SOURCES})
    target_include_directories(${program_name} PUBLIC ${${program_name}_INCLUDE_DIRS})
endmacro()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas")

basic_program(hapi "")
target_include_directories(hapi PUBLIC "/usr/include/spinnaker")
target_link_libraries(hapi wiringPi Spinnaker stdc++fs)

basic_program(hapi-config "tools/config/")
target_link_libraries(hapi-config stdc++fs)

basic_program(hapi-pmt-calibrate "tools/pmt_calibrate/")
target_link_libraries(hapi-pmt-calibrate stdc++fs)

install(TARGETS hapi hapi-config hapi-pmt-calibrate
        LIBRARY DESTINATION lib/
        RUNTIME DESTINATION bin/)
