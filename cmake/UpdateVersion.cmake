set(PATCH 0)

execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe
            --tags --abbrev=0 --match "v[0-9]*"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE LAST_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE TAG_RESULT
)

if(TAG_RESULT EQUAL 0)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-list
                --count "${LAST_TAG}..HEAD"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE PATCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE COUNT_RESULT
    )

    if(NOT COUNT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to count Git commits")
    endif()
endif()

set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${PATCH}")
set(VERSION_HEADER "${BUILD_DIR}/version.h")

set(CONTENT
    "#pragma once\n#define APP_VERSION \"${APP_VERSION}\"\n"
)

if(EXISTS "${VERSION_HEADER}")
    file(READ "${VERSION_HEADER}" OLD_CONTENT)

    if(OLD_CONTENT STREQUAL CONTENT)
        return()
    endif()
endif()

file(WRITE "${VERSION_HEADER}" "${CONTENT}")

message(STATUS "Application version: ${APP_VERSION}")
