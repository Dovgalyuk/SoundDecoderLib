cmake_minimum_required(VERSION 3.15)
project(SoundDecoderLib)

add_compile_options(-Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -ftrapv)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fsanitize=address,undefined,leak)
    add_link_options(-fsanitize=address,undefined,leak)
endif()

add_compile_definitions(EMULATOR)

add_subdirectory(libvm)
if (NOT DEFINED ENV{CI})
    add_subdirectory(sample)
endif()

#cmake -DCMAKE_BUILD_TYPE=Release ..
#cmake -DCMAKE_BUILD_TYPE=Debug ..
