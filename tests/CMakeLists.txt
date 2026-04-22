find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(SQLite3 REQUIRED)
find_package(libobs REQUIRED)

# Helper: apply runtime framework path so the test binary can find Qt and libobs transitive deps.
# libobs.framework is in .deps/Frameworks. Qt6 and ffmpeg dylibs are in obs-deps*/lib.
file(GLOB _qt6_prebuilt_dir    "${CMAKE_SOURCE_DIR}/.deps/obs-deps-qt6-*-universal/lib")
file(GLOB _obs_prebuilt_dir    "${CMAKE_SOURCE_DIR}/.deps/obs-deps-*-universal/lib")
list(FILTER _obs_prebuilt_dir EXCLUDE REGEX "qt6")
set(BIBLE_OBS_FW    "${CMAKE_SOURCE_DIR}/.deps/Frameworks")
set(BIBLE_QT_DIR    "${_qt6_prebuilt_dir}")
set(BIBLE_DEPS_LIB  "${_obs_prebuilt_dir}")
set(BIBLE_XCODE_RPATH "${BIBLE_OBS_FW} ${BIBLE_QT_DIR} ${BIBLE_DEPS_LIB}")

function(bible_add_test target)
  add_executable(${target} ${ARGN})
  target_include_directories(${target} PRIVATE ../src)
  set_target_properties(${target} PROPERTIES
    AUTOMOC OFF
    BUILD_RPATH "${BIBLE_OBS_FW};${BIBLE_QT_DIR};${BIBLE_DEPS_LIB}"
    INSTALL_RPATH "${BIBLE_OBS_FW};${BIBLE_QT_DIR};${BIBLE_DEPS_LIB}"
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${BIBLE_XCODE_RPATH}"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  )
endfunction()

bible_add_test(test_make_search_pattern
  test_make_search_pattern.cpp
  ../src/BibleSearch.cpp
)
target_link_libraries(test_make_search_pattern PRIVATE Qt6::Core)

bible_add_test(test_bible_source
  test_bible_source.cpp
  ../src/BibleSource.cpp
  ../src/BibleSearch.cpp
)
target_link_libraries(test_bible_source PRIVATE Qt6::Core SQLite3::SQLite3 OBS::libobs)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fixture_bible.sql
               ${CMAKE_CURRENT_BINARY_DIR}/fixture_bible.sql COPYONLY)

add_custom_target(run_tests
  COMMAND test_make_search_pattern
  COMMAND test_bible_source ${CMAKE_CURRENT_BINARY_DIR}/fixture_bible.sql
  DEPENDS test_make_search_pattern test_bible_source
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
