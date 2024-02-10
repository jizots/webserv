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

target_sources(webserv PRIVATE "${sources}")
target_include_directories(webserv PRIVATE src)