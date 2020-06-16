cmake_minimum_required(VERSION 3.10)

project("LibKansai" C)
option(KA_EXPORT_SYMBOLS "Export symbols" ON)
option(KA_BUILD_SHARED   "Build shared library" ON)
option(KA_BUILD_STATIC   "Build static library" ON)

include_directories("./include/")

if ((CMAKE_C_COMPILER_ID MATCHES "Clang") OR (CMAKE_C_COMPILER_ID MATCHES "GNU"))
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -std=c99 -Wall -Wextra -Wconversion -pedantic -DKA_DEBUG")
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
	"./source/version.c"
)

if (KA_BUILD_SHARED)
	add_library("kansai" SHARED ${SOURCES})
endif ()

if (KA_BUILD_STATIC)
	add_library("kansai-static" STATIC ${SOURCES})
endif ()

# LibJapan
option(JA_EXPORT_SYMBOLS OFF)
option(JA_BUILD_SHARED OFF)
option(JA_BUILD_TEST OFF)

add_subdirectory("./source/libjapan")

if (KA_BUILD_SHARED)
	target_include_directories("kansai" PRIVATE "./source/libjapan/include")
	target_link_libraries("kansai" "japan-static")
endif ()

if (KA_BUILD_STATIC)
	target_include_directories("kansai-static" PRIVATE "./source/libjapan/include")
	target_link_libraries("kansai-static" "japan-static")
endif ()

