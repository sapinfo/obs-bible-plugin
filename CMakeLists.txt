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
  # Workaround: Qt 6.x FindWrapOpenGL on macOS 15+ SDK links -framework AGL,
  # but AGL was removed from the SDK. Create a tiny stub AGL.framework so
  # find_library finds it and Qt won't fall back to literal "-framework AGL".
  if(APPLE)
    set(_agl_stub_root "${CMAKE_BINARY_DIR}/stub_frameworks")
    set(_agl_stub_fw   "${_agl_stub_root}/AGL.framework")
    if(NOT EXISTS "${_agl_stub_fw}/AGL")
      file(MAKE_DIRECTORY "${_agl_stub_fw}")
      file(WRITE "${CMAKE_BINARY_DIR}/agl_stub.c" "void __bible_agl_stub(void) {}\n")
      execute_process(
        COMMAND clang -dynamiclib -arch arm64 -arch x86_64
                -install_name "@rpath/AGL.framework/AGL"
                -o "${_agl_stub_fw}/AGL"
                "${CMAKE_BINARY_DIR}/agl_stub.c"
        RESULT_VARIABLE _agl_stub_rc
      )
      if(NOT _agl_stub_rc EQUAL 0)
        message(WARNING "Failed to build AGL stub; linker may fail")
      endif()
    endif()
    list(PREPEND CMAKE_FRAMEWORK_PATH "${_agl_stub_root}")
  endif()

  find_package(Qt6 REQUIRED COMPONENTS Widgets Core)
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

find_package(SQLite3 REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SQLite3::SQLite3)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BibleSource.cpp
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
