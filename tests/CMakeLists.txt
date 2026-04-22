find_package(Qt6 REQUIRED COMPONENTS Core)

# Helper: apply runtime framework path so the test binary can find Qt at runtime.
# Qt6 frameworks live in the obs-deps-qt6 prebuilt directory (libobs is in Frameworks/).
file(GLOB _qt6_prebuilt_dir "${CMAKE_SOURCE_DIR}/.deps/obs-deps-qt6-*-universal/lib")
set(BIBLE_OBS_FW   "${CMAKE_SOURCE_DIR}/.deps/Frameworks")
set(BIBLE_QT_DIR   "${_qt6_prebuilt_dir}")
# Xcode wants space-separated paths in the LD_RUNPATH_SEARCH_PATHS setting.
set(BIBLE_XCODE_RPATH "${BIBLE_OBS_FW} ${BIBLE_QT_DIR}")

function(bible_add_test target)
  add_executable(${target} ${ARGN})
  target_include_directories(${target} PRIVATE ../src)
  set_target_properties(${target} PROPERTIES
    AUTOMOC OFF
    BUILD_RPATH "${BIBLE_OBS_FW};${BIBLE_QT_DIR}"
    INSTALL_RPATH "${BIBLE_OBS_FW};${BIBLE_QT_DIR}"
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

add_custom_target(run_tests
  COMMAND test_make_search_pattern
  DEPENDS test_make_search_pattern
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
