#
# $Id$
#
# conf CMakeLists.txt
#

# Generate system defaults file
set (GMT_CONF_TEMPLATE "System-wide")
if (UNITS EQUAL "US")
	set (GMT_PS_CHAR_ENCODING "Standard+")
	set (GMT_PROJ_LENGTH_UNIT "inch")
	set (GMT_PS_MEDIA "letter")
else (UNITS EQUAL "US")
	set (GMT_PS_CHAR_ENCODING "ISOLatin1+")
	set (GMT_PROJ_LENGTH_UNIT "cm")
	set (GMT_PS_MEDIA "a4")
endif (UNITS EQUAL "US")
configure_file (gmt.conf.cmake gmt.conf)

set (GMT_CONF_TEMPLATE "Template US")
set (GMT_PS_CHAR_ENCODING "Standard+")
set (GMT_PROJ_LENGTH_UNIT "inch")
set (GMT_PS_MEDIA "letter")
configure_file (gmt.conf.cmake gmt_US.conf)

set (GMT_CONF_TEMPLATE "Template SI")
set (GMT_PS_CHAR_ENCODING "ISOLatin1+")
set (GMT_PROJ_LENGTH_UNIT "cm")
set (GMT_PS_MEDIA "a4")
configure_file (gmt.conf.cmake gmt_SI.conf)

set (GMT_CONFIG_FILES
	${CMAKE_CURRENT_BINARY_DIR}/gmt.conf
	${CMAKE_CURRENT_BINARY_DIR}/gmt_US.conf
	${CMAKE_CURRENT_BINARY_DIR}/gmt_SI.conf
)

install (FILES ${GMT_CONFIG_FILES} DESTINATION ${GMT_SHARE_PATH}/conf)

