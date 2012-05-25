##convenience function for string manipulation 



function(replace_occurence output input pattern replacer)
	string(REGEX REPLACE ${pattern} ${replacer} tmp_str ${input})
	set(${output} ${tmp_str} PARENT_SCOPE)
endfunction(replace_occurence output input pattern replacer)


function(replace_all_occurence)
  PARSE_ARGUMENTS(REPLACE_ALL
    "LIST_PATTERN;LIST_REPLACER"
    ""
    ${ARGN}
    )
	LIST(APPEND list_pattern ${REPLACE_ALL_LIST_PATTERN})
	LIST(APPEND list_replacer ${REPLACE_ALL_LIST_REPLACER})
	
	LIST(LENGTH list_pattern list_size )
	LIST(LENGTH list_replacer list2_size )
	math(EXPR list_size ${list_size}-1)
	LIST(GET  REPLACE_ALL_DEFAULT_ARGS 0 output_var)
	LIST(GET  REPLACE_ALL_DEFAULT_ARGS 1 intput_content )
	
	SET(tmp_str ${intput_content})
	SET(tmp_str2 "")
	
	foreach(i RANGE ${list_size})
		list(GET list_pattern ${i} current_pattern )
		list(GET list_replacer ${i} current_replacer )
		replace_occurence(tmp_str2 ${tmp_str} ${current_pattern} ${current_replacer} )
		SET(tmp_str ${tmp_str2})
	endforeach(i RANGE ${list_size})

	SET(${output_var} ${tmp_str} PARENT_SCOPE)
endfunction(replace_all_occurence)

function(STRING_APPEND var_name content)
SET(${var_name} "${${var_name}}${content}" PARENT_SCOPE)

endfunction(STRING_APPEND var_name content)
