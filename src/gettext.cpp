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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "gettext.h"

#include <libendian.h> // TODO: Use EndianStream
#include <iconv.h>
#include <locale.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && defined _MSC_VER
#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

GetText::GetText() : catalog_("messages"), directory_("/usr/share/locale"), locale_("C"), codepage_(""), lastcatalog_(""), iconv_cd_(0)
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
    this->catalog_ = catalog;

    stack_.clear();
    lastcatalog_.clear();

    return catalog_.c_str();
}

const char* GetText::setLocale(const char* locale)
{
    // TODO
    this->locale_ = locale;

#undef setlocale
    const char* nl = ::setlocale(LC_ALL, locale);
    if(nl != NULL)
        this->locale_ = nl;

    stack_.clear();
    lastcatalog_.clear();

    std::string::size_type pos = this->locale_.find(".");
    if(pos != std::string::npos)
        this->locale_ = this->locale_.substr(0, pos);

    std::string lang = "", region = "";

    pos = this->locale_.find("_");
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

const char* GetText::get(const char* text)
{
    // Wort eintragen falls nicht gefunden
    if(stack_.find(text) == stack_.end())
        stack_[text] = text;

    // do nothing if locale is "C" or "POSIX"
    if(locale_ == "C" || locale_ == "POSIX")
        return stack_[text].c_str();

    // Katalog laden
    loadCatalog();

    return stack_[text].c_str();
}

// There are 2 versions of iconv: One with const char ** as input and one without const
// This template is used, if const char** version exists
template<typename T>
size_t iconv (iconv_t cd, T** inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft){
    return iconv(cd, const_cast<const T**>(inbuf), inbytesleft, outbuf, outbytesleft);
}

void GetText::loadCatalog()
{
    std::string catalogfile = directory_ + "/" + this->catalog_ + "-" + locale_ + ".mo";
    if(catalogfile == lastcatalog_)
        return;

    FILE* file = fopen(catalogfile.c_str(), "rb");
    if(!file)
    {
        catalogfile = directory_ + "/" + locale_ + ".mo";
        if(catalogfile == lastcatalog_)
            return;

        file = fopen(catalogfile.c_str(), "rb");
        if(!file)
        {
            std::string lang = "", region = "";

            std::string::size_type pos = locale_.find("_");
            if(pos != std::string::npos)
            {
                lang = locale_.substr(0, pos);
                region = locale_.substr(pos + 1);
            }

            catalogfile = directory_ + "/" + this->catalog_ + "-" + lang + ".mo";
            if(catalogfile == lastcatalog_)
                return;

            file = fopen(catalogfile.c_str(), "rb");
            if(!file)
            {
                catalogfile = directory_ + "/" + lang + ".mo";
                if(catalogfile == lastcatalog_)
                    return;

                file = fopen(catalogfile.c_str(), "rb");
            }
        }
    }

    if(!file)
    {
        lastcatalog_ = catalogfile;
        return;
    }

    unsigned int magic; // magic number = 0x950412de
    if(libendian::be_read_ui(&magic, file) != 0)
        return;
    if(magic != 0xDE120495)
        return;

    unsigned int revision; // file format revision = 0
    if(libendian::le_read_ui(&revision, file) != 0)
        return;

    unsigned int count; // number of strings
    if(libendian::le_read_ui(&count, file) != 0)
        return;

    unsigned int offsetkeytable; // offset of table with original strings
    if(libendian::le_read_ui(&offsetkeytable, file) != 0)
        return;
    unsigned int offsetvaluetable; // offset of table with translation strings
    if(libendian::le_read_ui(&offsetvaluetable, file) != 0)
        return;
    unsigned int sizehashtable; // size of hashing table
    if(libendian::le_read_ui(&sizehashtable, file) != 0)
        return;
    unsigned int offsethashtable; // offset of hashing table
    if(libendian::le_read_ui(&offsethashtable, file) != 0)
        return;

    //stack.clear();

    for(unsigned int i = 0; i < count; ++i)
    {
        unsigned int klength, vlength;
        unsigned int koffset, voffset;
        std::vector<char> key;
        std::vector<char> value;

        // Key einlesen
        fseek(file, offsetkeytable + i * 8, SEEK_SET);
        if(libendian::le_read_ui(&klength, file) != 0)
            return;
        if(libendian::le_read_ui(&koffset, file) != 0)
            return;

        key.resize(klength + 1);

        fseek(file, koffset, SEEK_SET);
        if(libendian::le_read_c(&key.front(), klength + 1, file) != (int)klength + 1)
            return;

        // Key einlesen
        fseek(file, offsetvaluetable + i * 8, SEEK_SET);
        if(libendian::le_read_ui(&vlength, file) != 0)
            return;
        if(libendian::le_read_ui(&voffset, file) != 0)
            return;

        value.resize(vlength + 1);

        fseek(file, voffset, SEEK_SET);
        if(libendian::le_read_c(&value.front(), vlength + 1, file) != (int)vlength + 1)
            return;

        if(iconv_cd_ != 0)
        {
            std::vector<char> buffer(10000);
            size_t ilength = vlength;
            size_t olength = buffer.size();

            char* input = &value.front();
            char* output = &buffer.front();
            iconv(iconv_cd_, &input, &ilength, &output, &olength);
            stack_[&key.front()] = &buffer.front();
        }
        else
            stack_[&key.front()] = &value.front();
    }

    fclose(file);

    lastcatalog_ = catalogfile;
}
