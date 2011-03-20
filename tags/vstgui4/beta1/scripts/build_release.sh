#!/bin/sh

RELEASENAME=vstgui_4_0_beta_1
CHECKOUTDIR=tmp
SVNDIR=tags/vstgui4/beta1/vstgui/

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

# Cleanup
rm -r vstgui
