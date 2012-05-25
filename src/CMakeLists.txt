# main file for src


FILE(GLOB src_davix "*.cpp")

FILE(GLOB src_neon "neon/*.cpp")
FILE(GLOB src_logger "logging/*.c")
FILE(GLOB src_curl "curl/*.cpp")

include_directories("libs")

add_subdirectory(libs)
add_subdirectory(datetime)
add_subdirectory(logging)
add_subdirectory(xmlpp)
if ( ${HTTP_LIB} STREQUAL "NEON" )

    add_subdirectory(neon)
    SET(SRC_HTTP ${src_neon})

else ( ${HTTP_LIB} STREQUAL "NEON" )

    add_subdirectory(curl)
    SET(SRC_HTTP ${src_curl})

endif ( ${HTTP_LIB} STREQUAL "NEON" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(${GLIBMM_PKG_CFLAGS} ${HTTPLIB_PKG_CFLAGS} ${LIBXMLPP_PKG_CFLAGS})
add_library(davix  SHARED ${src_davix} ${SRC_HTTP}  ${src_xmlparser} ${src_logger} ${src_datetime})


target_link_libraries(davix ${GLIBMM_PKG_LIBRARIES} ${HTTPLIB_PKG_LIBRARIES})

target_link_libraries(davix  ${LIBXMLPP_PKG_LIBRARIES})


