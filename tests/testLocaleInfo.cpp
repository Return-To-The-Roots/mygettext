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

#include "mygettext/LocaleInfo.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(Parse)
{
    auto localeStrings = {"de_de.iso8859-1@var", "DE_DE.ISO8859-1@var", "DE-DE.ISO8859-1@var"};
    for(std::string str : localeStrings)
    {
        mygettext::LocaleInfo info;
        info.parse(str);
        BOOST_TEST(info.getName() == str);
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "DE");
        BOOST_TEST(info.getEncoding() == "iso8859-1");
        BOOST_TEST(!info.isUtf8());
        BOOST_TEST(info.getVariant() == "var");
    }
}

BOOST_AUTO_TEST_CASE(ParseWithDefaults)
{
    {
        mygettext::LocaleInfo info;
        info.parse("de");
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "");
        BOOST_TEST(info.getEncoding() == "us-ascii");
        BOOST_TEST(!info.isUtf8());
        BOOST_TEST(info.getVariant() == "");
    }
    {
        mygettext::LocaleInfo info;
        info.parse("de_DE");
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "DE");
        BOOST_TEST(info.getEncoding() == "us-ascii");
        BOOST_TEST(!info.isUtf8());
        BOOST_TEST(info.getVariant() == "");
    }
    {
        mygettext::LocaleInfo info;
        info.parse("de.UTF-8");
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "");
        BOOST_TEST(info.getEncoding() == "utf-8");
        BOOST_TEST(info.isUtf8());
        BOOST_TEST(info.getVariant() == "");
    }
    {
        mygettext::LocaleInfo info;
        info.parse("de@var");
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "");
        BOOST_TEST(info.getEncoding() == "us-ascii");
        BOOST_TEST(!info.isUtf8());
        BOOST_TEST(info.getVariant() == "var");
    }
    {
        mygettext::LocaleInfo info;
        info.parse("de_DE.UTF-8");
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "DE");
        BOOST_TEST(info.getEncoding() == "utf-8");
        BOOST_TEST(info.isUtf8());
        BOOST_TEST(info.getVariant() == "");
    }
    {
        mygettext::LocaleInfo info;
        info.parse("de_DE@var");
        BOOST_TEST(info.getLanguage() == "de");
        BOOST_TEST(info.getCountry() == "DE");
        BOOST_TEST(info.getEncoding() == "us-ascii");
        BOOST_TEST(!info.isUtf8());
        BOOST_TEST(info.getVariant() == "var");
    }
}
