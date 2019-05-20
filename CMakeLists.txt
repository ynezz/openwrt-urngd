cmake_minimum_required(VERSION 2.6)

PROJECT(urngd)
INCLUDE(GNUInstallDirs)
INCLUDE (FindPkgConfig)

SET(URNGD_VERSION 1.0.0)
SET(JTEN_DIR 3rdparty/jitterentropy-rngd)

FIND_PATH(ubox_include_dir NAMES libubox/usock.h)
FIND_LIBRARY(ubox NAMES ubox)
INCLUDE_DIRECTORIES(${ubox_include_dir} ${JTEN_DIR})

SET(CMAKE_C_FLAGS_DEBUG -DURNGD_DEBUG)

ADD_DEFINITIONS(-Wall -Werror -Wextra --std=gnu99  -DURNGD_VERSION="${URNGD_VERSION}")
ADD_DEFINITIONS(-Wno-unused-parameter)

ADD_EXECUTABLE(urngd
	urngd.c
	${JTEN_DIR}/jitterentropy-base.c
)
TARGET_LINK_LIBRARIES(urngd ${ubox})

# jitter RNG must not be compiled with optimizations
SET_SOURCE_FILES_PROPERTIES(${JTEN_DIR}/jitterentropy-base.c PROPERTIES COMPILE_FLAGS -O0)

INSTALL(TARGETS urngd RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})

SET(REMOTE_ADDR 192.168.1.20)
ADD_CUSTOM_TARGET(upload
	COMMAND scp ${CMAKE_BINARY_DIR}/urngd root@${REMOTE_ADDR}:/usr/sbin
)
