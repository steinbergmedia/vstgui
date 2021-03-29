##########################################################################################
# VSTGUI UIDescription Library
##########################################################################################
set(target vstgui_uidescription)

set(${target}_sources
    base64codec.h
    compresseduidescription.cpp
    compresseduidescription.h
    cstream.cpp
    cstream.h
    delegationcontroller.h
    doc.h
    icontentprovider.h
    icontroller.h
    iuidescription.h
    iviewcreator.h
    iviewfactory.h
    uiattributes.cpp
    uiattributes.h
    uicontentprovider.cpp
    uicontentprovider.h
    uidescription.cpp
    uidescription.h
    uidescriptionlistener.h
    uidescriptionfwd.h
    uiviewcreator.cpp
    uiviewcreator.h
    uiviewfactory.cpp
    uiviewfactory.h
    uiviewswitchcontainer.cpp
    uiviewswitchcontainer.h
    xmlparser.cpp
    xmlparser.h
    detail/locale.h
    detail/parsecolor.h
    detail/scalefactorutils.h
    detail/uidesclist.cpp
    detail/uidesclist.h
    detail/uijsonpersistence.cpp
    detail/uijsonpersistence.h
    detail/uinode.cpp
    detail/uinode.h
    detail/uiviewcreatorattributes.h
    detail/uixmlpersistence.cpp
    detail/uixmlpersistence.h
    editing/doc.h
    editing/iaction.h
    editing/igridprocessor.h
    editing/uiactions.cpp
    editing/uiactions.h
    editing/uiattributescontroller.cpp
    editing/uiattributescontroller.h
    editing/uibasedatasource.h
    editing/uibitmapscontroller.cpp
    editing/uibitmapscontroller.h
    editing/uicolor.cpp
    editing/uicolor.h
    editing/uicolorchoosercontroller.cpp
    editing/uicolorchoosercontroller.h
    editing/uicolorscontroller.cpp
    editing/uicolorscontroller.h
    editing/uicolorslider.cpp
    editing/uicolorslider.h
    editing/uicrosslines.cpp
    editing/uicrosslines.h
    editing/uidescriptioneditor.uidesc
    editing/uidialogcontroller.cpp
    editing/uidialogcontroller.h
    editing/uieditcontroller.cpp
    editing/uieditcontroller.h
    editing/uieditmenucontroller.cpp
    editing/uieditmenucontroller.h
    editing/uieditview.cpp
    editing/uieditview.h
    editing/uifocussettingscontroller.cpp
    editing/uifocussettingscontroller.h
    editing/uifontscontroller.cpp
    editing/uifontscontroller.h
    editing/uigradientscontroller.cpp
    editing/uigradientscontroller.h
    editing/uigrid.h
    editing/uigridcontroller.cpp
    editing/uigridcontroller.h
    editing/uioverlayview.cpp
    editing/uioverlayview.h
    editing/uiselection.cpp
    editing/uiselection.h
    editing/uitagscontroller.cpp
    editing/uitagscontroller.h
    editing/uitemplatecontroller.cpp
    editing/uitemplatecontroller.h
    editing/uitemplatesettingscontroller.cpp
    editing/uitemplatesettingscontroller.h
    editing/uiundomanager.cpp
    editing/uiundomanager.h
    editing/uiviewcreatecontroller.cpp
    editing/uiviewcreatecontroller.h
    viewcreator/animationsplashscreencreator.cpp
    viewcreator/animationsplashscreencreator.h
    viewcreator/animknobcreator.cpp
    viewcreator/animknobcreator.h
    viewcreator/checkboxcreator.cpp
    viewcreator/checkboxcreator.h
    viewcreator/controlcreator.cpp
    viewcreator/controlcreator.h
    viewcreator/gradientviewcreator.cpp
    viewcreator/gradientviewcreator.h
    viewcreator/kickbuttoncreator.cpp
    viewcreator/kickbuttoncreator.h
    viewcreator/knobcreator.cpp
    viewcreator/knobcreator.h
    viewcreator/layeredviewcontainercreator.cpp
    viewcreator/layeredviewcontainercreator.h
    viewcreator/moviebitmapcreator.cpp
    viewcreator/moviebitmapcreator.h
    viewcreator/moviebuttoncreator.cpp
    viewcreator/moviebuttoncreator.h
    viewcreator/multibitmapcontrolcreator.cpp
    viewcreator/multibitmapcontrolcreator.h
    viewcreator/multilinetextlabelcreator.cpp
    viewcreator/multilinetextlabelcreator.h
    viewcreator/onoffbuttoncreator.cpp
    viewcreator/onoffbuttoncreator.h
    viewcreator/optionmenucreator.cpp
    viewcreator/optionmenucreator.h
    viewcreator/paramdisplaycreator.cpp
    viewcreator/paramdisplaycreator.h
    viewcreator/rockerswitchcreator.cpp
    viewcreator/rockerswitchcreator.h
    viewcreator/rowcolumnviewcreator.cpp
    viewcreator/rowcolumnviewcreator.h
    viewcreator/scrollviewcreator.cpp
    viewcreator/scrollviewcreator.h
    viewcreator/searchtexteditcreator.cpp
    viewcreator/searchtexteditcreator.h
    viewcreator/segmentbuttoncreator.cpp
    viewcreator/segmentbuttoncreator.h
    viewcreator/shadowviewcontainercreator.cpp
    viewcreator/shadowviewcontainercreator.h
    viewcreator/sliderviewcreator.cpp
    viewcreator/sliderviewcreator.h
    viewcreator/splitviewcreator.cpp
    viewcreator/splitviewcreator.h
    viewcreator/stringlistcontrolcreator.cpp
    viewcreator/stringlistcontrolcreator.h
    viewcreator/switchcreators.cpp
    viewcreator/switchcreators.h
    viewcreator/textbuttoncreator.cpp
    viewcreator/textbuttoncreator.h
    viewcreator/texteditcreator.cpp
    viewcreator/texteditcreator.h
    viewcreator/textlabelcreator.cpp
    viewcreator/textlabelcreator.h
    viewcreator/uiviewswitchcontainercreator.cpp
    viewcreator/uiviewswitchcontainercreator.h
    viewcreator/viewcontainercreator.cpp
    viewcreator/viewcontainercreator.h
    viewcreator/viewcreator.cpp
    viewcreator/viewcreator.h
    viewcreator/vumetercreator.cpp
    viewcreator/vumetercreator.h
    viewcreator/xypadcreator.cpp
    viewcreator/xypadcreator.h
)

