#!/bin/bash
echo `pwd`
mkdir -p "$1"

rm -f "$1/editoruidesc.h"
echo -n "constexpr auto editorUIDesc = R\"====(" >> "$1/editoruidesc.h"
cat uidescriptioneditor.uidesc >> "$1/editoruidesc.h"
echo ")====\";" >> "$1/editoruidesc.h"

echo -n "constexpr auto editorUILightDesc = R\"====(" >> "$1/editoruidesc.h"
cat uidescriptioneditor_res_light.uidesc >> "$1/editoruidesc.h"
echo ")====\";" >> "$1/editoruidesc.h"

echo -n "constexpr auto editorUIDarkDesc = R\"====(" >> "$1/editoruidesc.h"
cat uidescriptioneditor_res_dark.uidesc >> "$1/editoruidesc.h"
echo ")====\";" >> "$1/editoruidesc.h"

