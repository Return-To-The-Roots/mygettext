// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "LocaleInfo.h"
#include <map>
#include <string>

namespace mygettext {

class GetText
{
public:
    GetText();
    ~GetText();

    const char* setCatalogDir(const char* catalog, const char* directory);
    const char* get(const char* text);

    const char* setCatalog(const char* catalog);
    const char* setLocale(const char* locale);
    const char* setCodepage(const char* codepage);

    std::string getCatalogFilePath() const;
    bool loadCatalog();

    const std::map<std::string, std::string>& getAllTranslations() const { return entries_; }

private:
    void unloadCatalog();

    std::map<std::string, std::string> catalogDirs_;
    std::string catalog_, localeName_;
    LocaleInfo localeInfo_;
    std::string codepage_;
    std::map<std::string, std::string> entries_;
    bool isLoaded;
};
} // namespace mygettext
