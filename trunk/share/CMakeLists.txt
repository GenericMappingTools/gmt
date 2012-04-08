#
# $Id$
#
# share CMakeLists.txt
#

set (_gmt_share_dirs conf cpt custom dbase mgd77 mgg pattern
	pslib time x2sys)

# install target for data
install (DIRECTORY ${_gmt_share_dirs}
	DESTINATION ${GMT_SHARE_PATH}
	COMPONENT Runtime
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	REGEX "[.](cmake|in)$" EXCLUDE)

# install version file (used during runtime to check for correct share dir)
configure_file (VERSION.in VERSION @ONLY NEWLINE_STYLE LF)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
	DESTINATION ${GMT_SHARE_PATH}
	COMPONENT Runtime)

# only attempt to install shorelines when requested and path is known
if (GSHHS_PATH AND COPY_GSHHS)
	install (DIRECTORY ${GSHHS_PATH}/
		DESTINATION ${GMT_SHARE_PATH}/coast
		COMPONENT GSHHS)
	# set installed location
	set (GSHHS_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${GMT_SHARE_PATH}/coast)
	message ("${GSHHS_INSTALL_PATH}")
else (GSHHS_PATH AND COPY_GSHHS)
	# set installed location
	set (GSHHS_INSTALL_PATH ${GSHHS_PATH})
endif (GSHHS_PATH AND COPY_GSHHS)

# create coastline.conf which is needed for in-build-dir ctest
configure_file (coastline.conf.in coastline.conf @ONLY)

# add subtrees
add_subdirectory (conf)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
