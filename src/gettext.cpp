// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mygettext/gettext.h"
#include "mygettext/readCatalog.h"
#include "mygettext/utils.h"
#include <boost/filesystem.hpp>
#include <clocale>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace bfs = boost::filesystem;

namespace mygettext {

GetText::GetText() : isLoaded(false)
{
    setCatalogDir("messages", "/usr/share/locale");
    setCatalog("messages");
    setLocale("C");
    setCodepage("ISO-8859-1");
}

GetText::~GetText() = default;

const char* GetText::setCatalogDir(const char* catalog, const char* directory)
{
    if(!catalog)
        return nullptr;
    if(directory)
    {
        catalogDirs_[catalog] = directory;
        if(catalog == catalog_)
            unloadCatalog();
    }
    return catalogDirs_[catalog].c_str();
}

const char* GetText::setCatalog(const char* catalog)
{
    if(catalog)
    {
        unloadCatalog();
        catalog_ = catalog;
    }
    return catalog_.c_str();
}

const char* GetText::setLocale(const char* locale)
{
#undef setlocale
    if(locale)
    {
        unloadCatalog();
        ::setlocale(LC_ALL, locale);
        localeInfo_.parse(locale);
        localeName_ = locale;
    }
    return localeName_.c_str();
}

const char* GetText::setCodepage(const char* codepage)
{
    if(codepage)
    {
        unloadCatalog();
        codepage_ = codepage;
    }

    return codepage_.c_str();
}

void GetText::unloadCatalog()
{
    entries_.clear();
    isLoaded = false;
}

const char* GetText::get(const char* text)
{
    if(!isLoaded)
        loadCatalog();

    const auto entry = entries_.find(text);
    if(entry == entries_.end())
        return text;
    else
        return entry->second.c_str();
}

std::string GetText::getCatalogFilePath() const
{
    auto it = catalogDirs_.find(catalog_);
    if(it == catalogDirs_.end())
        return "";
    std::string baseDir = it->second;

    std::vector<std::string> folders = getPossibleFoldersForLocale(localeInfo_);
    std::vector<std::string> possibleFileNames;
    possibleFileNames.reserve(folders.size() * 3);
    // Default path: dirname/locale/category/domainname.mo
    for(const std::string& folder : folders)
    {
        // NOLINTNEXTLINE(performance-inefficient-string-concatenation)
        possibleFileNames.push_back(baseDir + "/" + folder + "/LC_MESSAGES/" + catalog_ + ".mo");
    }
    // Extension: dirname/catalog-locale.mo
    for(const std::string& folder : folders)
    {
        // NOLINTNEXTLINE(performance-inefficient-string-concatenation)
        possibleFileNames.push_back(baseDir + "/" + catalog_ + "-" + folder + ".mo");
    }
    // Extension: dirname/locale.mo
    for(const std::string& folder : folders)
    {
        // NOLINTNEXTLINE(performance-inefficient-string-concatenation)
        possibleFileNames.push_back(baseDir + "/" + folder + ".mo");
    }

    for(const std::string& fileName : possibleFileNames)
    {
        if(bfs::exists(fileName))
            return fileName;
    }
    return "";
}

bool GetText::loadCatalog()
{
    if(isLoaded)
        return true;

    const std::string catalogFilepath = getCatalogFilePath();

    // Try loading only once even when this fails, as it is unlikely to succeed on another time
    isLoaded = true;

    // No catalog
    if(catalogFilepath.empty())
        return false;

    try
    {
        entries_ = readCatalog(catalogFilepath, codepage_);
    } catch(std::exception& e)
    {
        std::cerr << "Error loading catalog:" << e.what() << std::endl;
        return false;
    }
    return true;
}
} // namespace mygettext
