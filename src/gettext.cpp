// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
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
#include "gettext.h"
#include "mygettext.h"
#include "utils.h"
#include "libendian/EndianIStreamAdapter.h"
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <clocale>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bfs = boost::filesystem;

GetText::GetText() : isLoaded(false), iconv_cd_(nullptr)
{
    setCatalogDir("messages", "/usr/share/locale");
    setCatalog("messages");
    setLocale("C");
    setCodepage("ISO-8859-1");
}

GetText::~GetText()
{
    if(iconv_cd_)
        iconv_close(iconv_cd_);
}

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
        localeName_ = localeInfo_.getName();
    }
    return localeName_.c_str();
}

const char* GetText::setCodepage(const char* codepage)
{
    if(codepage)
    {
        unloadCatalog();
        codepage_ = codepage;

        if(iconv_cd_)
        {
            iconv_close(iconv_cd_);
            iconv_cd_ = nullptr;
        }
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

// There are 2 versions of iconv: One with const char ** as input and one without const
// This template is used, if const char** version exists
template<typename T>
size_t iconv(iconv_t cd, T** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
{
    return iconv(cd, const_cast<const T**>(inbuf), inbytesleft, outbuf, outbytesleft);
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

struct CatalogEntryDescriptor
{
    uint32_t keyLen, keyOffset;
    uint32_t valueLen, valueOffset;
    std::string key;
};

void GetText::loadCatalog()
{
    if(isLoaded)
        return;

    const std::string catalogfile = getCatalogFilePath();

    // Try loading only once even when this fails, as it is unlikely to succeed on another time
    isLoaded = true;

    // No catalog
    if(catalogfile.empty())
        return;

    try
    {
        libendian::EndianIStreamAdapter<false, boost::nowide::ifstream> file(catalogfile, std::ios::binary);

        if(!file)
            return;

        uint32_t magic; // magic number = 0x950412de
        file >> magic;
        if(magic != 0x950412de)
            return;

        uint32_t revision;         // file format revision = 0
        uint32_t count;            // number of strings
        uint32_t offsetKeyTable;   // offset of table with original strings
        uint32_t offsetValueTable; // offset of table with translation strings
        uint32_t sizeHashTable;    // size of hashing table
        uint32_t offsetHashTable;  // offset of hashing table

        file >> revision >> count >> offsetKeyTable >> offsetValueTable >> sizeHashTable >> offsetHashTable;

        // entries_.clear();

        // Read the descriptors first as they are at contigous positions in the file
        std::vector<CatalogEntryDescriptor> entryDescriptors(count);

        file.setPosition(offsetKeyTable);
        for(auto& entryDescriptor : entryDescriptors)
        {
            file >> entryDescriptor.keyLen >> entryDescriptor.keyOffset;
            ++entryDescriptor.keyLen; // Terminating zero
        }
        file.setPosition(offsetValueTable);
        for(auto& entryDescriptor : entryDescriptors)
        {
            file >> entryDescriptor.valueLen >> entryDescriptor.valueOffset;
            ++entryDescriptor.valueLen; // Terminating zero
        }

        // Declare those outside of the loop to avoid reallocating the memory at every iteration
        std::vector<char> readBuffer;
        std::vector<char> iconvBuffer;

        // Keys and values are most probably contigous. So read them at once
        for(auto& entryDescriptor : entryDescriptors)
        {
            readBuffer.resize(entryDescriptor.keyLen);

            file.setPosition(entryDescriptor.keyOffset);
            file.read(&readBuffer.front(), entryDescriptor.keyLen);
            entryDescriptor.key = &readBuffer.front();
        }

        if(!iconv_cd_ && codepage_ != "UTF-8")
            iconv_cd_ = iconv_open(this->codepage_.c_str(), "UTF-8");

        for(auto& entryDescriptor : entryDescriptors)
        {
            readBuffer.resize(entryDescriptor.valueLen);

            file.setPosition(entryDescriptor.valueOffset);
            file.read(&readBuffer.front(), entryDescriptor.valueLen);

            if(iconv_cd_ != nullptr)
            {
                iconvBuffer.resize(entryDescriptor.valueLen * 6); // UTF needs at most 6 times the size per char, so this should be enough
                size_t ilength = entryDescriptor.valueLen - 1;    // Don't count terminating zero
                size_t olength = iconvBuffer.size();

                char* input = &readBuffer.front();
                char* output = &iconvBuffer.front();
                iconv(iconv_cd_, &input, &ilength, &output, &olength);
                if(static_cast<size_t>(output - &iconvBuffer.front()) >= iconvBuffer.size())
                    throw std::runtime_error("Buffer overflow detected!"); // Should never happen due to the size given
                *output = 0;                                               // Terminator
                entries_[entryDescriptor.key] = &iconvBuffer.front();
            } else
                entries_[entryDescriptor.key] = &readBuffer.front();
        }
    } catch(std::exception& e)
    {
        std::cerr << "Error loading catalog:" << e.what() << std::endl;
    } //-V565
}
