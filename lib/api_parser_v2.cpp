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

#include "api_parser_v2.hpp"

#define ITEM_STATE_KEY		0x00
#define ITEM_STATE_VALUE	0x10


unordered_map<string, vector<string>> api_parser_v2(char *crap){
	string sbuf_key, sbuf_value;
	vector<string> values;
	unordered_map<string, vector<string>> ret;

	int item_state = ITEM_STATE_KEY;

	char *pcrap = crap;

	while (*pcrap) {
		switch (*pcrap) {
			case ' ':
				if (item_state == ITEM_STATE_VALUE && sbuf_value.length()) {
					values.push_back(sbuf_value);
					sbuf_value.clear();
				}
				break;
			case '%':
				break;
			case '[':
				item_state = ITEM_STATE_VALUE;
				break;
			case ']':
				values.push_back(sbuf_value);
				ret[sbuf_key] = values;
				values = vector<string>();
				sbuf_key.clear();
				sbuf_value.clear();
				item_state = ITEM_STATE_KEY;
				break;
			default:
				if (item_state == ITEM_STATE_KEY) {
					sbuf_key.push_back(*pcrap);
				} else {
					sbuf_value.push_back(*pcrap);
				}
				break;
		}

		pcrap++;
	}

	return ret;
}