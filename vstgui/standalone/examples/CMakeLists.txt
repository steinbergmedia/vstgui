set(VSTGUI_STANDALONE_EXAMPLES_FOLDER FOLDER "VSTGUI Standalone Examples")

if(NOT MSVC_TOOLSET_VERSION OR MSVC_TOOLSET_VERSION VERSION_GREATER 140)
  add_subdirectory(mandelbrot)
  add_subdirectory(minesweeper)
endif()
add_subdirectory(simple_standalone)
add_subdirectory(standalone)
