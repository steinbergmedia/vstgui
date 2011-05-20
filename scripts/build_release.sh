#!/bin/sh

MAJORVERSION=4_0
RELEASEVERSION=beta3

RELEASENAME=vstgui_$MAJORVERSION\_$RELEASEVERSION
CHECKOUTDIR=tmp
SVNDIR=tags/vstgui$MAJORVERSION/$RELEASEVERSION/vstgui/

# Export vstgui from Sourceforge
mkdir -p $CHECKOUTDIR
cd $CHECKOUTDIR
svn export https://vstgui.svn.sourceforge.net/svnroot/vstgui/$SVNDIR vstgui

if [[ -d vstgui/doxygen ]]
then

	# Build documentation
	cd vstgui/doxygen
	doxygen Doxyfile

	# Build archive
	cd ../../
	tar cjvf $RELEASENAME.tar.bz2 .

	# Cleanup
	rm -r vstgui

fi
