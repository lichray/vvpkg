cmake_minimum_required(VERSION 3.1.3)
project(vvpkg)
enable_testing()

include(CTest)

option(USE_SYSTEM_SQLITE3 "Use system libsqlite3" OFF)

if(NOT MSVC)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
elseif(VCPKG_TARGET_TRIPLET MATCHES "x..-windows-static")
	foreach(flagvar CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		string(REGEX REPLACE "/MD" "/MT" ${flagvar} "${${flagvar}}")
		string(REGEX REPLACE "/MDd" "/MTd" ${flagvar} "${${flagvar}}")
	endforeach()
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY tests)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(MINGW)
	add_definitions(-DMINGW_HAS_SECURE_API)
endif()

if(NOT MSVC)
	add_compile_options(-Wall)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wconversion>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wsign-conversion>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wsign-compare>)
	set(CMAKE_CXX_EXTENSIONS OFF)
else()
	add_definitions(/std:c++latest)
endif()

include_directories(3rdparty/include)
include_directories(include)

file(GLOB vvpkg_srcs src/*.cc)
file(GLOB tests_srcs tests/*.cc)
file(GLOB sqxx_srcs 3rdparty/src/sqxx/*.cpp)

add_library(vvpkg ${vvpkg_srcs})

if(USE_SYSTEM_SQLITE3)
	add_library(sqxx ${sqxx_srcs})
	find_package(SQLITE3 REQUIRED)
	target_link_libraries(sqxx sqlite3)
else()
	add_library(sqxx ${sqxx_srcs} 3rdparty/src/sqlite3.c)
	target_include_directories(sqxx PRIVATE 3rdparty/include/sqlite3)
endif()

if(MSVC)
	target_compile_options(sqxx PRIVATE /wd4800)
endif()

target_include_directories(sqxx PRIVATE 3rdparty/include/sqxx)
target_link_libraries(vvpkg sqxx Threads::Threads)

if(BUILD_TESTING)
	add_executable(run ${tests_srcs})
	target_link_libraries(run vvpkg)
	add_test(testall tests/run)
endif()

file(GLOB demo_srcs demo/*.cc)

foreach(demo_src ${demo_srcs})
	get_filename_component(demo ${demo_src} NAME_WE)
	add_executable(${demo} ${demo_src})
	target_link_libraries(${demo} vvpkg)
	set_target_properties(${demo} PROPERTIES RUNTIME_OUTPUT_DIRECTORY demo)
endforeach()
