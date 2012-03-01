#
# $Id$
#
# Useful CMake variables.
#
# There are three configuration files:
#   1) "ConfigDefault.cmake" - is version controlled and used to add new default
#      variables and set defaults for everyone.
#   2) "ConfigUser.cmake" in the source tree - is not version controlled
#      (currently listed in svn:ignore property) and used to override defaults on
#      a per-user basis.
#   3) "ConfigUser.cmake" in the build tree - is used to override
#      "ConfigUser.cmake" in the source tree.
#
# NOTE: If you want to change CMake behaviour just for yourself then copy
#      "ConfigUserTemplate.cmake" to "ConfigUser.cmake" and then edit
#      "ConfigUser.cmake" (not "ConfigDefault.cmake" or "ConfigUserTemplate.cmake").
#
include ("${CMAKE_SOURCE_DIR}/cmake/ConfigDefault.cmake")

# If "ConfigUser.cmake" doesn't exist then create one for convenience.
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")
	include ("${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")
endif (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")

# If you've got a 'ConfigUser.cmake' in the build tree then that overrides the
# one in the source tree.
if (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")
	include ("${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")
endif (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")

###########################################################
# Do any needed processing of the configuration variables #
###########################################################

# Build type
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
	set (DEBUG_BUILD TRUE)
endif (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")

# Here we change it to add the SVN revision number for non-public releases - see Package.cmake for
# why this has to be done here.
set (GMT_PACKAGE_VERSION_WITH_SVN_REVISION ${GMT_PACKAGE_VERSION})
# Add the Subversion version number to the package filename if this is a non-public release.
# A non-public release has an empty 'GMT_SOURCE_CODE_CONTROL_VERSION_STRING' variable in 'ConfigDefault.cmake'.
set (HAVE_SVN_VERSION)
if (NOT GMT_SOURCE_CODE_CONTROL_VERSION_STRING)
	# Get the location, inside the staging area location, to copy the application bundle to.
	execute_process (
		COMMAND svnversion ${GMT_SOURCE_DIR}
		RESULT_VARIABLE SVN_VERSION_RESULT
		OUTPUT_VARIABLE SVN_VERSION_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	if (SVN_VERSION_RESULT)
		message (STATUS "Unable to determine svn version number for non-public release - ignoring.")
	else (SVN_VERSION_RESULT)
		if (SVN_VERSION_OUTPUT MATCHES "Unversioned")
			message (STATUS "Unversioned source tree, non-public release.")
		else (SVN_VERSION_OUTPUT MATCHES "Unversioned")
			# The 'svnversion' command can output a range of revisions with a colon
			# separator - but this causes problems with filenames so we'll remove the
			# colon and the end revision after it.
			string (REGEX REPLACE ":.*$" "" SVN_VERSION ${SVN_VERSION_OUTPUT})
			if (NOT SVN_VERSION STREQUAL exported)
				# Set the updated package version.
				set (GMT_PACKAGE_VERSION_WITH_SVN_REVISION "${GMT_PACKAGE_VERSION}_r${SVN_VERSION}")
				set (HAVE_SVN_VERSION TRUE)
			endif (NOT SVN_VERSION STREQUAL exported)
		endif (SVN_VERSION_OUTPUT MATCHES "Unversioned")
	endif (SVN_VERSION_RESULT)
endif (NOT GMT_SOURCE_CODE_CONTROL_VERSION_STRING)

# The current GMT version.
set (GMT_VERSION_STRING "${GMT_PACKAGE_NAME} ${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")

set (GMT_LONG_VERSION_STRING "${GMT_PACKAGE_NAME} - ${GMT_PACKAGE_DESCRIPTION_SUMMARY}, Version ${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")

# Get date
if (WIN32)
	execute_process (COMMAND "${GMT_SOURCE_DIR}/getdate.bat" OUTPUT_VARIABLE _today)
	#string (REGEX REPLACE "(..)/(..)/(....).*" "\3-\2-\1" _today ${_today})
elseif (UNIX)
	set(ENV{LANG} en_US)
	execute_process (COMMAND "date" "+%Y %m %d %B" OUTPUT_VARIABLE _today)
else (WIN32)
	message (WARNING "Date not implemented")
	set(_today "2013 13 13 Undecember")
endif (WIN32)
if (_today)
	string (REPLACE "\n" "" _today ${_today})
	string (REPLACE " " ";" _today ${_today})
	list(GET _today 0 YEAR)
	list(GET _today 1 MONTH)
	list(GET _today 2 DAY)
	list(GET _today 3 MONTHNAME)
	list(GET _today 0 1 2 DATE)
	string (REPLACE ";" "-" DATE "${DATE}")
	set (_today)
endif ()

# set package date
if (NOT GMT_VERSION_YEAR)
	set (GMT_VERSION_YEAR ${YEAR})
endif (NOT GMT_VERSION_YEAR)

# apply license restrictions
if (LICENSE_RESTRICTED) # on
	if (LICENSE_RESTRICTED STREQUAL GPL)
		# restrict to GPL
	elseif (LICENSE_RESTRICTED STREQUAL LGPL)
		# restrict to LGPL
	else (LICENSE_RESTRICTED STREQUAL GPL)
		# unknown license
		message (WARNING "unknown license: ${LICENSE_RESTRICTED}")
	endif (LICENSE_RESTRICTED STREQUAL GPL)
	# restrictions that apply to any of the above licenses
	# disable Shewchuk's triangle routine
	set (TRIANGLE_D OFF)
else (LICENSE_RESTRICTED) # off
	# no restrictions at all
	# enable Shewchuk's triangle routine
	set (TRIANGLE_D ON)
endif (LICENSE_RESTRICTED)

# set default triangulation method
if (TRIANGLE_D)
	set (GMT_TRIANGULATE "Shewchuk")
else (TRIANGLE_D)
	set (GMT_TRIANGULATE "Watson")
endif (TRIANGLE_D)

# GMT paths used in the code
if (NOT GMT_SHARE_PATH)
	# do not reset user setting
	if (GMT_INSTALL_MONOLITHIC)
		set (GMT_SHARE_PATH "share")
	else(GMT_INSTALL_MONOLITHIC)
		set (GMT_SHARE_PATH
			"share/gmt-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")
	endif(GMT_INSTALL_MONOLITHIC)
endif (NOT GMT_SHARE_PATH)

if (NOT GMT_DOC_PATH)
	# do not reset user setting
	if (GMT_INSTALL_MONOLITHIC)
		set (GMT_DOC_PATH "share/doc")
	else(GMT_INSTALL_MONOLITHIC)
		set (GMT_DOC_PATH
			"share/doc/gmt-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")
	endif(GMT_INSTALL_MONOLITHIC)
endif (NOT GMT_DOC_PATH)

# Install path for GMT binaries, headers and libraries
if (GMT_INSTALL_MONOLITHIC)
	set (GMT_INCDIR include)
	set (GMT_LIBDIR lib)
	set (GMT_BINDIR bin)
else (GMT_INSTALL_MONOLITHIC)
	set (GMT_INCDIR include/gmt-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION})
	set (GMT_LIBDIR lib/gmt-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}/lib)
	set (GMT_BINDIR lib/gmt-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}/bin)
endif (GMT_INSTALL_MONOLITHIC)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
#set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# use, i.e. don't skip the full RPATH for the build tree
set (CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}")
if (APPLE)
	# on OSX make rpath relative to executable dir:
	file (RELATIVE_PATH _rpath /${GMT_BINDIR} /${GMT_LIBDIR})
	# remove trailing /:
	string (REGEX REPLACE "/$" "" _rpath "${_rpath}")
	set (CMAKE_INSTALL_NAME_DIR @loader_path/${_rpath})
endif (APPLE)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
