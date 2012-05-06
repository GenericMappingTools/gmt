cmake_minimum_required(VERSION 2.8.5)
project (docbookTest)

# Path of the DocBook XSL stylesheets
# install package docbook-xsl-ns or download from
# http://sourceforge.net/projects/docbook/files/
#set (XSL_STYLESHEETS /opt/local/share/xsl/docbook-xsl-ns)
set (XSL_STYLESHEETS ${CMAKE_CURRENT_SOURCE_DIR}/docbook-xsl-ns)

#include (FindLibXslt)
#include (FindLibXml2)
find_program (LIBXSLT_XSLTPROC_EXECUTABLE xsltproc)
find_program (LIBXML2_XMLLINT_EXECUTABLE xmllint)

if (NOT LIBXSLT_XSLTPROC_EXECUTABLE)
  message (FATAL_ERROR "Need to install xsltproc.")
endif ()

if (NOT LIBXML2_XMLLINT_EXECUTABLE)
  message (WARNING "Optionally install xmllint to verify xml.")
endif ()

set (YEAR 2012)
set (MONTHNAME March)
set (GMT_PACKAGE_VERSION_WITH_SVN_REVISION 5.0.2)

configure_file (info.xml.in info.xml @ONLY)

set (XML_FILES common-env.xml common-opts.xml conf-files.xml
  gmt.conf.5.xml gmtswitch.1.xml grdcut.1.xml
  installation.xml introduction.xml manual.xml)

set (TROFF_FILES gmt.conf.5 gmtswitch.1 grdcut.1)

if (LIBXML2_XMLLINT_EXECUTABLE)
  # validate XML
  add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xml.is-valid
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/xml.is-valid
    COMMAND ${LIBXML2_XMLLINT_EXECUTABLE}
    --noout --nonet --xinclude --relaxng docbookxi.rng
    # --noxincludenode
    manual.xml && ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/xml.is-valid
    DEPENDS ${XML_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM)

  add_custom_target(validate ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/xml.is-valid)
endif ()

# html manual
add_custom_command (OUTPUT manual.html
  COMMAND ${LIBXSLT_XSLTPROC_EXECUTABLE}
  --param section.autolabel 1
  --param section.label.includes.component.label 1
  --param html.stylesheet 'style.css'
  --param xref.with.number.and.title 1
  --param toc.section.depth 3
  --param admon.style ''
  --param callout.graphics.extension '.gif'
  --param contrib.inline.enabled 0
  --nonet
  --xinclude
  --output ${CMAKE_CURRENT_BINARY_DIR}/manual.html
  ${XSL_STYLESHEETS}/html/docbook.xsl
  manual.xml
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  style.css ${CMAKE_CURRENT_BINARY_DIR}/style.css
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${XML_FILES}
  VERBATIM)

# troff manpages
add_custom_command (OUTPUT ${TROFF_FILES}
  COMMAND ${LIBXSLT_XSLTPROC_EXECUTABLE}
  --param section.autolabel 1
  --param section.label.includes.component.label 1
  --param xref.with.number.and.title 1
  --param contrib.inline.enabled 0
  --nonet
  --xinclude
  --output ${CMAKE_CURRENT_BINARY_DIR}/
  ${XSL_STYLESHEETS}/manpages/docbook.xsl
  manual.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${XML_FILES}
  VERBATIM)

add_custom_target (man ALL
  DEPENDS manual.html ${TROFF_FILES})
add_dependencies (man validate)

