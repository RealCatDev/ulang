cmake_minimum_required(VERSION 3.14)

project(ULang)

file(GLOB_RECURSE ULANG_CORE_SOURCES "./core/**.c" "./include/**.h")
add_library(ulang STATIC ${ULANG_CORE_SOURCES})
target_include_directories(ulang PUBLIC "./include/")

file(GLOB_RECURSE ULANG_SOURCES "./compiler/**.c" "./compiler/**.h")
add_executable(ulangc ${ULANG_SOURCES})
target_link_libraries(ulangc ulang)