FIND_PACKAGE(Iconv REQUIRED)
FIND_PACKAGE(Boost 1.55.0 COMPONENTS filesystem iostreams system REQUIRED)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/libendian/src" ${ICONV_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

ADD_LIBRARY(mygettext mygettext.cpp mygettext.h gettext.cpp gettext.h)

TARGET_LINK_LIBRARIES(mygettext endian ${ICONV_LIBRARY} ${Boost_LIBRARIES})

#################################################################################
