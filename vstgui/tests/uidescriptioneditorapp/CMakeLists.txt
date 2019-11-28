##########################################################################################
# VSTGUI UIDescriptionEditor Application
##########################################################################################
option(VSTGUI_UIDESCRIPTIONEDITOR_APPLICATION "UIDescriptionEditorApp Target" OFF)

if(VSTGUI_UIDESCRIPTIONEDITOR_APPLICATION)

set(target UIDescriptionEditorApp)

set(${target}_sources
  "source/app.cpp"
)

vstgui_add_executable(${target} "${${target}_sources}")
vstgui_add_resources(${target} "${target_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/winresources.rc")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} FOLDER Tests)

endif(VSTGUI_UIDESCRIPTIONEDITOR_APPLICATION)
