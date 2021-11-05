function(prepare_source_files SRC_FILES)
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/inc/)
        message("Creating inc directory")
        file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/inc/)
    endif ()
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/src/)
        message("Creating src directory")
        file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
    endif ()
    include_directories(inc/)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
            ${PROJECT_SOURCE_DIR}/inc/*.hpp
            ${PROJECT_SOURCE_DIR}/src/*.cpp
            )
    set(${SRC_FILES} ${SOURCES} PARENT_SCOPE)
endfunction()

function(set_project_properties)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # using Clang
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "-g -g3 -glldb -ggdb -ggdb3 -O0 -Wall -Wextra -Wpedantic -Wfloat-equal -Werror")
            set(CMAKE_CXX_FLAGS "-DDEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "-g0 -ggdb0 -glldb0 -O3")
            set(CMAKE_CXX_FLAGS "-DRELEASE")
        endif (CMAKE_BUILD_TYPE STREQUAL "Release")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "-g -ggdb3 -O0 -Wall -Wextra -Wpedantic -Wfloat-equal -Werror")
            set(CMAKE_CXX_FLAGS "-DDEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "-g0 -ggdb0 -O3")
            set(CMAKE_CXX_FLAGS "-DRELEASE")
        endif (CMAKE_BUILD_TYPE STREQUAL "Release")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # using Visual Studio C++
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "/Zi /Od /JMC /WX")
            set(CMAKE_CXX_FLAGS "/DDEBUG")
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "/Ot /O2")
            set(CMAKE_CXX_FLAGS "/DRELEASE")
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

function(link_to_my_lib LIB_NAME)
    include_directories(PRIVATE
            /home/mitya/MyLibs/${LIB_NAME}/${CMAKE_BUILD_TYPE}/inc
            )
    link_directories(PRIVATE
            /home/mitya/MyLibs/${LIB_NAME}/${CMAKE_BUILD_TYPE}/lib/static
            )
    link_libraries(PRIVATE
            -l${LIB_NAME}
            )
endfunction()