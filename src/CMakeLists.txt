FIND_PACKAGE(Iconv REQUIRED)
FIND_PACKAGE(Boost 1.55.0 COMPONENTS filesystem iostreams system REQUIRED)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${ICONV_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

ADD_LIBRARY(mygettext SHARED mygettext.cpp mygettext.h gettext.cpp gettext.h)

TARGET_LINK_LIBRARIES(mygettext endian ${ICONV_LIBRARY} ${Boost_LIBRARIES})

INSTALL(TARGETS mygettext RUNTIME DESTINATION ${RTTR_LIBDIR})

#################################################################################
