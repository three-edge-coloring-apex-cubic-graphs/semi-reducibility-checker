cmake_minimum_required(VERSION 3.16)
project(coloring CXX)

include(FetchContent)
FetchContent_Declare(
    cppcoro
    GIT_REPOSITORY https://github.com/andreasbuhr/cppcoro.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(cppcoro)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

add_executable(a.out main.cpp)
target_compile_options(a.out PUBLIC -O3 -Wall)
target_compile_features(a.out PUBLIC cxx_std_20)
target_link_libraries(a.out PRIVATE 
    Boost::boost Boost::program_options
    spdlog::spdlog
    fmt::fmt
    cppcoro)
