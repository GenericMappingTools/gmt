#!/bin/csh
#
#	$Id: guruenv.csh,v 1.2 2000-12-29 02:37:44 pwessel Exp $
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment
#
setenv MAKE make							# Must be GNU make or compatible
setenv MAN2HTML "man2html"						# Standard setting
#setenv MAN2HTML "man2html -leftm 1 -topm 8 -botm 6 -pgsize 65"		# Use this for HPUX
#setenv MAN2HTML "man2html -sun"					# Use this for Sun/Solaris
setenv GMT050dpi "convert -density 50x50 -page 425x550"
setenv GMT100dpi "convert -density 100x100 -page 850x1100"
#setenv GMT050dpi "~wessel/bin/ps2gif -w 8.5 -h 11 -d 8 -r 50"
#setenv GMT100dpi "~wessel/bin/ps2gif -w 8.5 -h 11 -d 8 -r 100"
setenv CVSROOT :pserver:anonymous@gmt.soest.hawaii.edu:/home/gmt/cvs	# anonymous for reading; writers need specific account
setenv GMT_SHARED_LIBS --enable-shared					# --enable-shared for shared; empty for static libraries
setenv GMT_US --enable-US						# --enable-US for inch; empty for cm
setenv GMT_TRIANGLE --enable-triangle					# --enable-triangle for Shewchuck's triangulation; empty for Watson's
setenv GMT_DEBUG --enable-debug						# --enable-debug will use -g; else -O with compiler
