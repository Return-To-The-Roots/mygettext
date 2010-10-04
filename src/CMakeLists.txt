#################################################################################
### $Id: CMakeLists.txt 6460 2010-05-31 11:42:38Z FloSoft $
#################################################################################

INCLUDE(FindIconv)

IF (NOT Iconv_FOUND)
	MESSAGE(FATAL_ERROR "libiconv not found")
ENDIF (NOT Iconv_FOUND)

INCLUDE_DIRECTORIES(../libendian/src)
LINK_DIRECTORIES(../libendian/src)

ADD_LIBRARY(mygettext mygettext.cpp mygettext.h gettext.cpp gettext.h)

TARGET_LINK_LIBRARIES(mygettext endian ${ICONV_LIBRARY})

#################################################################################
