#!/bin/csh
#
#	$Id: guruenv.csh,v 1.7 2001-03-08 20:42:41 pwessel Exp $
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment before making GMT
#
# 1. Using awk
#   Different platforms have different version of awk.  While GMT configure
#   will determine this, you need $AWK to be set to run make_examples.csh etc
#   so you might as well set it to nawk, gawk, mawk, or awk.  Note old awk is
#   not smart enough so try the others first.

setenv AWK gawk								# nawk, gawk, or compatible

#
# 2. Enable Matlab
#   Some of the gurus may have Matlab installed on their system, and thus can
#   compile the mex files for the src/mex supplement.  Give the path to the
#   Matlab directory or set it to NONE if you dont have it

setenv MATLAB /usr/local/matlab						# Set to NONE if you do not have Matlab

#
# 3. Converting Man pages to HTML:
#  There seems to be two tools called man2html that does this task.  In the past I was
#  using man2html the Perl script written by Earl Hood and it had some special options
#  for Suns and I had to mess with the options for the HP.  Under Linux, man2html is
#  an executable program with completely other options.  The first choice below worked
#  with both packages under Linux; the two other choices are for the Perl script only.

setenv MAN2HTML "man2html"						# Use this for Linux (Standard setting)
#setenv MAN2HTML "man2html -leftm 1 -topm 8 -botm 6 -pgsize 65"		# Use this for HPUX
#setenv MAN2HTML "man2html -sun"					# Use this for Sun/Solaris

#
# 4. Simplifying the life of the guru
#   The Guru makefile contains numerous targets that only guru's will need to use,
#   e.g., make tar_all will tar off all the archives.  Since you can only issue
#   those make commands from the main GMT directory, the following alias is very
#   handy as it fails unless you are in the GMT directory:

alias gurumake 'make -f guru/Makefile.guru'				# Simple shorthand for guru making


#
# 5. The GMT Environment
#   Before you start issuing make commands, you should have these set properly
#
setenv GMTHOME <fullpathtoyour>/GMTdev/GMT
setenv MANPATH $GMTHOME/man						# Or add this part if MANPATH exists for other reasons
setenv NETCDFHOME  /usr/local						# Set this to where netcdf lives

#
# 6. Searchable path
#   Make sure you add $GMTHOME/bin to your path.

#
# 7. Authentication of CVS access
#   This parameter needs to be set so you dont have to use the -d option on every
#   cvs command.

setenv CVSROOT :pserver:anonymous@gmt.soest.hawaii.edu:/home/gmt/cvs	# anonymous for reading; writers need a specific account
