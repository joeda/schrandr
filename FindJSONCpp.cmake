find_path(JSONCPP_INCLUDE_DIR NAMES json/json.h
          DOC "The JsonCpp include directory"
)

find_library(JSONCPP_LIBRARY NAMES jsoncpp
          DOC "The JsonCpp library"
)

if(JSONCPP_INCLUDE_DIR AND EXISTS "${JSONCPP_INCLUDE_DIR}/json/version.h")
  file(STRINGS "${JSONCPP_INCLUDE_DIR}/json/version.h" jsoncpp_version_str REGEX "^#define[\t ]+JSONCPP_VERSION_STR[\t ]+\".*\"")

  string(REGEX REPLACE "^.*JSONCPP_VERSION_STR[\t ]+\"([^\"]*)\".*$" "\\1" JSONCPP_VERSION_STRING "${jsoncpp_version_str}")
  unset(jsoncpp_version_str)
endif()

# handle the QUIETLY and REQUIRED arguments and set ALSA_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JSONCPP
                                  REQUIRED_VARS JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR
                                  VERSION_VAR JSONCPP_VERSION_STRING)

if(JSONCPP_FOUND)
  set( JSONCPP_LIBRARIES ${JSONCPP_LIBRARY} )
  set( JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR} )
endif()

mark_as_advanced(JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY) 
