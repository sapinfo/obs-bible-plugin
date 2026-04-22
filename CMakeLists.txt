cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api" ON)
option(ENABLE_QT "Use Qt functionality" ON)

set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

  # Workaround: Qt 6.x FindWrapOpenGL on macOS 15+ SDK adds "-framework AGL" to
  # WrapOpenGL::WrapOpenGL, but AGL was removed from the SDK. Nothing in Qt6
  # Widgets/Gui actually calls AGL symbols at runtime — the link is just a
  # legacy transitive reference. Strip AGL from the interface to avoid a
  # missing library error at both link and runtime.
  if(APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    get_target_property(_wgl_libs WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    if(_wgl_libs)
      list(REMOVE_ITEM _wgl_libs "-framework AGL")
      list(FILTER _wgl_libs EXCLUDE REGEX "AGL\\.framework")
      set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES
        INTERFACE_LINK_LIBRARIES "${_wgl_libs}")
    endif()
  endif()
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

# Vendor SQLite3 amalgamation (Public Domain) to avoid per-platform
# packaging differences (macOS SDK vs Ubuntu libsqlite3-dev vs missing on
# Windows runners). Compiled as part of the plugin target.
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE vendor/sqlite3)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE vendor/sqlite3/sqlite3.c)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BiblePluginManager.cpp
  src/BiblePluginSettings.cpp
  src/BibleSource.cpp
  src/BibleVerseSource.cpp
  src/ui/BibleDock.cpp
  src/ui/BibleWidget.cpp
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

# Tests are a macOS-only local developer smoke check; skip in CI and on
# non-Apple platforms. Enable manually with -DBUILD_TESTS=ON locally.
option(BUILD_TESTS "Build unit tests (macOS only)" OFF)
if(BUILD_TESTS AND APPLE)
  add_subdirectory(tests)
endif()
