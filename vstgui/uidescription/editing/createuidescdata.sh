# /bin/bash
echo `pwd`
rm -f editoruidesc.h
echo -n "constexpr auto editorUIDesc = R\"====(" >> editoruidesc.h
cat uidescriptioneditor.uidesc >> editoruidesc.h
echo ")====\";" >> editoruidesc.h

