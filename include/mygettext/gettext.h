// Copyright (c) 2005 - 2020 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

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
