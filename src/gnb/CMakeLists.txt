file(GLOB_RECURSE HDR_FILES *.hpp)
file(GLOB_RECURSE SRC_FILES *.cpp)

FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
  SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)
add_library(gnb ${HDR_FILES} ${SRC_FILES})
include_directories(${CURL_INCLUDE_DIR})
target_compile_options(gnb PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter -lcurl)

target_link_libraries(gnb asn-ngap)
target_link_libraries(gnb asn-rrc)
target_link_libraries(gnb common-lib)

