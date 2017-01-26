# @title cmake macro for pkgconfig files generation
# @brief generate a .pc package config file with a given name
# @author Adrien Devresse

include(DefineInstallationPaths REQUIRED)
include(CMakeMacroParseArguments REQUIRED)
include(CMakeStringHelpers REQUIRED)


SET(CMAKE_PKGCONFIG_TEMPLATE "prefix=@PREFIX@
exec_prefix=@PREFIX@
libdir=@LIBDIR_VAR
includedir=@INCLUDE_VAR@

Name: @NAME_PROJECT@
Description: @DESCRIPTION_PROJECT@
Version: @VERSION_PROJECT@
URL: @URL_PROJECT@
Requires: @REQUIRES_PROJECT@
Conflicts: @CONFLICTS_PROJECT@
Libs: @LIBS_PROJECT@
Libs.private: @LIBS_PRIVATE_PROJECT@
Cflags: @CFLAGS_PROJECT@
")


SET(CMAKE_PKGCONFIG_TEMPLATE_BASE "
prefix=@PREFIX@
exec_prefix= \\\${prefix}
libdir= @LIBDIR_VAR@
includedir=@INCLUDE_VAR@

Name: @NAME_PROJECT@
Description: @DESCRIPTION_PROJECT@
Version: @VERSION_PROJECT@
Requires: @REQUIRES_PROJECT@
Libs: @LIBS_PROJECT@
Cflags: @CFLAGS_PROJECT@
" )


LIST(APPEND CMAKE_PKGCONFIG_TEMPLATE_BASE_PATTERN "@PREFIX@" "@LIBDIR_VAR@"
			"@INCLUDE_VAR@" "@NAME_PROJECT@" "@DESCRIPTION_PROJECT@" "@VERSION_PROJECT@"
			"@REQUIRES_PROJECT@" "@LIBS_PROJECT@" "@CFLAGS_PROJECT@")


# main function to use
# FORMAT : add_PkgConfigFile_for_Library("string_filename.pc" target_library
#											[DESCRIPTION] "description of the pkgconfig files"
#											[HEADER_DIRS] dir1, dir2 
#											[REQUIRES] req1 req 2 ) # list of dir to include in $prefix/include/, ex : $prefix/include/dir1
# the pc file is produced in the ${CMAKE_CURRENT_BINARY_DIR} directory

function(add_PkgConfigFile_for_Library)
	PARSE_ARGUMENTS(PKGCONFIGFILE
                "HEADER_DIRS;DESCRIPTION;REQUIRES;CFLAGS"
		""
		${ARGN}
    )

	LIST(GET  PKGCONFIGFILE_DEFAULT_ARGS 0 pkgconfig_filename)
	LIST(GET  PKGCONFIGFILE_DEFAULT_ARGS 1 lib_target)
	LIST(GET  PKGCONFIGFILE_DESCRIPTION 0 description)

	get_target_property(library_name ${lib_target} OUTPUT_NAME)
	get_target_property(library_version ${lib_target} VERSION)
	
	set(pkgconfig_prefix "${CMAKE_INSTALL_PREFIX}")
	set(pkgconfig_libdir_var "\\\${prefix}/lib${LIB_SUFFIX}")
	set(pkgconfig_include_var "\\\${prefix}/include")
	
	set(pkgconfig_linkflags "-l${library_name} -L\\\${libdir}")
	set(pkgconfig_name "${pkgconfig_filename}")
	set(pkgconfig_version "${library_version}")
	set(pkgconfig_description "pkgconfig file for ${library_name}")
	set(pkgconfig_requires " ")
        set(pkgconfig_cflags "")

	IF(PKGCONFIGFILE_REQUIRES)
		FOREACH(req ${PKGCONFIGFILE_REQUIRES})
			set(pkgconfig_requires "${pkgconfig_requires} ${req}")	
		ENDFOREACH(req PKGCONFIGFILE_REQUIRES)
	ENDIF(PKGCONFIGFILE_REQUIRES)

        IF(PKGCONFIGFILE_CFLAGS)
                FOREACH(req ${PKGCONFIGFILE_CFLAGS})
                        set(pkgconfig_cflags "${pkgconfig_cflags} ${req}")
                ENDFOREACH(req PKGCONFIGFILE_CFLAGS)
        ENDIF(PKGCONFIGFILE_CFLAGS)
	
	IF(PKGCONFIGFILE_HEADER_DIRS)
		FOREACH(dir ${PKGCONFIGFILE_HEADER_DIRS})
			set(pkgconfig_includedir "${pkgconfig_includedir} -I\\\${includedir}/${dir}")	
		ENDFOREACH(dir PKGCONFIGFILE_HEADER_DIRS)
	ELSE(PKGCONFIGFILE_HEADER_DIRS)
		set(pkgconfig_includedir " -I\\\${includedir}")
	ENDIF(PKGCONFIGFILE_HEADER_DIRS)	
	
	IF(description)
		set(pkgconfig_description "${description}")	
	
	ENDIF(description)
	
        set(pkgconfig_cflags "${pkgconfig_cflags} ${pkgconfig_includedir} ")
	
	LIST(APPEND pkgconfig_list_var ${pkgconfig_prefix} ${pkgconfig_libdir_var} 
								${pkgconfig_include_var} ${pkgconfig_name} ${pkgconfig_description}
								${pkgconfig_version} ${pkgconfig_requires} ${pkgconfig_linkflags} ${pkgconfig_cflags})
	replace_all_occurence(pc_file_content ${CMAKE_PKGCONFIG_TEMPLATE_BASE}
							LIST_PATTERN ${CMAKE_PKGCONFIG_TEMPLATE_BASE_PATTERN} LIST_REPLACER ${pkgconfig_list_var})

	SET(filename "${CMAKE_CURRENT_BINARY_DIR}/${pkgconfig_filename}")
	FILE(WRITE ${filename} "${pc_file_content}" )
	message(STATUS "generate pkgconfig file for ${lib_target} under ${filename}")
endfunction(add_PkgConfigFile_for_Library)