##########################################################################################
if(CMAKE_HOST_APPLE)
endif()

##########################################################################################
if(MSVC)
endif()

##########################################################################################
add_library(${target} STATIC ${${target}_sources})

add_dependencies(${target} vstgui)
target_link_libraries(${target} PRIVATE vstgui)
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
vstgui_set_cxx_version(${target} 14)
vstgui_source_group_by_folder(${target})

##########################################################################################
if(LINUX)
    set (VSTGUI_GENERATED_DIR ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.generated)
    
    execute_process(
        COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/editing/createuidescdata.sh ${VSTGUI_GENERATED_DIR}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/editing/
        RESULT_VARIABLE
            result
    )

    if(result EQUAL 0)
        target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS} "HAVE_EDITORUIDESC_H")
    else()
        message(FATAL_ERROR "Generating editoruidesc.h failed: ${result}")
    endif()

    target_include_directories(${target}
        PRIVATE
            ${VSTGUI_GENERATED_DIR}
    )
endif()
##########################################################################################
find_package(EXPAT)
if(EXPAT_FOUND)
    target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS} "VSTGUI_USE_SYSTEM_EXPAT")
    target_link_libraries(${target} PRIVATE expat)
else()
    message(STATUS "VSTGUI will use the embedded Expat package!")
endif()
