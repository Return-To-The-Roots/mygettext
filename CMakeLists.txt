# Copyright (C) 2005 - 2021 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

find_package(Iconv REQUIRED)
find_package(Boost 1.55 REQUIRED COMPONENTS filesystem)

get_filename_component(parentPath ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set(includePath ${parentPath}/include)

set(SRC_FILES
	src/mygettext.cpp include/mygettext/mygettext.h
	src/gettext.cpp include/mygettext/gettext.h
	src/readCatalog.cpp include/mygettext/readCatalog.h
	src/utils.cpp include/mygettext/utils.h
	src/LocaleInfo.cpp include/mygettext/LocaleInfo.h
	)
add_library(mygettext STATIC ${SRC_FILES})

target_include_directories(mygettext PUBLIC include)
target_include_directories(mygettext SYSTEM PRIVATE ${ICONV_INCLUDE_DIR})
target_link_libraries(mygettext PRIVATE ${ICONV_LIBRARY} Boost::filesystem Boost::nowide PUBLIC Boost::disable_autolinking)
target_compile_features(mygettext PUBLIC cxx_std_14)
set_target_properties(mygettext PROPERTIES CXX_EXTENSIONS OFF)

include(EnableWarnings)
enable_warnings(mygettext)

if(ClangFormat_FOUND)
    add_clangFormat_files(${SRC_FILES})
endif()

include(RttrTestingCfg)
if(BUILD_TESTING)
    add_subdirectory(tests)
    if(ClangFormat_FOUND)
        add_ClangFormat_folder(tests TRUE)
    endif()
endif()
