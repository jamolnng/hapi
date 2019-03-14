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

##### main program #####

file(GLOB_RECURSE HAPI_SOURCES "src/*.cpp")
file(GLOB_RECURSE HAPI_HEADERS "include/*.h")

get_include_dirs("${HAPI_HEADERS}" HAPI_INCLUDE_DIRS)

add_executable(hapi ${HAPI_SOURCES})
target_include_directories(hapi PUBLIC ${HAPI_INCLUDE_DIRS} /usr/include/spinnaker)
target_link_libraries(hapi wiringPi Spinnaker stdc++fs)

##### end main program #####

##### hapi-config #####

file(GLOB_RECURSE HAPI_CONFIG_SOURCES "tools/config/src/*.cpp")
file(GLOB_RECURSE HAPI_CONFIG_HEADERS "tools/config/include/*.h")

get_include_dirs("${HAPI_CONFIG_HEADERS}" HAPI_CONFIG_INCLUDE_DIRS)

add_executable(hapi-config ${HAPI_CONFIG_SOURCES})
target_include_directories(hapi-config PUBLIC ${HAPI_CONFIG_INCLUDE_DIRS})
target_link_libraries(hapi-config stdc++fs)

file(GLOB_RECURSE HAPI_PMT_CALIBRATE_SOURCES "tools/pmt_calibrate/src/*.cpp")
file(GLOB_RECURSE HAPI_PMT_CALIBRATE_HEADERS "tools/pmt_calibrate/include/*.h")
get_include_dirs("${HAPI_PMT_CALIBRATE_HEADERS}" HAPI_PMT_CALIBRATE_INCLUDE_DIRS)
add_executable(hapi-pmt-calibrate ${HAPI_PMT_CALIBRATE_SOURCES})
target_include_directories(hapi-pmt-calibrate PUBLIC ${HAPI_CONFIG_INCLUDE_DIRS})
target_sources(hapi-pmt-calibrate PUBLIC "src/board.cpp" "src/config.cpp" "src/logger.cpp"
                                         "src/routines/get_config.cpp" "src/routines/os_utils.cpp"
                                         "src/routines/pmt_calibrate.cpp" "src/routines/str_utils.cpp")
target_include_directories(hapi-pmt-calibrate PUBLIC "include/" "include/routines/")
target_link_libraries(hapi-pmt-calibrate wiringPi stdc++fs)

install(TARGETS hapi hapi-config hapi-pmt-calibrate
        LIBRARY DESTINATION lib/
        RUNTIME DESTINATION bin/)

##### end hapi-config #####
