// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mygettext/mygettext.h"
#include "mygettext/gettext.h"

namespace mygettext {
static GetText __gettext;

const char* setlocale(int /*category*/, const char* locale)
{
    return __gettext.setLocale(locale);
}

const char* gettext(const char* msgid)
{
    return __gettext.get(msgid);
}

const char* bindtextdomain(const char* domainname, const char* dirname)
{
    return __gettext.setCatalogDir(domainname, dirname);
}

const char* textdomain(const char* domainname)
{
    return __gettext.setCatalog(domainname);
}

const char* bind_textdomain_codeset(const char* /*domainname*/, const char* codeset)
{
    return __gettext.setCodepage(codeset);
}

} // namespace mygettext
