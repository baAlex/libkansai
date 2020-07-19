cmake_minimum_required(VERSION 3.10)

project("LibKansai" C)
option(KA_EXPORT_SYMBOLS "Export symbols" ON)
option(KA_BUILD_SHARED   "Build shared library" ON)
option(KA_BUILD_STATIC   "Build static library" ON)
option(KA_BUILD_SKETCHES "Build sketches" ON)

include_directories("./include/")

if ((CMAKE_C_COMPILER_ID MATCHES "Clang") OR (CMAKE_C_COMPILER_ID MATCHES "GNU"))
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -std=c11 -Wall -Wextra -Wconversion -pedantic -DKA_DEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -mtune=generic")
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /DKA_DEBUG /D_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /D_CRT_SECURE_NO_WARNINGS")
endif()

if (KA_EXPORT_SYMBOLS)
	if ((CMAKE_C_COMPILER_ID MATCHES "Clang") OR (CMAKE_C_COMPILER_ID MATCHES "GNU"))
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DKA_EXPORT_SYMBOLS")
	elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DKA_EXPORT_SYMBOLS")
	endif()
endif ()

set(SOURCES
	"./source/aabounding.c"
	"./source/context-agnostic.c"
	"./source/context-sdl2.c"
	"./source/random.c"
	"./source/utilities.c"
	"./source/version.c"
	"./source/glad/glad.c"
)

if (KA_BUILD_SHARED)
	add_library("kansai" SHARED ${SOURCES})
endif ()

if (KA_BUILD_STATIC)
	add_library("kansai-static" STATIC ${SOURCES})
endif ()

# LibJapan
option(JA_BUILD_TEST OFF)

if (KA_BUILD_SHARED)
	if(NOT TARGET "japan")
		# https://stackoverflow.com/a/42985142
		add_subdirectory("./source/libjapan/")
	endif ()

	target_link_libraries("kansai" "japan")
	target_include_directories("kansai" PRIVATE "./source/libjapan/include")
endif ()

if (KA_BUILD_STATIC)
	if(NOT TARGET "japan-static")
		# https://stackoverflow.com/a/42985142
		add_subdirectory("./source/libjapan/")
	endif ()

	target_link_libraries("kansai-static" "japan-static")
	target_include_directories("kansai-static" PRIVATE "./source/libjapan/include")
endif ()

# SDL2
find_package(SDL2)

if (DEFINED VCPKG_TOOLCHAIN)
	if (KA_BUILD_SHARED)
		target_link_libraries("kansai" SDL2::SDL2 SDL2::SDL2main)
	endif ()

	if (KA_BUILD_STATIC)
		target_link_libraries("kansai-static" SDL2::SDL2 SDL2::SDL2main)
	endif ()
else()
	if (KA_BUILD_SHARED)
		target_link_libraries("kansai" ${SDL2_LIBRARIES})
	endif ()

	if (KA_BUILD_STATIC)
		target_link_libraries("kansai-static" ${SDL2_LIBRARIES})
	endif ()
endif()

# Sketches
if (KA_BUILD_SKETCHES)
	include_directories("./source/libjapan/include")

	add_executable("square" "./sketches/square.c")
	target_link_libraries("square" "kansai-static")

	add_executable("multiple-windows" "./sketches/multiple-windows.c")
	target_link_libraries("multiple-windows" "kansai-static")

	add_executable("noise" "./sketches/noise.c")
	target_link_libraries("noise" "kansai-static")
endif()
