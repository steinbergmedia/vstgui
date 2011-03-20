#!/bin/sh

RELEASENAME=vstgui_beta_1
CHECKOUTDIR=tmp
SVNDIR=trunk/vstgui/

# Export vstgui from Sourceforge
mkdir -p $CHECKOUTDIR
cd $CHECKOUTDIR
svn export https://vstgui.svn.sourceforge.net/svnroot/vstgui/$SVNDIR vstgui

# Build documentation
cd vstgui/doxygen
doxygen Doxyfile

# Build archive
cd ../../
tar cjvf $RELEASENAME.tar.bz2 .
