find_package(GTest "1.10.0" REQUIRED)

# Basic options for every unit test
add_library(UnitTestOptions INTERFACE)

# Add requested sanitizers to build
include(CMakeDependentOption)
set(Sanitizers "")

cmake_dependent_option(UseAddressSanitizer "Build tests with address sanitizer" ON "BuildTest" OFF)
if (UseAddressSanitizer)
	list(APPEND Sanitizers "address")	
endif()

cmake_dependent_option(UseMemorySanitizer "Build tests with memory sanitizer" OFF "BuildTest" OFF)
if (UseMemorySanitizer)
	list(APPEND Sanitizers "memory")	
endif()

cmake_dependent_option(UseUBSanitizer "Build tests with undefined behavior sanitizer" ON "BuildTest" OFF)
if (UseUBSanitizer)
	list(APPEND Sanitizers "undefined")	
endif()

cmake_dependent_option(UseThreadSanitizer "Build tests with thread sanitizer" OFF "BuildTest" OFF)
if (UseThreadSanitizer)
	list(APPEND Sanitizers "thread")	
endif()

list(JOIN Sanitizers "," ListOfSanitizers)
if (NOT "${ListOfSanitizers}" STREQUAL "")
	target_compile_features(UnitTestOptions INTERFACE cxx_std_20)
	target_compile_options(UnitTestOptions INTERFACE -fsanitize=${ListOfSanitizers})
	target_link_options(UnitTestOptions INTERFACE -fsanitize=${ListOfSanitizers})
endif()

# Set options for coverage using gcov
target_compile_options(UnitTestOptions INTERFACE --coverage -O0 -g)
target_link_options(UnitTestOptions INTERFACE --coverage)
# Inherit default compiler options and link against googletest
target_link_libraries(UnitTestOptions INTERFACE GTest::GTest gmock compileWarnings)
target_include_directories(UnitTestOptions INTERFACE ${CMAKE_SOURCE_DIR}/Include)

# Unit test target
add_executable(UnitTests EventParserTest.cpp GTestMain.cpp)
target_link_libraries(UnitTests UnitTestOptions)
gtest_discover_tests(UnitTests
					TEST_SUFFIX .hostTests
					TEST_LIST UnitHostTests)
set_tests_properties(${UnitHostTests} PROPERTIES TIMEOUT 5)

# Example how to build a compile failure test
# Can be used to test e.g. static_asserts in templated code, to ensure necessary conditions are correctly enforced.
# The target is excluded from the default build, to avoid breaking the normal build.
# A custom test is added that tries to use cmake to build the target and checks that it fails.
add_executable(CompileFailureTest CompileChecks.cpp)
target_include_directories(CompileFailureTest PRIVATE mocks ${CMAKE_SOURCE_DIR}/Include)
target_compile_features(CompileFailureTest PRIVATE cxx_std_17)
set_target_properties(CompileFailureTest PROPERTIES
				EXCLUDE_FROM_ALL TRUE
				EXCLUDE_FROM_DEFAULT_BUILD TRUE)

#add_test(NAME CompileChecks
#		COMMAND ${CMAKE_COMMAND} --build . --target CompileFailureTest --config $<CONFIGURATION> WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
#set_tests_properties(CompileChecks PROPERTIES WILL_FAIL TRUE)

# If gcovr is available add a target 'gcovr' that builds and runs available tests and creates a coverage report
find_program(GCOVR gcovr)
if (GCOVR)
	# Builds and runs all tests
	add_custom_command(OUTPUT RunTests
	  COMMAND ${CMAKE_CTEST_COMMAND}
	  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	  DEPENDS UnitTests
	  VERBATIM
	)
	# Invokes gcovr with links to sources and generated coverage files
	add_custom_command(OUTPUT GenerateGcovReport
	  COMMAND gcovr -r "${CMAKE_SOURCE_DIR}" --object-directory "${CMAKE_BINARY_DIR}" --exclude "${CMAKE_SOURCE_DIR}/Test"
	  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	  DEPENDS RunTests
	  COMMENT "Generating a test coverage report"
	  VERBATIM
	)
	add_custom_target(gcovr DEPENDS GenerateGcovReport)
endif()