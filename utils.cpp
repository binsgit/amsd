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

#include "amsd.hpp"

#include <random>
#include <netinet/in.h>

const uint8_t zero4[4] = {0};

string amsd_random_string(){
	string ret = "";
	char buf[24] = {0};
	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(0, INTMAX_MAX);

	snprintf(buf, 20, "%lx", dist6(rng));
	ret += string(buf);

	rng.seed(std::random_device()());
	snprintf(buf, 20, "%lx", dist6(rng));
	ret += string(buf);

	return ret;
}

string amsd_strerror(GeneralStatus status) {
	switch (status) {
		case Uninitialized:
			return "正在初始化";
		case Connecting:
			return "正在连接";
		case Connected:
			return "已连接";
		case ConnectionFailure:
			return "连接失败";
		case AuthFailure:
			return "认证失败";
		case SSHSessionFailure:
			return "SSH 会话失败";
		case ExecInProgress:
			return "正在执行";
		case Finished:
			return "已完成";
		default:
			return "喵喵喵？我不认识这个错误码的说～";
	}
}

string amsd_strerror(GeneralStatus status, int _errnooo) {
	return amsd_strerror(status) + ": " + string(strerror(_errnooo));
}

string amsd_strerror(GeneralStatus status, string xmsg) {
	return amsd_strerror(status) + ": " + xmsg;
}

// My strrnchr(): Returns a pointer to the n'st occurrence of the character c in the string s in reverse direction.

char *strrnchr(char *s, int c, size_t n){
	return strrnchr(s, c, strlen(s), n);
}

char *strrnchr(char *s, int c, size_t len, size_t n){
	char *end = s+len;
	char *pos = end;
	size_t cnt = 0;
	while (pos >= s) {
		if (*pos == c) {
			cnt++;
			if (cnt == n)
				break;
		}
		pos--;
	}

	pos++;

	if (!cnt)
		return NULL;
	else
		return pos;
}


string hashrate_h(long double mhs){
	string ret;
	char sbuf[32];

	long double duhs = mhs;
	const char *unitstr = " MH/s";

	if (mhs > 1000000000000) {
		duhs = mhs/1000000000000;
		unitstr = " EH/s";
		goto final;
	}

	if (mhs > 1000000000) {
		duhs = mhs/1000000000;
		unitstr = " PH/s";
		goto final;
	}

	if (mhs > 1000000) {
		duhs = mhs/1000000;
		unitstr = " TH/s";
		goto final;
	}

	if (mhs > 1000) {
		duhs = mhs/1000;
		unitstr = " GH/s";
		goto final;
	}

	final:
	snprintf(sbuf, 31, "%.2Lf", mhs);
	ret += sbuf;
	ret += unitstr;
	return ret;
}

double diffaccept2ghs(double diffaccept, size_t elapsed){
	return diffaccept * 4 / elapsed;
}

string strbinaddr(void *addr, size_t addrlen){
	char sbuf[INET6_ADDRSTRLEN];

	if (addrlen == 16)
		inet_ntop(AF_INET6, addr, sbuf, INET6_ADDRSTRLEN);
	else
		inet_ntop(AF_INET, addr, sbuf, INET_ADDRSTRLEN);

	return string(sbuf);
}

string strbinaddr(void *addr, size_t addrlen, uint16_t port){
	return strbinaddr(addr, addrlen) + ":" + to_string(port);
}

uint64_t bindna2int(void *dna){
	uint64_t ret = le64toh(*((uint64_t *)dna));
	return ret;
}

string strbindna(void *dna){
	char sbuf[16] = {0};
	sprintf(sbuf, "%016" PRIx64, bindna2int(dna));
	string ret = string(sbuf);
	return ret;
}

bool isOperationNoAuthPermitted(void *func){ // NOTE: Only use with amsd_operation_* functions!!
	uint8_t *first_func_call = (uint8_t *)func;
#if defined(__i386__) || defined(__x86_64__)
	first_func_call = (uint8_t *)memmem(func, 256, "\xe8", 1);
	return memmem(func, first_func_call - (uint8_t *)func, "\x90\x90\x90\x90", 4) != NULL;
#endif
#if defined(__arm__)
	first_func_call = memmem(func, 256, "\xeb", 1);
	return memmem(func, first_func_call - func, "\xe1\xa0\x00\x00\xe1\xa0\x00\x00\xe1\xa0\x00\x00\xe1\xa0\x00\x00", 16) != NULL;
#endif
	return false;
}