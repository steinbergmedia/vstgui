set(VSTGUI_TOOLS FOLDER "VSTGUI Tools")

if(VSTGUI_STANDALONE)
    add_subdirectory(imagestitcher)
endif()
add_subdirectory(uidesccompressor)

