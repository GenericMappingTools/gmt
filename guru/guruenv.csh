#!/bin/csh
#
#	$Id: guruenv.csh,v 1.3 2000-12-29 19:35:14 pwessel Exp $
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment
#
setenv MAKE make							# Must be GNU make or compatible
setenv MAN2HTML "man2html"						# Standard setting
#setenv MAN2HTML "man2html -leftm 1 -topm 8 -botm 6 -pgsize 65"		# Use this for HPUX
#setenv MAN2HTML "man2html -sun"					# Use this for Sun/Solaris
setenv CVSROOT :pserver:anonymous@gmt.soest.hawaii.edu:/home/gmt/cvs	# anonymous for reading; writers need specific account
