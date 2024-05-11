#  ************************************************************************** */
#                                                                             */
#                                                         :::      ::::::::   */
#    CMakeLists.txt                                     :+:      :+:    :+:   */
#                                                     +:+ +:+         +:+     */
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
#                                                 +#+#+#+#+#+   +#+           */
#    Created: 2024/02/10 13:34:45 by tchoquet          #+#    #+#             */
#                                                     ###   ########.fr       */
#                                                                             */
#  ************************************************************************** */

cmake_minimum_required(VERSION 3.23)

project(webserv LANGUAGES CXX C)

include(FetchContent)

FetchContent_Declare (
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

option(WEBSERV_BUILD_TESTS "Build webserv test executable" OFF)

if (WEBSERV_BUILD_TESTS)
    enable_testing()
endif()

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(webserv-lib)
add_executable(webserv)
if (WEBSERV_BUILD_TESTS)
    add_executable(webserv-test)
endif()

file(GLOB_RECURSE sources "src/*.cpp" "src/*.tpp" "src/*.hpp")
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_sources(webserv-lib PRIVATE "${sources}")

target_sources(webserv PRIVATE "src/main.cpp")

if (WEBSERV_BUILD_TESTS)
    file(GLOB_RECURSE sources "tests/*.cpp" "tests/*.tpp" "tests/*.hpp")
    target_sources(webserv-test PRIVATE "${sources}")
endif()

target_include_directories(webserv-lib PUBLIC src)
if (WEBSERV_BUILD_TESTS)
    target_include_directories(webserv-test PRIVATE tests)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(webserv-lib PRIVATE -Wall -Wextra -Werror)
    target_compile_options(webserv PRIVATE -Wall -Wextra -Werror)
endif()    

target_link_libraries(webserv webserv-lib)
if (WEBSERV_BUILD_TESTS)
    FetchContent_MakeAvailable(googletest)
    target_link_libraries(webserv-test webserv-lib GTest::gtest_main)
endif()

if (WEBSERV_BUILD_TESTS)
    include(GoogleTest)
    gtest_discover_tests(webserv-test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
