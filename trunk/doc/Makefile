#	$Id: Makefile,v 1.3 2011-03-27 17:03:39 remko Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
#		Makefile for GMT Version 5 documentation
#
#	Normally, this makefile is activated by the main Makefile in
#	the GMT main directory.  However, you can also issue commands
#	directly to this makefile from within the doc directory.
#
#	Date:		25-MAR-2011

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include ../src/config.mk	# GMT-specific settings determined by user & configure

install::
		@if [ . -ef $(docdir) ]; then \
			echo "Install doc directory the same as distribution doc directory - nothing copied"; \
		else \
			mkdir -p $(docdir); \
			cp -pr html pdf examples tutorial $(docdir); \
			rm -rf $(docdir)/examples/{orig,tests,*.ps,*.pdf,*.png} \
				{*,*/*,*/*/*}/{CVS,.gmt*,.cvs*}; \
		fi

uninstall:
		@if [ . -ef $(docdir) ]; then \
			echo "Install doc directory the same as distribution doc directory - nothing deleted"; \
		else \
			\rm -rf $(docdir); \
		fi
