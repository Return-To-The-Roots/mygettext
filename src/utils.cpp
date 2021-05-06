// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mygettext/utils.h"
#include "mygettext/LocaleInfo.h"

namespace mygettext {
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
} // namespace mygettext
