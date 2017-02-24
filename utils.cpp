//
// Created by root on 17-2-12.
//

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


