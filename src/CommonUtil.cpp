/*********************************************************************************/
/*!
@file           CommonUtil.cpp

@brief          Common Utility

@author         Vy Ho

    Copyright (c)   2022, Vy Ho, all rights reserved

    This file is part of the PianoBooster application

    PianoBooster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PianoBooster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PianoBooster.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "CommonUtil.h"

#include <iostream>

CommonUtil::CommonUtil()
{

}


void CommonUtil::tokenize(std::string const &text, const std::string & delim,
            std::vector<std::string> & list)
{
    size_t prevPos = 0;
    size_t pos;
    std::string token;
    while ((pos = text.find(delim, prevPos)) != std::string::npos)
    {
        token = text.substr(prevPos, pos);
        if ( !token.empty()) {
            list.push_back(token);
        }
        prevPos = pos + delim.length();
    }
    token = text.substr(prevPos, text.length());
    if ( !token.empty()) {
        list.push_back(token);
    }
}

std::string CommonUtil::trim(std::string const &text) {
    size_t start;
    for ( start = 0; start < text.length(); start++) {
        if ( !std::isspace(text.at(start)) ) {
            break;
        }
    }
    size_t end = text.length() - 1;
    for ( ; end > start; end--) {
        if ( !std::isspace(text.at(end)) ) {
            break;
        }
    }

    return text.substr(start, end + 1);
}

uint64_t timeSinceEpochMillisec2() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
