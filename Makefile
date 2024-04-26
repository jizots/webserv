# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/26 14:15:09 by tchoquet          #+#    #+#              #
#    Updated: 2024/04/26 14:40:53 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS_DIR  = $(shell find src -type d)
BUILD_DIR = build

SRC = $(shell find src -type f -name '*.cpp')
OBJ = $(foreach file, ${SRC:.cpp=.o}, ${BUILD_DIR}/$(notdir ${file}))

CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -DNDEBUG -Isrc

NAME = webserv

vpath %.cpp ${SRCS_DIR}

.PHONY: all clean fclean re

all: ${NAME}

${NAME}: ${OBJ}
	@${CXX} -o $@ $^
	@echo "Executable created at $@."

${BUILD_DIR}/%.o: %.cpp | ${BUILD_DIR}
	${CXX} ${CXXFLAGS} -o $@ -c $<

${BUILD_DIR}:
	mkdir -p $@

clean:
	@rm -rf ${BUILD_DIR}
	@echo "${BUILD_DIR} deleted."

fclean: clean
	@rm -rf ${NAME}
	@echo "${NAME} deleted."

re: fclean all
