function(enable_doxygen src_dir)
  option(ENABLE_DOXYGEN "Enable doxygen doc builds of source" OFF)
  if(ENABLE_DOXYGEN)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    find_package(Doxygen REQUIRED dot)
    set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/doc)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_EXCLUDE_PATTERNS "*/build/*")
    doxygen_add_docs(doxygen-docs ${src_dir})



  endif()
endfunction()