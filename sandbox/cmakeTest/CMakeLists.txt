#
# setup/macros
#

cmake_minimum_required(VERSION 2.8.5)
project (cmakeTest)

# add_depend_to_target (TARGET DEPEND [ DEPEND [ DEPEND ... ]])
# example: add_depend_to_target (main_target custom_target)
macro (ADD_DEPEND_TO_TARGET _TARGET)
  if(NOT TARGET ${_TARGET})
    add_custom_target (${_TARGET}
      WORKING_DIRECTORY ${cmakeTest_BINARY_DIR})
  endif(NOT TARGET ${_TARGET})
  add_dependencies(${_TARGET} ${ARGN})
endmacro (ADD_DEPEND_TO_TARGET)

# add_file_to_list (LIST [ FILE [ FILE ... ]])
# if FILE is omitted then the list is cleared
# if FILE is not absolute then it is assumed to be in CMAKE_CURRENT_BINARY_DIR
# example: add_file_to_list (list file)
macro (ADD_FILE_TO_LIST _LIST)
  set (_files ${ARGN})
  if (_files)
    set (_files_abs)
    foreach (_file ${_files})
      if (_file MATCHES "^[^/]")
        # make absolute path
        file(RELATIVE_PATH _file / ${CMAKE_CURRENT_BINARY_DIR}/${_file})
      endif ()
      list(APPEND _files_abs /${_file})
    endforeach (_file ${_files})
    # append to list
    set (${_LIST} ${${_LIST}} ${_files_abs}
      CACHE INTERNAL "Global list")
  else (_theList)
    # clear list
    set (${_LIST} "" CACHE INTERNAL "Global list")
  endif (_files)
endmacro (ADD_FILE_TO_LIST)

# make_test_filename (VAR [SUFFIX])
macro (MAKE_TEST_FILENAME _VAR)
  set (_suffix ${ARGN})
  if (_suffix)
    list(GET _suffix 0 _suffix)
  elseif()
    set (_suffix testfile)
  endif ()
  file(RELATIVE_PATH ${_VAR} ${cmakeTest_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${_suffix})
  string (REPLACE "/" "-" ${_VAR} ${${_VAR}})
endmacro (MAKE_TEST_FILENAME)

#
# interesting bit
#

# maintain list of files
add_file_to_list (allFiles) # clear list

add_subdirectory (1)
add_subdirectory (2)

message (">> allFiles: ${allFiles}")

# make combinedFileList, depend only on files
add_custom_command(OUTPUT combinedFileList
  COMMAND cat ${allFiles} > combinedFileList
  DEPENDS ${allFiles}
  VERBATIM)

# make combinedFileTarget, depend only on targets
add_custom_command(OUTPUT combinedFileTarget
  COMMAND cat ${allFiles} > combinedFileTarget
  DEPENDS aTarget
  VERBATIM)

# make combinedFileTargetList, depend on both files+targets
add_custom_command(OUTPUT combinedFileTargetList
  COMMAND cat ${allFiles} > combinedFileTargetList
  DEPENDS aTarget dummy dummy_all ${allFiles}
  VERBATIM)

# triggers for the above
add_custom_target (combinedFileFromList
  DEPENDS combinedFileList)

add_custom_target (combinedFileFromTarget
  DEPENDS combinedFileTarget)

add_custom_target (combinedFileFromTargetList ALL
  DEPENDS combinedFileTargetList)

