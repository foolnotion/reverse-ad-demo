cmake_minimum_required(VERSION 3.14)

project(reverse-ad-demoTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(reverse-ad-demo REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(reverse-ad-demo_test source/reverse-ad-demo_test.cpp)
target_link_libraries(reverse-ad-demo_test PRIVATE reverse-ad-demo::reverse-ad-demo)
target_compile_features(reverse-ad-demo_test PRIVATE cxx_std_20)

add_test(NAME reverse-ad-demo_test COMMAND reverse-ad-demo_test)

# ---- End-of-file commands ----

add_folders(Test)
