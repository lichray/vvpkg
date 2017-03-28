cmake_minimum_required(VERSION 3.1.3)
project(vvpkg CXX)
enable_testing()

include(CTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY tests)
set(CMAKE_CXX_STANDARD 14)

find_package(OpenSSL REQUIRED)

if(MINGW)
	add_definitions(-DMINGW_HAS_SECURE_API)
endif()

if(NOT MSVC)
	add_definitions(-Wall -Wconversion -Wsign-conversion -Wsign-compare)
	set(CMAKE_CXX_EXTENSIONS OFF)
else()
	add_definitions(/std:c++latest)
endif()

include_directories(3rdparty/include)
include_directories(include)

file(GLOB vvpkg_srcs src/*.cc src/*.cpp)
file(GLOB tests_srcs tests/*.cc)

add_library(vvpkg ${vvpkg_srcs})

target_include_directories(vvpkg PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(vvpkg ${OPENSSL_CRYPTO_LIBRARIES})

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
