#!/bin/bash
echo `pwd`
mkdir -p "$1"
rm -f "$1/editoruidesc.h"
echo -n "constexpr auto editorUIDesc = R\"====(" >> "$1/editoruidesc.h"
cat uidescriptioneditor.uidesc >> "$1/editoruidesc.h"
echo ")====\";" >> "$1/editoruidesc.h"

