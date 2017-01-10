##########################################################################################
# VSTGUI UIDescription Library
##########################################################################################
set(target vstgui_uidescription)

set(${target}_sources
	base64codec.h
	cstream.cpp
	cstream.h
	delegationcontroller.h
	icontroller.h
	iuidescription.h
	iviewcreator.h
	iviewfactory.h
	uiattributes.cpp
	uiattributes.h
	uidescription.cpp
	uidescription.h
	uidescriptionfwd.h
	uiviewcreator.cpp
	uiviewcreator.h
	uiviewfactory.cpp
	uiviewfactory.h
	uiviewswitchcontainer.cpp
	uiviewswitchcontainer.h
	xmlparser.cpp
	xmlparser.h
	editing/iaction.h
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
	editing/uisearchtextfield.cpp
	editing/uisearchtextfield.h
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
)

##########################################################################################
if(CMAKE_HOST_APPLE)
endif()

##########################################################################################
if(MSVC)
endif()

##########################################################################################
add_library(${target} STATIC ${${target}_sources})

target_link_libraries(${target} PRIVATE vstgui)
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
vstgui_set_cxx_version(${target} 11)
vstgui_source_group_by_folder(${target})

##########################################################################################
if(UNIX AND NOT CMAKE_HOST_APPLE)
	execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/editing/createuidescdata.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/editing/ RESULT_VARIABLE result)
	target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS} "HAVE_EDITORUIDESC_H")
endif()
