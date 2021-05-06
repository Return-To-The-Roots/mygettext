// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>
#include <vector>

namespace mygettext {

class LocaleInfo;

std::vector<std::string> getPossibleFoldersForLangCode(const std::string& langCode);
std::vector<std::string> getPossibleFoldersForLocale(const LocaleInfo& locale);

} // namespace mygettext
