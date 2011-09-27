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
)

# add subtrees
add_subdirectory (conf)
