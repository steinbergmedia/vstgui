cmake_minimum_required(VERSION 3.25.0)

option(VSTGUI_PLUGIN_BINDINGS_TARGET "Create a plugin-bindings target (only useful for development purpose)" OFF)
if(VSTGUI_PLUGIN_BINDINGS_TARGET)

    add_library(vstgui_plugin_bindings INTERFACE
        aeffguieditor.cpp
        aeffguieditor.h
        getpluginbundle.h
        plugguieditor.cpp
        plugguieditor.h
        vst3editor.cpp
        vst3editor.h
        vst3groupcontroller.cpp
        vst3groupcontroller.h
        vst3padcontroller.cpp
        vst3padcontroller.h
    )

    vstgui_source_group_by_folder(vstgui_plugin_bindings)

endif(VSTGUI_PLUGIN_BINDINGS_TARGET)
