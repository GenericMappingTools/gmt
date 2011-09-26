#
# $Id$
#
# share CMakeLists.txt 
#

# install target for data
install (DIRECTORY ./ DESTINATION ${GMT_SHARE_PATH}
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN ".cmake" EXCLUDE
	REGEX "[.]in$" EXCLUDE
)

# add subtrees
add_subdirectory (conf)
