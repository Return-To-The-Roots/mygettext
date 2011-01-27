// $Id: gettext.cpp 7005 2011-01-27 22:01:07Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
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

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

GetText::GetText() : catalog("messages"), directory("/usr/share/locale"), locale("C"), codepage(""), lastcatalog(""), iconv_cd(0)
{
	setCodepage("ISO-8859-1");
}

GetText::~GetText()
{
	iconv_close(iconv_cd);
}

const char *GetText::init(const char *catalog, const char *directory)
{
	this->directory = directory;

	return setCatalog(catalog);
}

const char *GetText::setCatalog(const char *catalog)
{
	std::string temp = this->catalog;
	this->catalog = catalog;

	stack.clear();
	lastcatalog = "";

	return temp.c_str();
}

const char *GetText::setLocale(const char *locale)
{
	// TODO
	this->locale = locale;

#undef setlocale
	const char *nl = ::setlocale(LC_ALL, locale);
	if(nl != NULL)
		this->locale = nl;

	stack.clear();
	lastcatalog = "";

	std::string::size_type pos = this->locale.find(".");
	if(pos != std::string::npos)
		this->locale = this->locale.substr(0, pos);

	std::string lang = "", region = "";

	pos = this->locale.find("_");
	if(pos != std::string::npos)
	{
		lang = this->locale.substr(0, pos);
		region = this->locale.substr(pos+1);
	}

	// todo aliases
	if(lang == "German")
		lang = "de";
	else if(lang == "English")
		lang = "en";

	if(region == "Germany")
		region = "DE";
	else if(lang == "United States")
		region = "EN";

	this->locale = lang;
	if(region.length())
		this->locale += ( "_" + region);

	return this->locale.c_str();
}

const char *GetText::setCodepage(const char *codepage)
{
	this->codepage = codepage;

	if(iconv_cd != 0)
		iconv_close(iconv_cd);
	iconv_cd = iconv_open(codepage, "UTF-8");

	return this->codepage.c_str();
}

const char *GetText::get(const char *text)
{
	// Wort eintragen falls nicht gefunden
	if(stack.find(text) == stack.end())
		stack[text] = text;
	
	// do nothing if locale is "C" or "POSIX"
	if(locale == "C" || locale == "POSIX")
		return stack[text].c_str();

	// Katalog laden
	loadCatalog();

	return stack[text].c_str();
}

void GetText::loadCatalog()
{
	std::string catalogfile = directory + "/" + locale + ".mo";

	if(catalogfile == lastcatalog)
		return;

	FILE *file = fopen(catalogfile.c_str(), "rb");
	if(!file)
	{
		std::string lang = "", region = "";

		std::string::size_type pos = locale.find("_");
		if(pos != std::string::npos)
		{
			lang = locale.substr(0, pos);
			region = locale.substr(pos+1);
		}

		catalogfile = directory + "/" + lang + ".mo";

		if(catalogfile == lastcatalog)
			return;

		file = fopen(catalogfile.c_str(), "rb");
	}

	if(!file)
	{
		lastcatalog = catalogfile;
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
		char *key = NULL, *value = NULL;

		// Key einlesen
		fseek(file, offsetkeytable + i * 8, SEEK_SET);
		if(libendian::le_read_ui(&klength, file) != 0)
			return;
		if(libendian::le_read_ui(&koffset, file) != 0)
			return;

		key = new char[klength+1];

		fseek(file, koffset, SEEK_SET);
		if(libendian::le_read_c(key, klength+1, file) != (int)klength+1)
			return;

		// Key einlesen
		fseek(file, offsetvaluetable + i * 8, SEEK_SET);
		if(libendian::le_read_ui(&vlength, file) != 0)
			return;
		if(libendian::le_read_ui(&voffset, file) != 0)
			return;

		value = new char[vlength+1];

		fseek(file, voffset, SEEK_SET);
		if(libendian::le_read_c(value, vlength+1, file) != (int)vlength+1)
			return;

		char buffer[10000];
		std::memset(buffer, 0, 10000);
		size_t ilength = vlength;
		size_t olength = 10000;

		if(iconv_cd != 0)
		{
#if defined _WIN32 && !defined __CYGWIN__
			const
#endif
			char *input = value;
			char *output = buffer;
			iconv(iconv_cd, &input, &ilength, &output, &olength);
			stack[key] = buffer;
		}
		else
			stack[key] = value;

		delete[] key;
		delete[] value;
	}

	fclose(file);

	lastcatalog = catalogfile;
}

