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

# Install license files
if (NOT LICENSE_RESTRICTED STREQUAL GPL)
	install (FILES
		${COPYING_LGPL}
		DESTINATION ${GMT_DOC_PATH}/copying
		COMPONENT Runtime)
endif (NOT LICENSE_RESTRICTED STREQUAL GPL)
install (FILES
	${GMT_SOURCE_DIR}/LICENSE.TXT
	${COPYING_GPL}
	${GMT_EXTRA_LICENSE_FILES}
	DESTINATION ${GMT_DOC_PATH}/copying
	COMPONENT Runtime)

if (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install documentation files from external location without creating PDF
	# documentation first

	install (DIRECTORY ${GMT_INSTALL_EXTERNAL_DOC}/
		DESTINATION ${GMT_DOC_PATH}
		COMPONENT Documentation
		USE_SOURCE_PERMISSIONS)

else (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Create PDF documentation from scratch if possible

	# Find LaTeX
	find_package (LATEX)

	# Add tests and make verbatim copies from scripts
	add_subdirectory (scripts)

	if (PDFLATEX_COMPILER)
		# Copy common TeX dependends
		foreach (_input ${_docs_inputs} ${_common_inputs})
			add_custom_command (OUTPUT ${_input}
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${CMAKE_CURRENT_SOURCE_DIR}/${_input} ${_input}
				DEPENDS ${_input})
		endforeach (_input ${_common_inputs})
		add_custom_target (copy_tex_depends DEPENDS ${_docs_inputs} ${_common_inputs})

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

		# Make PDF figures
		add_subdirectory (fig)
		add_subdirectory (ppt) # must be added after fig

		foreach (_pdf_file ${_docs_output})
			string (REPLACE ".pdf" ".tex" _tex_file ${_pdf_file})
			string (REPLACE ".pdf" "_press.pdf" _shrink_file ${_pdf_file})
			string (REPLACE ".pdf" "_pdf" _pdf_target ${_pdf_file})

			add_latex_document (${_tex_file}
				DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/GMT_version.tex
				USE_INDEX
				DEFAULT_PDF
				MANGLE_TARGET_NAMES)

			# Add pdf to gmt_doc target
			add_depend_to_target (gmt_doc ${_pdf_target})

			# Make sure PS figures are converted to PDF before LaTeX runs
			add_dependencies (${_pdf_target} ${_common_depends})

			# Shrink PDF files
			add_custom_target (${_pdf_target}_shrink
				COMMAND gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.4
				-dPDFSETTINGS=/prepress -dNOPAUSE -dQUIET -dBATCH
				-sOutputFile=${_shrink_file} ${_pdf_file}
				COMMAND ${CMAKE_COMMAND} -E rename
				${_shrink_file} ${_pdf_file}
				DEPENDS gmt_doc)
			add_depend_to_target (gmt_doc_shrink ${_pdf_target}_shrink)

			# Install pdf file (if available)
			install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${_pdf_file}
				DESTINATION ${GMT_DOC_PATH}/pdf
				COMPONENT Documentation
				OPTIONAL)

			# Install target realease documentation
			add_custom_target (${_pdf_target}_release
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${_pdf_file} ${GMT_RELEASE_PREFIX}/doc_release/pdf/${_pdf_file}
				DEPENDS svn_export_release ${_pdf_file})
			add_depend_to_target (gmt_release ${_pdf_target}_release)
		endforeach (_pdf_file)
	endif (PDFLATEX_COMPILER)

	# Create and install pdf manpages file (if available)
	if (_manfiles_ps AND PS2PDF_CONVERTER)
		# create file with list of PS manpages
		set (_manfiles_ps_relative)
		foreach (_manfile ${_manfiles_ps})
			file(RELATIVE_PATH _rel_path ${CMAKE_CURRENT_BINARY_DIR} ${_manfile})
			list (APPEND _manfiles_ps_relative ${_rel_path})
		endforeach (_manfile ${_manfiles_ps})
		foreach (_manfile ${_manfiles_suppl_ps})
			file(RELATIVE_PATH _rel_path ${CMAKE_CURRENT_BINARY_DIR} ${_manfile})
			list (APPEND _manfiles_suppl_ps_relative ${_rel_path})
		endforeach (_manfile ${_manfiles_suppl_ps})
		string (REPLACE ";" "\n" _manfiles_ps_relative "${_manfiles_ps_relative}")
		string (REPLACE ";" "\n" _manfiles_suppl_ps_relative "${_manfiles_suppl_ps_relative}")
		file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/manfiles_ps.list ${_manfiles_ps_relative})
		file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/manfiles_suppl_ps.list ${_manfiles_suppl_ps_relative})

		# using the list, concatenate PS manpages and convert to PDF
		add_custom_command (OUTPUT GMT_Manpages.pdf
			COMMAND xargs cat < manfiles_ps.list |
			${PS2PDF_CONVERTER} - GMT_Manpages.pdf
			DEPENDS manpages_all
			VERBATIM)
		add_custom_command (OUTPUT GMT_Manpages_suppl.pdf
			COMMAND xargs cat < manfiles_suppl_ps.list |
			${PS2PDF_CONVERTER} - GMT_Manpages_suppl.pdf
			DEPENDS manpages_all
			VERBATIM)
		add_custom_target (gmt_pdf_manpages
			DEPENDS GMT_Manpages.pdf GMT_Manpages_suppl.pdf)
		add_depend_to_target (gmt_doc gmt_pdf_manpages)
		install (FILES
			${CMAKE_CURRENT_BINARY_DIR}/GMT_Manpages.pdf
			${CMAKE_CURRENT_BINARY_DIR}/GMT_Manpages_suppl.pdf
			DESTINATION ${GMT_DOC_PATH}/pdf
			COMPONENT Documentation
			OPTIONAL)

		# Install target for realease documentation
		add_custom_target (gmt_pdf_manpages_release
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			GMT_Manpages.pdf
			${GMT_RELEASE_PREFIX}/doc_release/pdf/GMT_Manpages.pdf
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			GMT_Manpages_suppl.pdf
			${GMT_RELEASE_PREFIX}/doc_release/pdf/GMT_Manpages_suppl.pdf
			DEPENDS gmt_pdf_manpages svn_export_release)
		add_depend_to_target (gmt_release gmt_pdf_manpages_release)
	endif (_manfiles_ps AND PS2PDF_CONVERTER)

endif (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

# Install target for examples
install (DIRECTORY examples
	DESTINATION ${GMT_DOC_PATH}
	COMPONENT Documentation
	USE_SOURCE_PERMISSIONS
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	REGEX ".cmake$" EXCLUDE)

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
