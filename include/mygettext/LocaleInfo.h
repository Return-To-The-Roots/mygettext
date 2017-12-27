//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef LocaleInfo_h__
#define LocaleInfo_h__

#include <string>

class LocaleInfo
{
public:
    LocaleInfo();

    const std::string& getName() const { return name; }
    const std::string& getLanguage() const { return language; }
    const std::string& getCountry() const { return country; }
    const std::string& getVariant() const { return variant; }
    const std::string& getEncoding() const { return encoding; }
    bool isUtf8() const { return utf8; }

    void parse(std::string locale_name);

private:
    std::string name, language, country, variant, encoding;
    bool utf8;

    void parse_from_lang(std::string const& locale_name);
    void parse_from_country(std::string const& locale_name);
    void parse_from_encoding(std::string const& locale_name);
    void parse_from_variant(std::string const& locale_name);
};

#endif // LocaleInfo_h__
