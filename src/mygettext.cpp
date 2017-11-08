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
#include "mygettext.h"
#include "gettext.h"

static GetText __gettext;

const char* mysetlocale(int /*category*/, const char* locale)
{
    return __gettext.setLocale(locale);
}

const char* mygettext(const char* msgid)
{
    return __gettext.get(msgid);
}

const char* mybindtextdomain(const char* domainname, const char* dirname)
{
    return __gettext.setCatalogDir(domainname, dirname);
}

const char* mytextdomain(const char* domainname)
{
    return __gettext.setCatalog(domainname);
}

const char* mybind_textdomain_codeset(const char* /*domainname*/, const char* codeset)
{
    return __gettext.setCodepage(codeset);
}

void splitLanguageCode(const std::string& code, std::string& lang, std::string& region, std::string& encoding)
{
    std::string::size_type pos = code.find('.');
    if(pos != std::string::npos)
    {
        lang = code.substr(0, pos);
        encoding = code.substr(pos + 1);
    } else
    {
        lang = code;
        encoding = "";
    }

    pos = lang.find('_');
    if(pos != std::string::npos)
    {
        region = lang.substr(pos + 1);
        lang = lang.substr(0, pos);
    } else
        region = "";

    // todo aliases
    if(lang == "German")
        lang = "de";
    else if(lang == "English")
        lang = "en";

    if(region == "Germany")
        region = "DE";
    else if(region == "United States")
        region = "EN";
}
