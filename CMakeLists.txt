cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
project(hapi CXX)

function(get_include_dirs headers dirs)
    set (tlist "")
    foreach (_headerFile ${headers})
        get_filename_component(_dir ${_headerFile} PATH)
        list (APPEND tlist ${_dir})
    endforeach()
    set(${dirs} "${tlist}" PARENT_SCOPE)
endfunction()

macro(basic_program program_name src_dir)
    file(GLOB_RECURSE ${program_name}_SOURCES "${src_dir}src/*.cpp")
    file(GLOB_RECURSE ${program_name}_HEADERS "${src_dir}include/*.h")

    get_include_dirs("${${program_name}_HEADERS}" ${program_name}_INCLUDE_DIRS)

    add_executable(${program_name} ${${program_name}_SOURCES})
    target_include_directories(${program_name} PUBLIC ${${program_name}_INCLUDE_DIRS})
endmacro()

basic_program(hapi "")
target_include_directories(hapi PUBLIC "/usr/include/spinnaker")
target_link_libraries(hapi wiringPi Spinnaker stdc++fs)

basic_program(hapi-config "tools/config/")
target_link_libraries(hapi-config stdc++fs)

install(TARGETS hapi hapi-config
        LIBRARY DESTINATION lib/
        RUNTIME DESTINATION bin/)
