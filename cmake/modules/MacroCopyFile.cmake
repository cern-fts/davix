# - macro_copy_file(_src _dst)
# Copies a file to ${_dst} only if ${_src} is different (newer) than ${_dst}
#
# Example:
# macro_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/icon.png ${CMAKE_CURRENT_BINARY_DIR}/.)
# Copies file icon.png to ${CMAKE_CURRENT_BINARY_DIR} directory
#
# Copyright (c) 2006-2007  Wengo
# Copyright (c) 2006-2008  Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro(copy_files GLOBPAT DESTINATION)
  file(GLOB COPY_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLOBPAT})
  add_custom_target(copy ALL
    COMMENT "Copying files: ${GLOBPAT}")

  foreach(FILENAME ${COPY_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(DST "${DESTINATION}/${FILENAME}")

    add_custom_command(
      TARGET copy
      COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
      )
  endforeach(FILENAME)
endmacro(copy_files)

