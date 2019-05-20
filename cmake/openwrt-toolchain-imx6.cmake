SET(CMAKE_SYSTEM_NAME Linux)

SET(OWRT_STAGING_DIR /opt/devel/openwrt/openwrt.git/staging_dir)
SET(OWRT_TARGET target-arm_cortex-a9+neon_musl_eabi)
SET(OWRT_TOOLCHAIN toolchain-arm_cortex-a9+neon_gcc-7.4.0_musl_eabi)
SET(OWRT_CROSS ${OWRT_STAGING_DIR}/${OWRT_TOOLCHAIN}/bin/arm-openwrt-linux-)

SET(CMAKE_C_COMPILER ${OWRT_CROSS}gcc)
SET(CMAKE_CXX_COMPILER ${OWRT_CROSS}g++)

SET(CMAKE_FIND_ROOT_PATH ${OWRT_STAGING_DIR}/${OWRT_TARGET})

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

ADD_DEFINITIONS(-mcpu=cortex-a9 -mfpu=neon -g3 -fno-caller-saves -fhonour-copts -mfloat-abi=hard)
