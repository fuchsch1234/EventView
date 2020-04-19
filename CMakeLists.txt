cmake_minimum_required(VERSION 3.13)

project(app CXX)

add_library(compileWarnings INTERFACE)
target_compile_options(compileWarnings INTERFACE -Wall -Werror -Wextra -pedantic)

option(QtApp "This project builds a QT QML app" OFF)
set(QtResources "")

if (QtApp)
	find_package(Qt5 COMPONENTS Quick Core Network REQUIRED)
	find_package(Qt5QuickCompiler)

	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	qt5_add_big_resources(QtResources Res/EventView.qrc OPTIONS -compress 2)
endif()

add_executable(app Src/Main.cpp ${QtResources})
target_link_libraries(app compileWarnings)
target_compile_features(app PUBLIC cxx_std_17)

if (QtApp)
	qt5_use_modules(app Quick Core Network)
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