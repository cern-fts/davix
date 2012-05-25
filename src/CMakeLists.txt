# main file for src


FILE(GLOB src_davix "*.cpp")

FILE(GLOB src_neon "neon/*.cpp")
FILE(GLOB src_logger "logging/*.c")
FILE(GLOB src_curl "curl/*.cpp")

add_subdirectory(datetime)
add_subdirectory(logging)
add_subdirectory(xmlpp)
add_subdirectory(neon)
add_subdirectory(curl)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(${GLIBMM_PKG_CFLAGS} ${LIBCURL_PKG_CFLAGS} ${LIBXMLPP_PKG_CFLAGS})
add_library(davix  SHARED ${src_davix} ${src_neon} ${src_curl} ${src_xmlparser} ${src_logger} ${src_datetime})


target_link_libraries(davix ${GLIBMM_PKG_LIBRARIES} ${LIBCURL_PKG_LIBRARIES})

target_link_libraries(davix  ${LIBXMLPP_PKG_LIBRARIES})


