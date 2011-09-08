// $Id: mygettext.cpp 7521 2011-09-08 20:45:55Z FloSoft $
//
// Copyright (c) 2005 - 2011 Settlers Freaks (sf-team at siedler25.org)
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
#include "mygettext.h"

#include "gettext.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

static GetText __gettext;

const char *mysetlocale(int category, const char *locale)
{
	return __gettext.setLocale(locale);
}

const char *mygettext(const char *msgid)
{
	return __gettext.get(msgid);
}

const char *mybindtextdomain(const char *domainname, const char *dirname)
{
	return __gettext.init(domainname, dirname);
}

const char *mytextdomain(const char *domainname)
{
	return __gettext.setCatalog(domainname);
}

const char *mybind_textdomain_codeset(const char *domainname, const char *codeset)
{
	return __gettext.setCodepage(codeset);
}


