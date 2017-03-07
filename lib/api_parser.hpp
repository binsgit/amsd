/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    AMSD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMSD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMSD_API_PARSER_HPP
#define AMSD_API_PARSER_HPP

#include "../amsd.hpp"

using namespace std;


struct APIOutput {

};

extern int api_parse_crap(char *crap, size_t crap_len, Avalon_MM *mm);

#endif //AMSD_API_PARSER_HPP
