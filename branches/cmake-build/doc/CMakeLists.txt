#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------

# Define minimum CMake version required
cmake_minimum_required (VERSION 2.8.5)

# Define project name
project (GMT_DOC)

include (GmtHelperMacros)

# TeX files
set (_docs_inputs GMT_Appendix_A.tex GMT_Appendix_B.tex GMT_Appendix_C.tex
	GMT_Appendix_D.tex GMT_Appendix_E.tex GMT_Appendix_F.tex GMT_Appendix_G.tex
	GMT_Appendix_H.tex GMT_Appendix_I.tex GMT_Appendix_J.tex GMT_Appendix_K.tex
	GMT_Appendix_L.tex GMT_Appendix_M.tex GMT_Appendix_N.tex GMT_Appendix_O.tex
	GMT_Appendix_P.tex GMT_Appendix_Q.tex GMT_Chapter_1.tex GMT_Chapter_2.tex
	GMT_Chapter_3.tex GMT_Chapter_4.tex GMT_Chapter_5.tex GMT_Chapter_6.tex
	GMT_Chapter_7.tex GMT_Chapter_8.tex GMT_Chapter_9.tex GMT_Frontmatter.tex)

set (_common_inputs GMT.ist GMT.sty GMT_macros.tex GMT_Cover.tex)

# Find LaTeX
find_package (LATEX)

if (PDFLATEX_COMPILER)
	# Copy common TeX dependends
	foreach (_input ${_common_inputs})
		add_custom_command (OUTPUT ${_input}
			COMMAND ${CMAKE_COMMAND}
			-E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${_input} ${_input}
			COMMENT "Copying ${_input}")
	endforeach (_input ${_common_inputs})
	add_custom_target (copy_tex_depends DEPENDS ${_common_inputs})

	set (_common_depends
		copy_tex_depends
		pdf_figures
		verbatim_examples
		verbatim_scripts)

	# Generate GMT_version.tex
	configure_file (GMT_version.tex.cmake GMT_version.tex)

	# Set TeX FLAGS
	set (MAKEINDEX_COMPILER_FLAGS "-s GMT.ist"
		CACHE STRING "Flags passed to makeindex.")
	set (PDFLATEX_COMPILER_FLAGS "--interaction=batchmode"
		CACHE STRING "Silence latex warnings.")

	# Find LaTeX
	include (UseLATEX)

	# Make verbatim copies from scripts
	add_subdirectory (scripts)

	# Make PDF figures
	add_subdirectory (fig)

	# GMT_Docs.pdf
	add_latex_document (GMT_Docs.tex
		INPUTS ${_docs_inputs}
		IMAGE_DIRS fig ppt
		USE_INDEX
		DEFAULT_PDF
		MANGLE_TARGET_NAMES)
	# add pdf to gmt_doc target
	add_depend_to_target (gmt_doc GMT_Docs_pdf)
	# Make sure PS figures are converted to PDF before LaTeX runs
	add_dependencies (GMT_Docs_pdf ${_common_depends})

	# GMT_Tutorial.pdf
	add_latex_document (GMT_Tutorial.tex
		DEPENDS copy_tex_depends pdf_figures verbatim_examples verbatim_scripts
		IMAGE_DIRS fig ppt
		USE_INDEX
		DEFAULT_PDF
		MANGLE_TARGET_NAMES)
	# add pdf to gmt_doc target
	add_depend_to_target (gmt_doc GMT_Tutorial_pdf)
	# Make sure PS figures are converted to PDF before LaTeX runs
	add_dependencies (GMT_Tutorial_pdf ${_common_depends})

	# GMT_API.pdf
	add_latex_document (GMT_API.tex
		DEPENDS copy_tex_depends pdf_figures verbatim_examples verbatim_scripts
		IMAGE_DIRS fig ppt
		USE_INDEX
		DEFAULT_PDF
		MANGLE_TARGET_NAMES)
	# add pdf to gmt_doc target
	add_depend_to_target (gmt_doc GMT_API_pdf)
	# Make sure PS figures are converted to PDF before LaTeX runs
	add_dependencies (GMT_API_pdf ${_common_depends})
endif (PDFLATEX_COMPILER)

# install target for examples
install (DIRECTORY examples
	DESTINATION ${GMT_DOC_PATH}
	USE_SOURCE_PERMISSIONS
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN ".cmake" EXCLUDE)

# run examples (test)
if (DO_EXAMPLES)
	# need to copy data first
	file(COPY examples tutorial
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
		USE_SOURCE_PERMISSIONS
		PATTERN ".svn" EXCLUDE
		PATTERN "CMakeLists.txt" EXCLUDE
		PATTERN ".cmake" EXCLUDE)

	add_subdirectory (examples)
endif (DO_EXAMPLES)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
