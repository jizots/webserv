#/* ************************************************************************** */
#/*                                                                            */
#/*                                                        :::      ::::::::   */
#/*   CMakeLists.txt                                       :+:      :+:    :+:   */
#/*                                                    +:+ +:+         +:+     */
#/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
#/*                                                +#+#+#+#+#+   +#+           */
#/*   Created: 2024/02/10 13:34:45 by tchoquet          #+#    #+#             */
#/*                                                    ###   ########.fr       */
#/*                                                                            */
#/* ************************************************************************** */

cmake_minimum_required(VERSION 3.23)

project(webserv LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(webserv)

file(GLOB_RECURSE sources "src/*.cpp" "src/*.tpp" "src/*.hpp")
list(REMOVE_ITEM sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTP/test.cpp
    )

target_sources(webserv PRIVATE "${sources}")
target_include_directories(webserv PRIVATE src)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(webserv PRIVATE -Wall -Wextra -Werror)
endif()
