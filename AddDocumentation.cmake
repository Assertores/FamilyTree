include_guard()

find_package(Doxygen REQUIRED)
include(FindDoxygen)

set(_ADDDOCS_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/AddDocumentation")

if(NOT TARGET ALL_DOCS)
    add_custom_target(ALL_DOCS)

    set_target_properties(ALL_DOCS
        PROPERTIES
            FOLDER "misc"
    )
endif()

function(add_documentation ARG_NAME)
    # 1. Check if target with given name already exists.
    if(TARGET ${ARG_NAME})
        message(FATAL_ERROR "Target '${ARG_NAME}' already exists.")
        return()
    endif()

    cmake_parse_arguments(ARG "" "DISPLAY_NAME;MAINPAGE;EXAMPLE;COMPONENT" "API;FILES" ${ARGN})
    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_documentation: unrecognized argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT Doxygen_FOUND)
        message(WARNING "doxygen not found, documentation was not compiled.")
        return()
    endif()

    set(INPUT_FILES)
    foreach(FILE ${ARG_FILES})
        cmake_path(ABSOLUTE_PATH FILE NORMALIZE)
        list(APPEND INPUT_FILES ${FILE})
    endforeach()
    if(DEFINED ARG_MAINPAGE)
        cmake_path(ABSOLUTE_PATH ARG_MAINPAGE NORMALIZE)
        list(APPEND INPUT_FILES ${ARG_MAINPAGE})
    endif()

    # 2. Configuration step
    # 2.1. Global configuration
    set(DOCS_STYLE_DIR "${CMAKE_SOURCE_DIR}/docs/doxygen")
    set(HTML_OUT_PREFIX "html")

    set(DOCS_OUT_DIR "${CMAKE_BINARY_DIR}/docs/${ARG_NAME}")
    if(DEFINED CMAKE_DOCS_OUTPUT_DIRECTORY)
        set(DOCS_OUT_DIR "${CMAKE_DOCS_OUTPUT_DIRECTORY}")
    endif()

    set(HTML_OUT_DIR "${DOCS_OUT_DIR}/${HTML_OUT_PREFIX}")

    set(MARKDOWN_FILES ${INPUT_FILES})
    list(FILTER MARKDOWN_FILES INCLUDE REGEX "\\.md")

    set(ASSET_FILES ${INPUT_FILES})
    list(FILTER ASSET_FILES EXCLUDE REGEX "\\.md")

    set(ALL_GENERATED_FILES)

    # 2.2. Doxygen specific configuration
    set(DOXYGEN_PROJECT_NAME ${ARG_DISPLAY_NAME})
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${ARG_MAINPAGE})
    set(DOXYGEN_STRIP_FROM_PATH ${ARG_API})
    set(DOXYGEN_EXAMPLE_PATH ${ARG_EXAMPLE})
    
    set(DOXYGEN_LAYOUT_FILE "${DOCS_STYLE_DIR}/DoxygenLayout.xml")
    set(DOXYGEN_HTML_EXTRA_STYLESHEET
        "${DOCS_STYLE_DIR}/css/doxygen-awesome.css"
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-sidebar-only.css"
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-sidebar-only-darkmode-toggle.css")
    set(DOXYGEN_HTML_EXTRA_FILES
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-darkmode-toggle.js"
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-fragment-copy-button.js"
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-interactive-toc.js"
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-paragraph-link.js"
        "${DOCS_STYLE_DIR}/css/doxygen-awesome-tabs.js")
    set(DOXYGEN_OUTPUT_DIRECTORY "${DOCS_OUT_DIR}")
    set(DOXYGEN_HTML_OUTPUT "${HTML_OUT_PREFIX}")
    
    set(DOXYGEN_FULL_PATH_NAMES YES)
    set(DOXYGEN_MARKDOWN_SUPPORT YES) # NOTE: default is YES but we don't want to take any risks of the default ever chainging
    set(DOXYGEN_DISABLE_INDEX NO)
    set(DOXYGEN_FULL_SIDEBAR NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HTML_COLORSTYLE LIGHT)
    set(DOXYGEN_SEPARATE_MEMBER_PAGES YES)
    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_LOCAL_CLASSES NO)
    set(DOXYGEN_TYPEDEF_HIDES_STRUCT YES)
    set(DOXYGEN_SHOW_HEADERFILE NO)
    set(DOXYGEN_SHOW_USED_GROUPS YES)

    # 3. Preprocess files
    # 3.1. Copy assets
    foreach(FILE_PATH ${ASSET_FILES})
        cmake_path(ABSOLUTE_PATH FILE_PATH NORMALIZE)
        cmake_path(GET FILE_PATH FILENAME FILE_NAME)

        set(OUTPUT_FILE "${HTML_OUT_DIR}/${FILE_NAME}")
        list(APPEND ALL_GENERATED_FILES ${OUTPUT_FILE})

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND "${CMAKE_COMMAND}" -E copy "${FILE_PATH}" "${OUTPUT_FILE}"
            MAIN_DEPENDENCY ${FILE_PATH}
            COMMENT "Copying asset '${FILE_PATH}'."
            VERBATIM)
    endforeach()

    set_source_files_properties(${ALL_GENERATED_FILES} PROPERTIES GENERATED TRUE)

    # 4. run doxygen
    doxygen_add_docs(${ARG_NAME}
        WORKING_DIRECTORY "${DOCS_OUT_DIR}"
        ${ARG_API}
        ${MARKDOWN_FILES}
        ${ALL_GENERATED_FILES})

    # 5. reset output directorys
    file(REMOVE_RECURSE "${DOCS_OUT_DIR}")
    file(MAKE_DIRECTORY "${DOCS_OUT_DIR}" "${HTML_OUT_DIR}")

    # 6. connect to ci
    add_dependencies(ALL_DOCS ${ARG_NAME})
    install(DIRECTORY ${HTML_OUT_DIR}
        COMPONENT ${ARG_COMPONENT}
        DESTINATION "docs/${ARG_COMPONENT}")
    install(DIRECTORY ${HTML_OUT_DIR}
        COMPONENT "ALL_DOCS"
        DESTINATION "docs/${ARG_COMPONENT}")
endfunction()
