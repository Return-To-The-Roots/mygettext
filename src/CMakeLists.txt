FIND_PACKAGE(Iconv REQUIRED)
FIND_PACKAGE(Boost 1.55.0 COMPONENTS filesystem iostreams system REQUIRED)

get_filename_component(parentPath ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set(includePath ${parentPath}/include)

include_directories(${includePath}/mygettext)
INCLUDE_DIRECTORIES(SYSTEM ${ICONV_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
set(SRC_FILES mygettext.cpp ${includePath}/mygettext/mygettext.h gettext.cpp gettext.h)
ADD_LIBRARY(mygettext STATIC ${SRC_FILES})

target_include_directories(mygettext PUBLIC ${includePath})
TARGET_LINK_LIBRARIES(mygettext endian ${ICONV_LIBRARY} ${Boost_LIBRARIES})

if(ClangFormat_FOUND)
    add_clangFormat_files(${SRC_FILES})
endif()
