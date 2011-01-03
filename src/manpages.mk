#-------------------------------------------------------------------------------
#  $Id: manpages.mk,v 1.8 2011-01-03 01:50:33 guru Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 of the License.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#	GNUmakefile to create manpages for GMT Version 4.x
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile spells out how to create manpages (*.1, *.3)
#	out of text files *.txt. Dependencies are automatically created.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		24-SEP-2007
#-------------------------------------------------------------------------------
SUFFIXES:	.1 .3 .5 .txt

DEP1=$(MAN1:.1=.dep)
DEP3=$(MAN3:.3=.dep)
DEP5=$(MAN5:.5=.dep)

ifeq "$(strip $(GMTSRCDIR))" ""
	INCDIR=.
else
	INCDIR=$(GMTSRCDIR)
endif

ifneq "$(MAKECMDGOALS)" "spotless"
	include $(DEP1) $(DEP3) $(DEP5)
endif

%.1 %.3 %.5:		%.txt
		@cp -f $< junk.c
		$(TXT2MAN) -I$(INCDIR) junk.c | egrep -v '^#|^$$' > $@
		@rm -f junk.c

$(DEP1):	%.dep:	%.txt
		$(TXT2MAN) -I$(INCDIR) -MM -MG $*.txt | sed s,.o:,.1:, > $@

$(DEP3):	%.dep:	%.txt
		$(TXT2MAN) -I$(INCDIR) -MM -MG $*.txt | sed s,.o:,.3:, > $@

$(DEP5):	%.dep:	%.txt
		$(TXT2MAN) -I$(INCDIR) -MM -MG $*.txt | sed s,.o:,.5:, > $@

manpages:	$(MAN1) $(MAN3) $(MAN5)

spotless::
		\rm -f *.1 *.3 *.5 *.dep
