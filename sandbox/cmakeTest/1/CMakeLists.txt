make_test_filename (_filename testfile)

add_custom_command(OUTPUT ${_filename}
  COMMAND date "+%F %T" > ${_filename}
  VERBATIM)

add_custom_target (${_filename}_target
  DEPENDS ${_filename})

add_depend_to_target (aTarget ${_filename}_target)

make_test_filename (_static_file extra)
add_file_to_list (allFiles ${_filename} ${CMAKE_CURRENT_SOURCE_DIR}/${_static_file})

add_subdirectory (a)

