SET(CMAKE_SYSTEM_NAME Linux)

SET(OWRT_STAGING_DIR /opt/devel/openwrt/openwrt.git/staging_dir)
SET(OWRT_TARGET target-mips_24kc_musl)
SET(OWRT_TOOLCHAIN toolchain-mips_24kc_gcc-7.4.0_musl)
SET(OWRT_CROSS ${OWRT_STAGING_DIR}/${OWRT_TOOLCHAIN}/bin/mips-openwrt-linux-)

SET(CMAKE_C_COMPILER ${OWRT_CROSS}gcc)
SET(CMAKE_CXX_COMPILER ${OWRT_CROSS}g++)

SET(CMAKE_FIND_ROOT_PATH ${OWRT_STAGING_DIR}/${OWRT_TARGET})

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

ADD_DEFINITIONS(
	-Os -pipe -mno-branch-likely -mips32r2 -mtune=24kc -fno-caller-saves -fno-plt
	-fhonour-copts -msoft-float -mips16 -minterlink-mips16
)
