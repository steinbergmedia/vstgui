##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target unittests)

set(${target}_sources
  "tests/unittest/unittests.cpp"
  "tests/unittest/unittests.h"
  "tests/unittest/lib/animation/animations_test.cpp"
  "tests/unittest/lib/animation/animator_test.cpp"
  "tests/unittest/lib/animation/timingfunction_tests.cpp"
  "tests/unittest/lib/controls/ccheckbox_test.cpp"
  "tests/unittest/lib/controls/ccontrol_test.cpp"
  "tests/unittest/lib/controls/conoffbutton_test.cpp"
  "tests/unittest/lib/controls/csegmentbutton_test.cpp"
  "tests/unittest/lib/controls/ctextbutton_test.cpp"
  "tests/unittest/lib/controls/cxypad_test.cpp"
  "tests/unittest/lib/cbitmap_test.cpp"
  "tests/unittest/lib/cbuttonstate_test.cpp"
  "tests/unittest/lib/ccolor_test.cpp"
  "tests/unittest/lib/cframe_test.cpp"
  "tests/unittest/lib/clinestyle_test.cpp"
  "tests/unittest/lib/cpoint_test.cpp"
  "tests/unittest/lib/crect_test.cpp"
  "tests/unittest/lib/csplitview_test.cpp"
  "tests/unittest/lib/cview_test.cpp"
  "tests/unittest/lib/cviewcontainer_test.cpp"
  "tests/unittest/lib/idependency_test.cpp"
  "tests/unittest/lib/platform_helper.h"
  "tests/unittest/lib/utf8string_test.cpp"
  "tests/unittest/lib/utf8stringview_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/canimationsplashscreencreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/canimknobcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/ccheckboxcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/ccontrolcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cgradientviewcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/chorizontalswitchcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/ckickbuttoncreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cknobcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/clayeredviewcontainercreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cmoviebitmapcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cmoviebuttoncreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/conoffbuttoncreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/coptionmenucreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cparamdisplaycreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/crockerswitchcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/crowcolumnviewcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cscrollviewcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/csegmentbuttoncreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cshadowviewcontainercreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cslidercreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/csplitviewcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/ctextbuttoncreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/ctexteditcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/ctextlabelcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cverticalswitchcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cviewcontainercreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cviewcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cvumetercreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/cxypadcreator_test.cpp"
  "tests/unittest/uidescription/uiviewcreator/helpers.h"
  "tests/unittest/uidescription/uiviewcreator/uiviewswitchcontainercreator_test.cpp"
  "tests/unittest/uidescription/base64codec.cpp"
  "tests/unittest/uidescription/cstream_test.cpp"
  "tests/unittest/uidescription/delegationcontroller_test.cpp"
  "tests/unittest/uidescription/uiattributes_test.cpp"
  "tests/unittest/uidescription/uidescription_test.cpp"
  "tests/unittest/uidescription/uidescriptionadapter.h"
  "tests/unittest/uidescription/uiviewfactory_test.cpp"
  "tests/unittest/uidescription/uiviewswitchcontainer_test.cpp"
  "tests/unittest/uidescription/xmlparser_test.cpp"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
  set(${target}_sources
    ${${target}_sources}
    "tests/unittest/lib/platform_helper_mac.mm"
  )

	set(${target}_PLATFORM_LIBS
		"-framework Cocoa"
		"-framework OpenGL"
		"-framework QuartzCore"
		"-framework Accelerate"
	)
endif()

##########################################################################################
if(MSVC)
  set(${target}_sources
    ${${target}_sources}
    "tests/unittest/lib/platform_helper_win32.cpp"
  )
endif()

##########################################################################################
include_directories(../)
add_executable(${target}
  ${${target}_sources}
  ${simple_standalone_resources}
)
target_link_libraries(${target}
	vstgui
  vstgui_uidescription
	${${target}_PLATFORM_LIBS}
)

vstgui_set_cxx_version(${target} 11)
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS} ENABLE_UNIT_TESTS=1)
vstgui_source_group_by_folder(${target})

add_custom_command(TARGET ${target} POST_BUILD COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittests")
