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

#include "../amsd.hpp"

static uint8_t *RuntimeData_Memory = NULL;

static uint8_t *RDM_OFFSET_TimeStamp_CurrentDataCollection;
static uint8_t *RDM_OFFSET_TimeStamp_LastDataCollection;

int AMSD::RuntimeData::Init() {
	string shm_path = path_runtime + "RuntimeData";
	RuntimeData_Memory = reimu_shm_open(shm_path, 32768);
	if (!RuntimeData_Memory)
		return -2;

	RDM_OFFSET_TimeStamp_CurrentDataCollection = RuntimeData_Memory + 128;
	RDM_OFFSET_TimeStamp_LastDataCollection = RDM_OFFSET_TimeStamp_CurrentDataCollection + 8;

	return 0;
}

time_t AMSD::RuntimeData::TimeStamp::CurrentDataCollection() {
	time_t *pRD = (time_t *)RDM_OFFSET_TimeStamp_CurrentDataCollection;
	return *pRD;
}

void AMSD::RuntimeData::TimeStamp::CurrentDataCollection(time_t t) {
	time_t *pRD = (time_t *)RDM_OFFSET_TimeStamp_CurrentDataCollection;
	*pRD = t;
}

time_t AMSD::RuntimeData::TimeStamp::LastDataCollection() {
	time_t *pRD = (time_t *)RDM_OFFSET_TimeStamp_LastDataCollection;
	return *pRD;
}


void AMSD::RuntimeData::TimeStamp::LastDataCollection(time_t t) {
	time_t *pRD = (time_t *)RDM_OFFSET_TimeStamp_LastDataCollection;
	*pRD = t;
}

