/*********************************************************************************/
/*!
@file           CommonUtil.h

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
#ifndef COMMONUTIL_H
#define COMMONUTIL_H

#include <vector>
#include <string>

class CommonUtil
{
public:
    CommonUtil();

    static void tokenize(std::string const &text, const std::string & delim,
                    std::vector<std::string> & list);

    static std::string trim(std::string const &text);
};

#endif // COMMONUTIL_H
