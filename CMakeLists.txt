cmake_minimum_required(VERSION 3.20)
project(myl LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-missing-field-initializers -Wno-shadow -Wno-sign-conversion -Wno-shorten-64-to-32")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# --- LLVM -------------------------------------------------------------------
find_package(LLVM 15 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")

# On Ubuntu with llvm-15-dev the components are consolidated in libLLVM-15.so.
find_library(LLVM_SHARED_LIBRARY LLVM-15)
if(NOT LLVM_SHARED_LIBRARY)
    message(FATAL_ERROR "Could not locate libLLVM-15 shared library")
endif()
message(STATUS "LLVM_SHARED_LIBRARY: ${LLVM_SHARED_LIBRARY}")

find_library(FFI_LIBRARY ffi)
find_library(DL_LIBRARY dl)
find_library(RT_LIBRARY rt)
find_library(PTHREAD_LIBRARY pthread)
find_library(M_LIBRARY m)

set(LLVM_LIBS
    ${LLVM_SHARED_LIBRARY}
    ${FFI_LIBRARY}
    ${DL_LIBRARY}
    ${RT_LIBRARY}
    ${PTHREAD_LIBRARY}
    ${M_LIBRARY}
)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# --- Flex / Bison -----------------------------------------------------------
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(MYL_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${MYL_GENERATED_DIR})

BISON_TARGET(MylParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
    ${MYL_GENERATED_DIR}/parser.cpp
    DEFINES_FILE ${MYL_GENERATED_DIR}/parser.hpp)

FLEX_TARGET(MylLexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
    ${MYL_GENERATED_DIR}/lexer.cpp)

add_flex_bison_dependency(MylLexer MylParser)

# --- Compiler library -------------------------------------------------------
add_library(myl_compiler
    src/ast.cpp
    src/semantic.cpp
    src/codegen.cpp
    src/driver.cpp
    src/frontend.cpp
    ${BISON_MylParser_OUTPUTS}
    ${FLEX_MylLexer_OUTPUTS}
)

target_include_directories(myl_compiler
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${MYL_GENERATED_DIR}
)

target_link_libraries(myl_compiler ${LLVM_LIBS})

# --- CLI --------------------------------------------------------------------
add_executable(myl src/main.cpp)
target_link_libraries(myl myl_compiler)

enable_testing()
add_subdirectory(tests)
