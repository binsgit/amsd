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

#include "dataprocessing.hpp"


int DataProcessing::Collector::CgMinerAPI::RequestRaw(Reimu::IPEndPoint ep, string in_data, string &out_data) {
	int fd, ret = 0;
	bool IPv6 = false;
	ssize_t rrc = 0;
	uint8_t buf_in[512] = {0};
	uint8_t remote_addr[sizeof(struct sockaddr_in6)];

	try {
		fd = ep.Connect();
	} catch (Reimu::Exception e) {
		goto badending;
	}

	in_data.push_back('\n');
	send(fd, in_data.c_str(), in_data.length(), MSG_WAITALL);

	while (1) {
		if (rrc > 1)
			if (buf_in[rrc-1] == '\n')
				break;

		rrc = read(fd, buf_in, 512);

		fprintf(stderr, "amsd: cgminer_api_request_raw: read(%d, %p, %d) = %zd\n", fd, (void *)buf_in, 512, rrc);

		if (rrc < 0)
			goto badending;
		else if (rrc == 0)
			break;
		else
			out_data.insert(out_data.end(), buf_in, buf_in+rrc);
	}

	ending:
	ep.Close();
	return ret;

	badending:
	ret = -1;
	goto ending;
}

const char *DataProcessing::Collector::CgMinerAPI::ToString(CgMinerAPI::APIType v) {
	switch (v) {
		case Summary:
			return "summary";
		case EStats:
			return "estats";
		case EDevs:
			return "edevs";
		case Pools:
			return "pools";
		default:
			return "喵喵喵？？";
	}
}

DataProcessing::Collector::CgMinerAPI::CgMinerAPI() {


}

DataProcessing::Collector::CgMinerAPI::CgMinerAPI(time_t timestamp, DataProcessing::Collector::CgMinerAPI::APIType t,
						  Reimu::SQLAutomator::SQLite3 db, Reimu::IPEndPoint ep) {
	TimeStamp = timestamp;
	Type = t;
	DestDB = db;
	RemoteEP = ep;
}

void DataProcessing::Collector::CgMinerAPI::ParseAPIData(const char *api_obj_name, vector<string> json_keys) {
	json_t *j_apidata_array;
	json_t *j_apidata;
	size_t j;

	size_t narg;


	j_apidata_array = json_object_get(JsonAPIData, api_obj_name);

	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}

	try {

		json_array_foreach(j_apidata_array, j, j_apidata) {

			if (!json_is_object(j_apidata)) {
				// TODO
				goto end;
			}

			DestDB.Bind(1, TimeStamp);
			DestDB.Bind(2, {RemoteEP.Addr, RemoteEP.AddressFamily == AF_INET ? 4 : 16});
			DestDB.Bind(3, RemoteEP.Port);

			narg = 4;

			for (auto const &key: json_keys) {
				json_t *buf0 = json_object_get(j_apidata, key.c_str());

				if (!buf0) {
					// TODO
					goto end;
				}

				if (json_is_integer(buf0))
					DestDB.Bind(narg, (uint64_t)json_integer_value(buf0));
				else if (json_is_string(buf0))
					DestDB.Bind(narg, string(json_string_value(buf0)));
				else if (json_is_real(buf0))
					DestDB.Bind(narg, json_real_value(buf0));
				else if (json_is_boolean(buf0))
					DestDB.Bind(narg, json_boolean_value(buf0));

				narg++;
			}

			DestDB.Step();
			DestDB.Reset();

		}

	} catch (Reimu::Exception e) {
		fprintf(stderr, "amsd: CgMinerAPI: error writing database: %s\n", e.what());
	}

	end:
	return;
}


void DataProcessing::Collector::CgMinerAPI::Process() {
	fprintf(stderr, "amsd: CgMinerAPI: processing %s for %s\n", ToString(Type), RemoteEP.ToString());

	json_error_t j_err;

	if (!(JsonAPIData = json_loadb((const char *)&RawAPIData[0], RawAPIData.size(), 0, &j_err))) {
		Reimu::Exception e(&j_err);

		fprintf(stderr, "amsd: CgMinerAPI: error processing %s for %s: json error: %s\n", ToString(Type),
			RemoteEP.ToString(), e.what());

		throw e;
	}





}

void DataProcessing::Collector::CgMinerAPI::ParseCrap() {

	json_t *j_apidata_array = json_object_get(JsonAPIData, "STATS");

	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}

	json_t *j_apidata;
	size_t j;

	json_array_foreach(j_apidata_array, j, j_apidata) {

		if (!json_is_object(j_apidata)) {
			// TODO
			continue;
		}

		json_t *j_mm_count = json_object_get(j_apidata, "MM Count");

		if (!json_is_integer(j_mm_count)) {
			// TODO
			continue;
		}

		long mm_count = json_integer_value(j_mm_count);

		for (long thismmid = 1; thismmid <= mm_count; thismmid++) {
			string thismmkey = "MM ID" + to_string(thismmid);
			json_t *j_this_mm_crap = json_object_get(j_apidata, thismmkey.c_str());

			if (!json_is_string(j_this_mm_crap)) {
				// TODO
				continue;
			}


			const char *this_mm_crap = json_string_value(j_this_mm_crap);

			unordered_map<string, vector<string>> killedCrap = CrapNemesis((char *)this_mm_crap);

			map<string, Reimu::UniversalType> sequencedCrap;

			for (auto &thisCrap : killedCrap) {
				if (thisCrap.second.size() == 1) {
					sequencedCrap[thisCrap.first] = thisCrap.second[0];
				} else {
					int ccounter = 0;
					for (auto &moreCrap : thisCrap.second) {
						sequencedCrap[thisCrap.first+"_"+to_string(ccounter)] = moreCrap;
						ccounter++;
					}
				}
			}


			DestDB.PPB(INSERT_INTO, "module_avalon7", sequencedCrap);
			cout << DestDB.Statement << endl;
			DestDB.Exec();

		}

	}


}

