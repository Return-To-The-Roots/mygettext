// Copyright (c) 2017 - 2020 Settlers Freaks (sf-team at siedler25.org)
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

#include "mygettext/readCatalog.h"
#include <boost/endian/arithmetic.hpp>
#include <boost/nowide/fstream.hpp>
#include <iconv.h>
#include <memory>
#include <stdexcept>
#include <vector>

// There are 2 versions of iconv: One with const char ** as input and one without const
// This template is used, if const char** version exists
template<typename T>
static size_t iconv(iconv_t cd, T** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
{
    return iconv(cd, const_cast<const T**>(inbuf), inbytesleft, outbuf, outbytesleft);
}

namespace mygettext {

struct IconvCloser
{
    using pointer = iconv_t;
    void operator()(iconv_t p)
    {
        if(p)
            iconv_close(p);
    }
};
using IconvHandle = std::unique_ptr<iconv_t, IconvCloser>;

template<typename T>
std::istream& read(std::istream& s, T& value)
{
    return s.read(reinterpret_cast<char*>(&value), sizeof(value));
}
template<typename T>
std::istream& read(std::istream& s, T* value) = delete; // Don't read to pointers

std::map<std::string, std::string> readCatalog(const std::string& catalogFilepath, const std::string& targetCodepage)
{
    boost::nowide::ifstream file(catalogFilepath, std::ios::binary);

    if(!file)
        throw std::runtime_error("Could not open file");

    boost::endian::little_uint32_t magic; // magic number = 0x950412de
    if(!read(file, magic) || magic != 0x950412de)
    {
        if(file && magic == 0xde120495)
            throw std::runtime_error("File is in big endian format. Can't read that");
        throw std::runtime_error("Not a valid mo file");
    }

    struct Header
    {
        boost::endian::little_uint32_t revision;         // file format revision = 0
        boost::endian::little_uint32_t count;            // number of strings
        boost::endian::little_uint32_t offsetKeyTable;   // offset of table with original strings
        boost::endian::little_uint32_t offsetValueTable; // offset of table with translation strings
        boost::endian::little_uint32_t sizeHashTable;    // size of hashing table
        boost::endian::little_uint32_t offsetHashTable;  // offset of hashing table
    };
    static_assert(sizeof(Header) == 24, "Invalid padding");

    Header header;
    if(!read(file, header))
        throw std::runtime_error("Failed to read header");

    struct CatalogEntryDescriptor
    {
        boost::endian::little_uint32_t keyLen, keyOffset;
        boost::endian::little_uint32_t valueLen, valueOffset;
        std::string key;
    };

    // Read the descriptors first as they are at contiguous positions in the file
    std::vector<CatalogEntryDescriptor> entryDescriptors(header.count);

    file.seekg(static_cast<std::streampos>(header.offsetKeyTable));
    for(auto& entryDescriptor : entryDescriptors)
    {
        read(file, entryDescriptor.keyLen);
        read(file, entryDescriptor.keyOffset);
    }
    file.seekg(static_cast<std::streampos>(header.offsetValueTable));
    for(auto& entryDescriptor : entryDescriptors)
    {
        read(file, entryDescriptor.valueLen);
        read(file, entryDescriptor.valueOffset);
    }
    if(!file)
        throw std::runtime_error("Failed to read offsets");

    // Keys and values are most probably contiguous. So read them at once
    for(auto& entryDescriptor : entryDescriptors)
    {
        file.seekg(static_cast<std::streampos>(entryDescriptor.keyOffset));
        entryDescriptor.key.resize(entryDescriptor.keyLen);
        if(!file.read(&entryDescriptor.key[0], entryDescriptor.keyLen))
            throw std::runtime_error("Failed to read key");
    }

    std::map<std::string, std::string> entries;
    if(targetCodepage != "UTF-8")
    {
        IconvHandle iconvHandle(iconv_open(targetCodepage.c_str(), "UTF-8"));
        // Declare outside of loop to avoid allocations
        std::vector<char> readBuffer, iconvBuffer;
        for(const auto& entryDescriptor : entryDescriptors)
        {
            file.seekg(static_cast<std::streampos>(entryDescriptor.valueOffset));

            readBuffer.resize(entryDescriptor.valueLen);
            if(!file.read(readBuffer.data(), entryDescriptor.valueLen))
                throw std::runtime_error("Failed to read entry");

            iconvBuffer.resize(entryDescriptor.valueLen
                               * 6); // UTF needs at most 6 times the size per char, so this should be enough
            size_t iLength = entryDescriptor.valueLen; // Don't count terminating zero
            size_t oLength = iconvBuffer.size();

            char* input = readBuffer.data();
            char* output = iconvBuffer.data();
            const size_t result = iconv(iconvHandle.get(), &input, &iLength, &output, &oLength);
            if(result == static_cast<size_t>(-1))
                throw std::runtime_error("Conversion failed");
            entries[entryDescriptor.key].assign(iconvBuffer.data(), output);
        }
    } else
    {
        for(const auto& entryDescriptor : entryDescriptors)
        {
            file.seekg(static_cast<std::streampos>(entryDescriptor.valueOffset));

            std::string& entry = entries[entryDescriptor.key];
            entry.resize(entryDescriptor.valueLen);
            if(!file.read(&entry[0], entryDescriptor.valueLen))
                throw std::runtime_error("Failed to read entry");
        }
    }
    return entries;
}

} // namespace mygettext
