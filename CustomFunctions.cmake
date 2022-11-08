function(prepare_source_files INC_FILES SRC_FILES)
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/inc/)
        message("Creating inc directory")
        file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/inc/)
    endif ()
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/src/)
        message("Creating src directory")
        file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
    endif ()
    include_directories(inc/)
    file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS
            ${PROJECT_SOURCE_DIR}/inc/*.hpp
            )
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
            ${PROJECT_SOURCE_DIR}/src/*.cpp
            )
    set(${INC_FILES} ${INCLUDES} PARENT_SCOPE)
    set(${SRC_FILES} ${SOURCES} PARENT_SCOPE)
endfunction()

function(set_project_properties)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # using Clang
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "-g -g3 -glldb -ggdb -ggdb3 -O0 -Wall -Wextra -Wpedantic -Wfloat-equal -Werror")
            set(CMAKE_CXX_FLAGS "-DTRISTAN_DEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "-g0 -ggdb0 -glldb0 -O3 -DLOG_DISABLE_TRACE -DLOG_DISABLE_DEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Release")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "-g -ggdb3 -O0 -Wall -Wextra -Wpedantic -Wfloat-equal -Werror")
            set(CMAKE_CXX_FLAGS "-DTRISTAN_DEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "-g0 -ggdb0 -O3 -DLOG_DISABLE_TRACE -DLOG_DISABLE_DEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Release")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # using Visual Studio C++
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "/Zi /Od /JMC /WX")
            set(CMAKE_CXX_FLAGS "/DTRISTAN_DEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "/Ot /O2 /DLOG_DISABLE_TRACE /DLOG_DISABLE_DEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Release")
    else ()
        message(FATAL_ERROR "Compiler not supported")
    endif ()
endfunction()

function(generate_doxygen_config DESCRIPTION)
    get_target_property(L_VERSION ${PROJECT_NAME} VERSION)
    message(STATUS "Generating doxygen config file")
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/doxygen.conf)
        file(TOUCH ${PROJECT_SOURCE_DIR}/doxygen.conf)
        file(WRITE ${PROJECT_SOURCE_DIR}/doxygen.conf
                    "
                DOXYFILE_ENCODING     = UTF-8
                PROJECT_NAME           = \"${PROJECT_NAME}\"
                PROJECT_NUMBER         = ${L_VERSION}
                PROJECT_BRIEF          = \"${DESCRIPTION}\"
                OUTPUT_DIRECTORY       = Docs
                EXTRACT_ALL            = YES
                EXTRACT_PRIV_VIRTUAL   = YES
                EXTRACT_LOCAL_CLASSES  = YES
                QUIET                  = YES
                INPUT                  = ${PROJECT_SOURCE_DIR}/inc/
                ENUM_VALUES_PER_LINE   = 1
                "
                )
    endif (NOT EXISTS ${PROJECT_SOURCE_DIR}/doxygen.conf)
endfunction()


function(generate_uninstall_script PROJECT_INSTALL_DIR CMAKE_CONFIG_INSTALL_DIR GENERAL_INSTALL_DIR UNINSTALLER_NAME)

    file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt FILES_TO_DELETE)
    file(WRITE ${UNINSTALLER_NAME} "#!/usr/bin/env bash\n\n")
    file(APPEND ${UNINSTALLER_NAME} "rm -v")
    foreach (FILE ${FILES_TO_DELETE})
        file(APPEND ${UNINSTALLER_NAME} " ${FILE}")
    endforeach ()
    file(APPEND ${UNINSTALLER_NAME} "\n")
    file(APPEND ${UNINSTALLER_NAME} "rm -v -rf ${PROJECT_INSTALL_DIR}\n\n")
    file(APPEND ${UNINSTALLER_NAME} "if [ -z \"$(ls -A ${CMAKE_CONFIG_INSTALL_DIR})\" ]; then\n")
    file(APPEND ${UNINSTALLER_NAME} "\trm -v -rf ${CMAKE_CONFIG_INSTALL_DIR}\n")
    file(APPEND ${UNINSTALLER_NAME} "fi\n\n")
    file(APPEND ${UNINSTALLER_NAME} "if [ -z \"$(ls -A ${GENERAL_INSTALL_DIR}/cmake)\" ]; then\n")
    file(APPEND ${UNINSTALLER_NAME} "\trm -v -rf ${GENERAL_INSTALL_DIR}/cmake\n")
    file(APPEND ${UNINSTALLER_NAME} "fi\n\n")
    file(APPEND ${UNINSTALLER_NAME} "if [ -z \"$(ls -A ${GENERAL_INSTALL_DIR})\" ]; then\n")
    file(APPEND ${UNINSTALLER_NAME} "\trm -v -rf ${GENERAL_INSTALL_DIR}\n")
    file(APPEND ${UNINSTALLER_NAME} "fi\n\n")
    file(APPEND ${UNINSTALLER_NAME} "exit 0\n")

endfunction()

#function(target_link_external_lib TARGET EXTERNAL_LIBS_FOLDER LIB_NAME LIB_BRANCH)
#
#    if (NOT EXISTS ${EXTERNAL_LIBS_FOLDER})
#        message(STATUS "Creating ${EXTERNAL_LIBS_FOLDER}")
#        file(MAKE_DIRECTORY ${EXTERNAL_LIBS_FOLDER})
#    endif (NOT EXISTS ${EXTERNAL_LIBS_FOLDER})
#
#    if (NOT EXISTS ${EXTERNAL_LIBS_FOLDER}/${LIB_NAME}/${CMAKE_BUILD_TYPE})
#        MESSAGE(STATUS "Downloading ${LIB_NAME}")
#        file(MAKE_DIRECTORY ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME})
#        file(DOWNLOAD https://github.com/Mitya1983/${LIB_NAME}/archive/${LIB_BRANCH}.zip
#                SHOW_PROGRESS
#                ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}/${LIB_NAME}.zip
#                HTTPHEADER "Authorization: token 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"
#                HTTPHEADER "Accept: application/vnd.github.v3.raw"
#                )
#        message(STATUS "Extracting ${LIB_NAME}")
#        file(ARCHIVE_EXTRACT
#                INPUT ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}/${LIB_NAME}.zip
#                DESTINATION ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}
#                )
#        file(MAKE_DIRECTORY ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}/${LIB_NAME}-${LIB_BRANCH}/build)
#        message(STATUS "Configuring ${LIB_NAME}")
#        execute_process(
#                WORKING_DIRECTORY ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}/${LIB_NAME}-${LIB_BRANCH}/build
#                COMMAND cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ../
#        )
#        message(STATUS "Building DateTime")
#        execute_process(
#                WORKING_DIRECTORY ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}/${LIB_NAME}-${LIB_BRANCH}/build
#                COMMAND cmake --build .
#        )
#        message(STATUS "Installing DateTime")
#        execute_process(
#                WORKING_DIRECTORY ${EXTERNAL_LIBS_FOLDER}/sources_${LIB_NAME}/${LIB_NAME}-${LIB_BRANCH}/build
#                COMMAND cmake --install . --prefix "${EXTERNAL_LIBS_FOLDER}/${LIB_NAME}"
#        )
#        message(STATUS "Cleaning ${LIB_NAME} sources and build files")
#        execute_process(
#                WORKING_DIRECTORY ${EXTERNAL_LIBS_FOLDER}
#                COMMAND rm -r sources_${LIB_NAME}
#        )
#    endif (NOT EXISTS ${EXTERNAL_LIBS_FOLDER}/${LIB_NAME}/${CMAKE_BUILD_TYPE})
#
#    target_include_directories(${TARGET} PUBLIC
#            ${EXTERNAL_LIBS_FOLDER}/${LIB_NAME}/${CMAKE_BUILD_TYPE}/inc
#            )
#    target_link_directories(${TARGET} PUBLIC
#            ${EXTERNAL_LIBS_FOLDER}/${LIB_NAME}/${CMAKE_BUILD_TYPE}/lib
#            )
#    target_link_libraries(${TARGET} PUBLIC STATIC
#            -l${LIB_NAME}
#            )
#endfunction()
