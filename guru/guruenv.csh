#!/bin/csh
#
#	$Id: guruenv.csh,v 1.1 2000-12-29 01:49:17 pwessel Exp $
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment
#
setenv MAKE make							# Must be GNU make or compatible
setenv CVSROOT :pserver:anonymous@gmt.soest.hawaii.edu:/home/gmt/cvs	# anonymous for reading; writers need specific account
setenv GMT_SHARED_LIBS --enable-shared					# --enable-shared for shared; empty for static libraries
setenv GMT_US --enable-US						# --enable-US for inch; empty for cm
setenv GMT_TRIANGLE --enable-triangle					# --enable-triangle for Shewchuck's triangulation; empty for Watson's
setenv GMT_DEBUG --enable-debug						# --enable-debug will use -g; else -O with compiler
