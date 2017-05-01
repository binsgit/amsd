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

static shared_timed_mutex Lock;
static long Modules = 0, Controllers = 0, MHS_T = 0;
static double MHS = 0;

void Services::GlimpseDataUpdater(void *userp) {
	fprintf(stderr, "amsd: Services::GlimpseDataUpdater: Updating glimpse data in background...\n");

	DataProcessing::Report rpt("", 0);

	Lock.lock();

	Modules = rpt.Farm0.Modules;
	Controllers = rpt.Farm0.Controllers.size();
	MHS = (double)rpt.Farm0.MHS;
	MHS_T = (long)rpt.Farm0.Modules*1000*7300;

	Lock.unlock();

	fprintf(stderr, "amsd: Services::GlimpseDataUpdater: Done.\n");
}

int AMSD::Operations::glimpse(json_t *in_data, json_t *&out_data){
	DataProcessing::Report rpt("", 0);

	json_object_set_new(out_data, "mods", json_integer((long)rpt.Farm0.Modules));
	json_object_set_new(out_data, "ctls", json_integer((long)rpt.Farm0.Controllers.size()));
	json_object_set_new(out_data, "mhs", json_real((double)rpt.Farm0.MHS));
	json_object_set_new(out_data, "mhs_t", json_integer((long)rpt.Farm0.Modules*1000*7300));

	return 0;
}