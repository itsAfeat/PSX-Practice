# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

project(
	PSn00bSDK-3dprac
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK 3D Practice"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

psn00bsdk_add_executable(3dprac STATIC main.c)
psn00bsdk_add_cd_image(iso 3dprac iso.xml DEPENDS 3dprac)
