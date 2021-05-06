// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <map>
#include <string>

namespace mygettext {

std::map<std::string, std::string> readCatalog(const std::string& catalogFilepath, const std::string& targetCodepage);

} // namespace mygettext
