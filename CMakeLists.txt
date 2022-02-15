cmake_minimum_required(VERSION 3.15)

project("signature hash calculator")

set(OPENSSL_VERSION "openssl/1.1.1m")
set(BOOST_VERSION "boost/1.78.0")


if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
                  ${OPENSSL_VERSION}
                  ${BOOST_VERSION}
                BASIC_SETUP
                BUILD_TYPE ${CMAKE_BUILD_TYPE}
                BUILD missing)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(APP_SRC "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_executable(sighash ${APP_SRC})

if (MSVC)
  add_definitions(-D_WIN32_WINNT=0x0601)
  set(COMPILER_FLAGS "/std:c++17 /Ox")
elseif (UNIX)
  set(COMPILER_FLAGS "-std=c++17 -O3")
else()
  message(FATAL_ERROR "Not suported platform")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

include_directories(
  ${CMAKE_SOURCE_DIR}
)

add_subdirectory(src)

target_link_libraries(sighash job_controller ${CONAN_LIBS})
