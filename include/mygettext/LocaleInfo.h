// Copyright (c) 2009 - 2011 Artyom Beilis (Tonkikh)
// Copyright (C) 2019 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <string>

namespace mygettext {
class LocaleInfo
{
public:
    LocaleInfo();
    explicit LocaleInfo(const std::string& locale_name);

    const std::string& getName() const { return name; }
    const std::string& getLanguage() const { return language; }
    const std::string& getCountry() const { return country; }
    const std::string& getVariant() const { return variant; }
    const std::string& getEncoding() const { return encoding; }
    bool isUtf8() const { return utf8; }

    void parse(const std::string& locale_name);

private:
    std::string name, language, country, variant, encoding;
    bool utf8;

    void parse_from_lang(std::string const& locale_name);
    void parse_from_country(std::string const& locale_name);
    void parse_from_encoding(std::string const& locale_name);
    void parse_from_variant(std::string const& locale_name);
};
} // namespace mygettext
