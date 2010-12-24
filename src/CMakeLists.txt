#################################################################################
### $Id: CMakeLists.txt 6942 2010-12-24 07:50:41Z FloSoft $
#################################################################################

INCLUDE(FindIconv)

IF (NOT Iconv_FOUND)
	MESSAGE(FATAL_ERROR "libiconv not found")
ENDIF (NOT Iconv_FOUND)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/libendian/src")

ADD_LIBRARY(mygettext mygettext.cpp mygettext.h gettext.cpp gettext.h)

TARGET_LINK_LIBRARIES(mygettext endian ${ICONV_LIBRARY})

#################################################################################
