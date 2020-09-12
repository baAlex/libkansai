
cmake_minimum_required(VERSION 3.10)
project("LibKansai" C)

option(KANSAI_EXPORT_SYMBOLS "Export symbols" ON)
option(KANSAI_SHARED "Build shared library"   ON)
option(KANSAI_STATIC "Build static library"   ON)
option(KANSAI_BUILD_SKETCHES "Build sketches" ON)

if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

if (KANSAI_EXPORT_SYMBOLS)
	add_compile_definitions(KA_EXPORT_SYMBOLS)
endif ()

set(KANSAI_SOURCES
	"./source/aabounding.c"
	"./source/context-agnostic.c"
	"./source/context-sdl2.c"
	"./source/random.c"
	"./source/utilities.c"
	"./source/version.c"
	"./source/glad/glad.c"
)

find_package(SDL2)

if (KANSAI_SHARED)

	if (NOT TARGET "japan") # https://stackoverflow.com/a/42985142
		option(JAPAN_BUILD_TEST OFF)
		add_subdirectory("./source/libjapan/")
	endif ()

	add_library("kansai" SHARED ${KANSAI_SOURCES})
	target_include_directories("kansai" PUBLIC "./include/")
	target_link_libraries("kansai" PUBLIC "japan")

	if (NOT MSVC)
		target_link_libraries("kansai" PRIVATE "m")
	endif ()

	if (DEFINED VCPKG_TOOLCHAIN)
		target_link_libraries("kansai" PRIVATE SDL2::SDL2 SDL2::SDL2main)
	else ()
		target_link_libraries("kansai" PRIVATE ${SDL2_LIBRARIES})
	endif ()
endif ()

if (KANSAI_STATIC)

	if (NOT TARGET "japan-static") # https://stackoverflow.com/a/42985142
		option(JAPAN_BUILD_TEST OFF)
		add_subdirectory("./source/libjapan/")
	endif ()

	add_library("kansai-static" STATIC ${KANSAI_SOURCES})
	target_include_directories("kansai-static" PUBLIC "./include/")
	target_link_libraries("kansai-static" PUBLIC "japan-static")

	if (NOT MSVC)
		target_link_libraries("kansai-static" PRIVATE "m")
	endif ()

	if (DEFINED VCPKG_TOOLCHAIN)
		target_link_libraries("kansai-static" PRIVATE SDL2::SDL2 SDL2::SDL2main)
	else ()
		target_link_libraries("kansai-static" PRIVATE ${SDL2_LIBRARIES})
	endif ()
endif ()

if (KANSAI_BUILD_SKETCHES)

	add_executable("noise" "./sketches/noise.c")
	target_link_libraries("noise" PRIVATE "kansai-static")

	add_executable("square" "./sketches/square.c")
	target_link_libraries("square" PRIVATE "kansai-static")

	add_executable("multiple-windows" "./sketches/multiple-windows.c")
	target_link_libraries("multiple-windows" PRIVATE "kansai-static")

	add_executable("noise" "./sketches/viewer.c")
	target_link_libraries("viewer" PRIVATE "kansai-static")
endif()
