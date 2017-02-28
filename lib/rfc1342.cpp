//
// Created by root on 17-2-27.
//

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