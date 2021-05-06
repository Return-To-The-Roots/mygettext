// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>

#ifdef __GNUC__
#    define MGT_FORMAT_ARG(idx) __attribute__((format_arg(1)))
#else
#    define MGT_FORMAT_ARG(idx)
#endif

namespace mygettext {

const char* setlocale(int category, const char* locale);

#undef gettext
const char* gettext(const char* msgid) MGT_FORMAT_ARG(1);

#undef bindtextdomain
const char* bindtextdomain(const char* domainname, const char* dirname);

#undef textdomain
const char* textdomain(const char* domainname);

#undef bind_textdomain_codeset
const char* bind_textdomain_codeset(const char* domainname, const char* codeset);

/// Return translated text for given message (or unmodified text if not found)
inline MGT_FORMAT_ARG(1) const char* _(const char* const txt)
{
    return gettext(txt);
}
inline const char* _(const std::string& txt)
{
    return gettext(txt.c_str());
}
/// Return unmodified string (used when translation is done at other place, e.g. string constants)
inline constexpr MGT_FORMAT_ARG(1) const char* gettext_noop(const char* const str)
{
    return str;
}

} // namespace mygettext

using mygettext::_;
using mygettext::gettext_noop;

#undef MGT_FORMAT_ARG
