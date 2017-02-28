//
// Created by root on 17-2-27.
//

#include "rfc3339.hpp"

using namespace std;

string rfc3339_strftime(time_t timeee){
	char sbuf[48];

	tm tmtm;
	localtime_r(&timeee, &tmtm);
	strftime(sbuf, 46, "%Y-%m-%d %T", &tmtm);

	return string(sbuf);
}