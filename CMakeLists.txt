cmake_minimum_required(VERSION 3.30)
project(sl-service VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# 查询基线版本 https://github.com/microsoft/vcpkg/blob/48a0cef9ea67606c94f149647470c956bb78050a/versions/baseline.json
find_package(Boost REQUIRED COMPONENTS
        thread system date_time regex program_options
        mqtt5)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

message(STATUS "Boost version: ${Boost_VERSION} , Boost library dirs: ${Boost_INCLUDE_DIRS}")

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
add_executable(${PROJECT_NAME} main.cpp ${SRC_LIST})

# 链接库
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${Boost_INCLUDE_DIRS}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else()
    add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif()

if (MSVC)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_USE_WINAPI_VERSION=0x0602)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /bigobj)
endif()

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Boost::thread
        Boost::system
        Boost::date_time
        Boost::regex
        Boost::mqtt5
        Boost::program_options
        nlohmann_json::nlohmann_json
        cpr::cpr
        yaml-cpp::yaml-cpp
        spdlog::spdlog
)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        SL_SERVICE_VERSION="${PROJECT_VERSION}"
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "复制 config.yaml 到可执行文件目录..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config.yaml
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
