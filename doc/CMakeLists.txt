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

include (GmtHelperMacros)

# TeX output PDFs
set (_docs_output GMT_Docs.pdf GMT_Tutorial.pdf GMT_API.pdf)

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
		copy_other_figures
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
	add_subdirectory (ppt) # must be added after fig

	foreach (_pdf_file ${_docs_output})
		string (REPLACE ".pdf" ".tex" _tex_file ${_pdf_file})
		string (REPLACE ".pdf" "_pdf" _pdf_target ${_pdf_file})

		add_latex_document (${_tex_file}
			USE_INDEX
			DEFAULT_PDF
			MANGLE_TARGET_NAMES)

		# Add pdf to gmt_doc target
		add_depend_to_target (gmt_doc ${_pdf_target})

		# Make sure PS figures are converted to PDF before LaTeX runs
		add_dependencies (${_pdf_target} ${_common_depends})

		# Install pdf file (if available)
		install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${_pdf_file}
			DESTINATION ${GMT_DOC_PATH}/pdf OPTIONAL)
	endforeach (_pdf_file)
endif (PDFLATEX_COMPILER)

# Install target for examples
install (DIRECTORY examples
	DESTINATION ${GMT_DOC_PATH}
	USE_SOURCE_PERMISSIONS
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN ".cmake" EXCLUDE)

# Run examples (test)
if (DO_EXAMPLES)
	# Rules for copying example files to build dir
	file (GLOB_RECURSE _example_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/* tutorial/*)
	list_regex_get ("[.]svn/|[.]cmake|CMakeLists.txt" _example_files ${_example_files} INVERT)
	set (_test_files)
	foreach (_file ${_example_files})
		add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}
			COMMAND ${CMAKE_COMMAND}
			-E copy_if_different
			${CMAKE_CURRENT_SOURCE_DIR}/${_file}
			${CMAKE_CURRENT_BINARY_DIR}/${_file})
		list (APPEND _test_files ${CMAKE_CURRENT_BINARY_DIR}/${_file})
	endforeach (_file ${_example_files})

	# Add file dependencies to check target
	add_custom_target (copy_example_tests
		DEPENDS ${_test_files})
	add_dependencies (check copy_example_tests)

	add_subdirectory (examples)
endif (DO_EXAMPLES)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
