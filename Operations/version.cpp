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

#include "Operations.hpp"


int AMSD::Operations::version(json_t *in_data, json_t *&out_data) {
	json_t *j_version = json_object();
	json_t *j_amsd_version = json_real(AMSD_VERSION);

	json_object_set_new(j_version, "amsd", j_amsd_version);
	json_object_set_new(out_data, "version", j_version);

	return 0;
}

