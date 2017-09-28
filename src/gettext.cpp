// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
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

#include "libendian/EndianIStreamAdapter.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iconv.h>
#include <clocale>
#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <exception>
#include <utility>

namespace bfs = boost::filesystem;

GetText::GetText() : catalog_("messages"), directory_("/usr/share/locale"), locale_("C"), isLoaded(false), iconv_cd_(0)
{
    setCodepage("ISO-8859-1");
}

GetText::~GetText()
{
    iconv_close(iconv_cd_);
}

const char* GetText::init(const char* catalog, const char* directory)
{
    this->directory_ = directory;

    return setCatalog(catalog);
}

const char* GetText::setCatalog(const char* catalog)
{
    unloadCatalog();
    this->catalog_ = catalog;
    return catalog_.c_str();
}

const char* GetText::setLocale(const char* locale)
{
    unloadCatalog();
    // TODO
    this->locale_ = locale;

#undef setlocale
    const char* nl = ::setlocale(LC_ALL, locale);
    if(nl != NULL)
        this->locale_ = nl;

    std::string::size_type pos = this->locale_.find('.');
    if(pos != std::string::npos)
        this->locale_ = this->locale_.substr(0, pos);

    std::string lang = "", region = "";

    pos = this->locale_.find('_');
    if(pos != std::string::npos)
    {
        lang = this->locale_.substr(0, pos);
        region = this->locale_.substr(pos + 1);
    }

    // todo aliases
    if(lang == "German")
        lang = "de";
    else if(lang == "English")
        lang = "en";

    if(region == "Germany")
        region = "DE";
    else if(region == "United States")
        region = "EN";

    this->locale_ = lang;
    if(region.length())
        this->locale_ += ( "_" + region);

    return this->locale_.c_str();
}

const char* GetText::setCodepage(const char* codepage)
{
    this->codepage_ = codepage;

    if(iconv_cd_ != 0)
        iconv_close(iconv_cd_);
    iconv_cd_ = iconv_open(codepage, "UTF-8");

    return this->codepage_.c_str();
}

void GetText::unloadCatalog()
{
    entries_.clear();
    curCatalogFilePath_.clear();
    isLoaded = false;
}

const char* GetText::get(const char* text)
{
    // Load catalog only if locale is not "C" or "POSIX"
    if(!isLoaded && locale_ != "C" && locale_ != "POSIX")
        loadCatalog();

    std::map<std::string, std::string>::const_iterator entry = entries_.find(text);
    if(entry == entries_.end())
    {
        // Add if not found
        entries_[text] = text;
        return text;
    }else
        return entry->second.c_str();
}

// There are 2 versions of iconv: One with const char ** as input and one without const
// This template is used, if const char** version exists
template<typename T>
size_t iconv (iconv_t cd, T** inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft){
    return iconv(cd, const_cast<const T**>(inbuf), inbytesleft, outbuf, outbytesleft);
}

std::string GetText::getCatalogFilePath()
{
    if(!curCatalogFilePath_.empty())
        return curCatalogFilePath_;

    std::vector<std::string> possibleFileNames;
    possibleFileNames.push_back(directory_ + "/" + this->catalog_ + "-" + locale_ + ".mo");
    possibleFileNames.push_back(directory_ + "/" + locale_ + ".mo");
    std::string::size_type pos = locale_.find('_');
    if(pos != std::string::npos)
    {
        std::string lang = locale_.substr(0, pos);
        possibleFileNames.push_back(directory_ + "/" + this->catalog_ + "-" + lang + ".mo");
        possibleFileNames.push_back(directory_ + "/" + lang + ".mo");
    }

    for(std::vector<std::string>::const_iterator it = possibleFileNames.begin(); it != possibleFileNames.end(); ++it)
    {
        if(bfs::exists(*it))
        {
            curCatalogFilePath_ = *it;
            break;
        }
    }
    return curCatalogFilePath_;
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

    try
    {
        libendian::EndianIStreamAdapter<false, bfs::ifstream> file(catalogfile, std::ios::binary);

        if(!file)
            return;

        uint32_t magic; // magic number = 0x950412de
        file >> magic;
        if(magic != 0x950412de)
            return;

        uint32_t revision; // file format revision = 0
        uint32_t count; // number of strings
        uint32_t offsetKeyTable; // offset of table with original strings
        uint32_t offsetValueTable; // offset of table with translation strings
        uint32_t sizeHashTable; // size of hashing table
        uint32_t offsetHashTable; // offset of hashing table

        file >> revision >> count >> offsetKeyTable >> offsetValueTable >> sizeHashTable >> offsetHashTable;

        //entries_.clear();

        //Read the descriptors first as they are at contigous positions in the file
        std::vector<CatalogEntryDescriptor> entryDescriptors(count);

        file.setPosition(offsetKeyTable);
        for(std::vector<CatalogEntryDescriptor>::iterator it = entryDescriptors.begin(); it != entryDescriptors.end(); ++it)
        {
            file >> it->keyLen >> it->keyOffset;
            ++it->keyLen; // Terminating zero
        }
        file.setPosition(offsetValueTable);
        for(std::vector<CatalogEntryDescriptor>::iterator it = entryDescriptors.begin(); it != entryDescriptors.end(); ++it)
        {
            file >> it->valueLen >> it->valueOffset;
            ++it->valueLen; // Terminating zero
        }

        // Declare those outside of the loop to avoid reallocating the memory at every iteration
        std::vector<char> readBuffer;
        std::vector<char> iconvBuffer;

        // Keys and values are most probably contigous. So read them at once
        for(std::vector<CatalogEntryDescriptor>::iterator it = entryDescriptors.begin(); it != entryDescriptors.end(); ++it)
        {
            readBuffer.resize(it->keyLen);

            file.setPosition(it->keyOffset);
            file.read(&readBuffer.front(), it->keyLen);
            it->key = &readBuffer.front();
        }

        for(std::vector<CatalogEntryDescriptor>::iterator it = entryDescriptors.begin(); it != entryDescriptors.end(); ++it)
        {
            readBuffer.resize(it->valueLen);

            file.setPosition(it->valueOffset);
            file.read(&readBuffer.front(), it->valueLen);

            if(iconv_cd_ != 0)
            {
                iconvBuffer.resize(it->valueLen * 6); // UTF needs at most 6 times the size per char, so this should be enough
                size_t ilength = it->valueLen - 1; // Don't count terminating zero
                size_t olength = iconvBuffer.size();

                char* input = &readBuffer.front();
                char* output = &iconvBuffer.front();
                iconv(iconv_cd_, &input, &ilength, &output, &olength);
                if(static_cast<size_t>(output - &iconvBuffer.front()) >= iconvBuffer.size())
                    throw std::runtime_error("Buffer overflow detected!"); // Should never happen due to the size given
                *output = 0; // Terminator
                entries_[it->key] = &iconvBuffer.front();
            }
            else
                entries_[it->key] = &readBuffer.front();
        }
    }catch(std::exception&)
    {} //-V565
}
