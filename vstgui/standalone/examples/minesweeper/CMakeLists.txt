##########################################################################################
# VSTGUI Minesweeper Example Application
##########################################################################################
set(target Minesweeper)

set(${target}_sources
  "source/commands.h"
  "source/highscorelist.cpp"
  "source/highscorelist.h"
  "source/highscores.cpp"
  "source/highscores.h"
  "source/highscoreviewcontroller.cpp"
  "source/highscoreviewcontroller.h"
  "source/keepchildviewscentered.h"
  "source/minefieldviewcontroller.cpp"
  "source/minefieldviewcontroller.h"
  "source/model.cpp"
  "source/model.h"
  "source/myapp.cpp"
  "source/windowcontroller.cpp"
  "source/windowcontroller.h"
)

set(Minesweeper_resources
  "resource/Window.uidesc"
  "resource/Minesweeper.icns"
)

set(Minesweeper_font
  "resource/font/LCD14.otf"
  "resource/font/NotoEmoji-Regular.ttf"
)

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" )
vstgui_add_resources(${target} "${Minesweeper_resources}")
vstgui_add_resources(${target} "${Minesweeper_font}" "Fonts")
vstgui_set_target_bundle_id(${target} "vstgui.examples.minesweeper")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/Minesweeper.rc")
vstgui_set_cxx_version(${target} 17)
target_include_directories(${target} PRIVATE ../../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES_FOLDER})

