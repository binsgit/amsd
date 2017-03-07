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

#include <sstream>
#include "rfc1342.hpp"

string rfc1342_encode_utf8(string s){
	stringstream is(s), os;
	base64::encoder be;

	be.encode(is, os);

	string ret = "=?UTF-8?B?";
	ret += os.str();
	ret.pop_back();
	ret += "?=";

	return ret;
}