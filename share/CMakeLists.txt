#
# $Id$
#
# share CMakeLists.txt
#

set (_gmt_share_dirs conf cpt custom dbase mgd77 mgg pattern
	pslib time x2sys)

if (GSHHS_PATH)
	# only attempt to install shorelines when path is known
	list (APPEND _gmt_share_dirs ${GSHHS_PATH})
endif (GSHHS_PATH)

# install path must be relative
file (RELATIVE_PATH _install_path ${CMAKE_INSTALL_PREFIX} ${GMT_SHARE_PATH})

# install target for data
install (DIRECTORY ${_gmt_share_dirs}
	DESTINATION ${_install_path}
	COMPONENT Runtime
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN ".cmake" EXCLUDE)

# create coastline.conf which is needed for in-build-dir ctest
configure_file (coastline.conf.cmake coastline.conf @ONLY)

# add subtrees
add_subdirectory (conf)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
