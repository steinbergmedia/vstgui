set(SCRIPTING_SOURCES
    tiny-js/TinyJS_Functions.cpp
    tiny-js/TinyJS_Functions.h
    tiny-js/TinyJS_MathFunctions.cpp
    tiny-js/TinyJS_MathFunctions.h
    tiny-js/TinyJS.cpp
    tiny-js/TinyJS.h
    uiscripting.cpp
    uiscripting.h
    uiscripting.md
)
add_library(vstgui_uiscripting STATIC ${SCRIPTING_SOURCES})
target_compile_definitions(vstgui_uiscripting ${VSTGUI_COMPILE_DEFINITIONS})
vstgui_set_cxx_version(vstgui_uiscripting 17)
vstgui_source_group_by_folder(vstgui_uiscripting)

option(VSTGUI_SCRIPTING_TINYJS_TESTS "Add Tiny-JS test target" OFF)
if(VSTGUI_SCRIPTING_TINYJS_TESTS)
    add_executable(uidescription_scripting_tiny_js_test
        tiny-js/run_tests.cpp
        tiny-js/TinyJS_Functions.cpp
        tiny-js/TinyJS_Functions.h
        tiny-js/TinyJS_MathFunctions.cpp
        tiny-js/TinyJS_MathFunctions.h
        tiny-js/TinyJS.cpp
        tiny-js/TinyJS.h
    )
    target_compile_definitions(uidescription_scripting_tiny_js_test ${VSTGUI_COMPILE_DEFINITIONS})
    vstgui_set_cxx_version(uidescription_scripting_tiny_js_test 17)
endif(VSTGUI_SCRIPTING_TINYJS_TESTS)
