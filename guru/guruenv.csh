#!/bin/csh
#
#	$Id: guruenv.csh,v 1.4 2001-01-03 22:42:04 pwessel Exp $
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment
#
setenv AWK gawk								# nawk, gawk, or compatible
setenv MAN2HTML "man2html"						# Use this for Linux (Standard setting)
#setenv MAN2HTML "man2html -leftm 1 -topm 8 -botm 6 -pgsize 65"		# Use this for HPUX
#setenv MAN2HTML "man2html -sun"					# Use this for Sun/Solaris
setenv CVSROOT :pserver:anonymous@gmt.soest.hawaii.edu:/home/gmt/cvs	# anonymous for reading; writers need a specific account
