cmake_minimum_required(VERSION 3.17)

project(app CXX)

add_library(compileWarnings INTERFACE)
target_compile_options(compileWarnings INTERFACE -Wall -Werror -Wextra -pedantic)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_add_remote(NAME bincrafters INDEX 1 URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_cmake_run(
	REQUIRES spdlog/[>=1.4.1]
		gtest/[>=1.10.0]						
	BASIC_SETUP CMAKE_TARGETS
	BUILD missing)

option(QtApp "This project builds a QT QML app" OFF)
set(QtResources "")

if (QtApp)
	find_package(Qt5 COMPONENTS Quick Core SerialPort REQUIRED)
	find_package(Qt5QuickCompiler)

	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	qt5_add_big_resources(QtResources Res/EventView.qrc OPTIONS -compress 2)
endif()

add_executable(app Src/Main.cpp ${QtResources})
target_include_directories(app PRIVATE Include)
target_link_libraries(app compileWarnings CONAN_PKG::spdlog)
target_compile_features(app PRIVATE cxx_std_20)
target_precompile_headers(app PRIVATE Include/PCH.h)

if (QtApp)
	target_link_libraries(app Qt5::Quick Qt5::Core Qt5::SerialPort)
endif()

option(EnableClangTidy "Enable clang-tidy" ON)

if (EnableClangTidy)
  find_program(CLANGTIDY clang-tidy)
  if (CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
  else()
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

option(BuildTest "Build tests for this project" ON)

if (BuildTest)
	enable_testing()
	add_subdirectory(Test)
endif()