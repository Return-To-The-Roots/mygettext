// Copyright (c) 2017 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "mygettextDefines.h" // IWYU pragma: keep
#include "utils.h"
#include "LocaleInfo.h"

std::vector<std::string> getPossibleFoldersForLangCode(const std::string& langCode)
{
    LocaleInfo info;
    info.parse(langCode);
    return getPossibleFoldersForLocale(info);
}

std::vector<std::string> getPossibleFoldersForLocale(const LocaleInfo& locale)
{
    const std::string& language = locale.getLanguage();
    const std::string& variant = locale.getVariant();
    const std::string& country = locale.getCountry();

    // Adapted from Boost.Locale
    // List of fallbacks: en_US@euro, en@euro, en_US, en.
    std::vector<std::string> paths;

    if(!variant.empty() && !country.empty())
        paths.push_back(language + "_" + country + "@" + variant);
    if(!variant.empty())
        paths.push_back(language + "@" + variant);
    if(!country.empty())
        paths.push_back(language + "_" + country);
    paths.push_back(language);
    if(language == "C" || language == "POSIX")
    {
        // Add defaults
        paths.push_back("en_GB");
        paths.push_back("en");
    }
    return paths;
}
