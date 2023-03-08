cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    reverse-ad-demo
    VERSION 0.1.0
    DESCRIPTION "Example implementation of reverse-mode automatic differentiation"
    HOMEPAGE_URL "https://git.sr.ht/~bogdanb/reverse-ad-demo"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(reverse-ad-demo_reverse-ad-demo INTERFACE)
add_library(reverse-ad-demo::reverse-ad-demo ALIAS reverse-ad-demo_reverse-ad-demo)

set_property(
    TARGET reverse-ad-demo_reverse-ad-demo PROPERTY
    EXPORT_NAME reverse-ad-demo
)

target_include_directories(
    reverse-ad-demo_reverse-ad-demo ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(reverse-ad-demo_reverse-ad-demo INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${reverse-ad-demo_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT reverse-ad-demo_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of reverse-ad-demo"
  )
endif()

include(cmake/dev-mode.cmake)
