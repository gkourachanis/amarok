# - Try to find the Taglib-Extras library
# Once done this will define
#
#  TAGLIB-EXTRAS_FOUND - system has the taglib-extras library
#  TAGLIB-EXTRAS_CFLAGS - the taglib-extras cflags
#  TAGLIB-EXTRAS_LIBRARIES - The libraries needed to use taglib-extras

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT TAGLIB-EXTRAS_MIN_VERSION)
  set(TAGLIB-EXTRAS_MIN_VERSION "1.4")
endif(NOT TAGLIB-EXTRAS_MIN_VERSION)

if(NOT WIN32)
    find_program(TAGLIB-EXTRASCONFIG_EXECUTABLE NAMES taglib-extras-config PATHS
       ${BIN_INSTALL_DIR}
    )
endif(NOT WIN32)

#reset vars
set(TAGLIB-EXTRAS_LIBRARIES)
set(TAGLIB-EXTRAS_CFLAGS)

# if taglib-extras-config has been found
if(TAGLIB-EXTRASCONFIG_EXECUTABLE)

  exec_program(${TAGLIB-EXTRASCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB-EXTRAS_VERSION)

  if(TAGLIB-EXTRAS_VERSION STRLESS "${TAGLIB-EXTRAS_MIN_VERSION}")
     message(STATUS "TagLib-Extras version not found: version searched :${TAGLIB-EXTRAS_MIN_VERSION}, found ${TAGLIB-EXTRAS_VERSION}")
     set(TAGLIB-EXTRAS_FOUND FALSE)
  else(TAGLIB-EXTRAS_VERSION STRLESS "${TAGLIB-EXTRAS_MIN_VERSION}")

     exec_program(${TAGLIB-EXTRASCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB-EXTRAS_LIBRARIES)

     exec_program(${TAGLIB-EXTRASCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB-EXTRAS_CFLAGS)

     if(TAGLIB-EXTRAS_LIBRARIES AND TAGLIB-EXTRAS_CFLAGS)
        set(TAGLIB-EXTRAS_FOUND TRUE)
     endif(TAGLIB-EXTRAS_LIBRARIES AND TAGLIB-EXTRAS_CFLAGS)
     string(REGEX REPLACE " *-I" ";" TAGLIB-EXTRAS_INCLUDES "${TAGLIB-EXTRAS_CFLAGS}")
  endif(TAGLIB-EXTRAS_VERSION STRLESS "${TAGLIB-EXTRAS_MIN_VERSION}") 
  mark_as_advanced(TAGLIB-EXTRAS_CFLAGS TAGLIB-EXTRAS_LIBRARIES TAGLIB-EXTRAS_INCLUDES)

else(TAGLIB-EXTRASCONFIG_EXECUTABLE)

  find_path(TAGLIB-EXTRAS_INCLUDES
    NAMES
    tag.h
    PATH_SUFFIXES taglib-extras
    PATHS
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
  )

    IF(NOT WIN32)
      # on non-win32 we don't need to take care about WIN32_DEBUG_POSTFIX

      FIND_LIBRARY(TAGLIB-EXTRAS_LIBRARIES tag PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})

    ELSE(NOT WIN32)

      # 1. get all possible libnames
      SET(args PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})             
      SET(newargs "")               
      SET(libnames_release "")      
      SET(libnames_debug "")        

      LIST(LENGTH args listCount)

        # just one name
        LIST(APPEND libnames_release "tag")
        LIST(APPEND libnames_debug   "tagd")

        SET(newargs ${args})

      # search the release lib
      FIND_LIBRARY(TAGLIB-EXTRAS_LIBRARIES_RELEASE
                   NAMES ${libnames_release}
                   ${newargs}
      )

      # search the debug lib
      FIND_LIBRARY(TAGLIB-EXTRAS_LIBRARIES_DEBUG
                   NAMES ${libnames_debug}
                   ${newargs}
      )

      IF(TAGLIB-EXTRAS_LIBRARIES_RELEASE AND TAGLIB-EXTRAS_LIBRARIES_DEBUG)

        # both libs found
        SET(TAGLIB-EXTRAS_LIBRARIES optimized ${TAGLIB-EXTRAS_LIBRARIES_RELEASE}
                        debug     ${TAGLIB-EXTRAS_LIBRARIES_DEBUG})

      ELSE(TAGLIB-EXTRAS_LIBRARIES_RELEASE AND TAGLIB-EXTRAS_LIBRARIES_DEBUG)

        IF(TAGLIB-EXTRAS_LIBRARIES_RELEASE)

          # only release found
          SET(TAGLIB-EXTRAS_LIBRARIES ${TAGLIB-EXTRAS_LIBRARIES_RELEASE})

        ELSE(TAGLIB-EXTRAS_LIBRARIES_RELEASE)

          # only debug (or nothing) found
          SET(TAGLIB-EXTRAS_LIBRARIES ${TAGLIB-EXTRAS_LIBRARIES_DEBUG})

        ENDIF(TAGLIB-EXTRAS_LIBRARIES_RELEASE)

      ENDIF(TAGLIB-EXTRAS_LIBRARIES_RELEASE AND TAGLIB-EXTRAS_LIBRARIES_DEBUG)

      MARK_AS_ADVANCED(TAGLIB-EXTRAS_LIBRARIES_RELEASE)
      MARK_AS_ADVANCED(TAGLIB-EXTRAS_LIBRARIES_DEBUG)

    ENDIF(NOT WIN32)
  
  INCLUDE(FindPackageMessage)

  IF("DEFAULT_MSG" STREQUAL "DEFAULT_MSG")
    SET(_FAIL_MESSAGE "Could NOT find Taglib-Extras")
  ELSE("DEFAULT_MSG" STREQUAL "DEFAULT_MSG")
    SET(_FAIL_MESSAGE "DEFAULT_MSG")
  ENDIF("DEFAULT_MSG" STREQUAL "DEFAULT_MSG")

  STRING(TOUPPER Taglib-Extras _NAME_UPPER)

  # collect all variables which were not found, so they can be printed, so the
  # user knows better what went wrong (#6375)
  SET(MISSING_VARS "")
  SET(DETAILS "")
  SET(${_NAME_UPPER}_FOUND TRUE)
  IF(NOT TAGLIB-EXTRAS_INCLUDES OR TAGLIB-EXTRAS_LIBRARIES)
    SET(${_NAME_UPPER}_FOUND FALSE)
    SET(MISSING_VARS " TAGLIB-EXTRAS_INCLUDES TAGLIB-EXTRAS_LIBRARIES")
  ELSE(NOT TAGLIB-EXTRAS_INCLUDES OR TAGLIB-EXTRAS_LIBRARIES)
    SET(DETAILS "${DETAILS}[${TAGLIB-EXTRAS_INCLUDES} ${TAGLIB-EXTRAS_LIBRARIES}]")
  ENDIF(NOT TAGLIB-EXTRAS_INCLUDES OR TAGLIB-EXTRAS_LIBRARIES)

  # check if all passed variables are valid
  FOREACH(_CURRENT_VAR ${ARGN})
    IF(NOT ${_CURRENT_VAR})
      SET(${_NAME_UPPER}_FOUND FALSE)
      SET(MISSING_VARS "${MISSING_VARS} ${_CURRENT_VAR}")
    ELSE(NOT ${_CURRENT_VAR})
      SET(DETAILS "${DETAILS}[${${_CURRENT_VAR}}]")
    ENDIF(NOT ${_CURRENT_VAR})
  ENDFOREACH(_CURRENT_VAR)

  IF (${_NAME_UPPER}_FOUND)
    FIND_PACKAGE_MESSAGE(Taglib-Extras "Found Taglib-Extras: ${TAGLIB-EXTRAS_INCLUDES} ${TAGLIB-EXTRAS_LIBRARIES}" "${DETAILS}")
  ELSE (${_NAME_UPPER}_FOUND)
    IF (Taglib-Extras_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "${_FAIL_MESSAGE} (missing: ${MISSING_VARS})")
    ELSE (Taglib-Extras_FIND_REQUIRED)
      IF (NOT Taglib-Extras_FIND_QUIETLY)
        MESSAGE(STATUS "${_FAIL_MESSAGE}  (missing: ${MISSING_VARS})")
      ENDIF (NOT Taglib-Extras_FIND_QUIETLY)
    ENDIF (Taglib-Extras_FIND_REQUIRED)
  ENDIF (${_NAME_UPPER}_FOUND)

  SET(${_NAME_UPPER}_FOUND ${${_NAME_UPPER}_FOUND} PARENT_SCOPE)
endif(TAGLIB-EXTRASCONFIG_EXECUTABLE)


if(TAGLIB-EXTRAS_FOUND)
  if(NOT Taglib-Extras_FIND_QUIETLY AND TAGLIB-EXTRASCONFIG_EXECUTABLE)
    message(STATUS "Taglib-Extras found: ${TAGLIB-EXTRAS_LIBRARIES}")
  endif(NOT Taglib-Extras_FIND_QUIETLY AND TAGLIB-EXTRASCONFIG_EXECUTABLE)
else(TAGLIB-EXTRAS_FOUND)
  if(Taglib-Extras_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Taglib-Extras")
  endif(Taglib-Extras_FIND_REQUIRED)
endif(TAGLIB-EXTRAS_FOUND)

