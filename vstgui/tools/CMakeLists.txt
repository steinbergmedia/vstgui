set(VSTGUI_TOOLS_FOLDER FOLDER "VSTGUI Tools")

if(VSTGUI_STANDALONE AND NOT (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4))
    add_subdirectory(imagestitcher)
endif()
add_subdirectory(uidesccompressor)
