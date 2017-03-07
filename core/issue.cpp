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

#include "issue.hpp"

Issue::Avalon_Error::Avalon_Error() {

}


Issue::Avalon_Error::Avalon_Error(vector<uint8_t> issue_desc) {
	/*
	 * issue_desc for AvalonError
	 * +---------+---------+---------+---------+
	 * | 2 bytes | 2 bytes | 8 bytes | 8 bytes |
	 * +---------+---------+---------+---------+
	 * |   AUC   |  Module |   DNA   | ErrCode |
	 * +---------+---------+---------+---------+
	 *
	 */

	if (issue_desc.size() != 20) {
		return;
	}

	uint8_t *p = &issue_desc[0];

	memcpy(&AUC, p, 2);
	memcpy(&Module, p+2, 2);
	memcpy(&DNA, p+4, 8);
	memcpy(&ErrNum, p+12, 8);

	AUC = le16toh(AUC);
	Module = le16toh(Module);
	DNA = le64toh(DNA);
	ErrNum = le64toh(ErrNum);

}

vector<uint8_t> Issue::Avalon_Error::Desc() {
	vector<uint8_t> ret;

	ret.reserve(20);

	uint8_t *p = &ret[0];

	*((uint16_t *)p) = htole16(AUC);
	*((uint16_t *)(p+2)) = htole16(Module);
	*((uint64_t *)(p+4)) = htole64(DNA);
	*((uint64_t *)(p+12)) = htole64(ErrNum);

	return ret;
}


string Issue::Avalon_Error::strerror(uint64_t ErrNum) {
	string errstr = "";

	if (ErrNum & Idle)
		errstr += "空闲；";

	if (ErrNum & Error_WU)
		errstr += "WU异常；";

	if (ErrNum & Error_MW)
		errstr += "MW异常；";

	if (ErrNum & Error_DH)
		errstr += "DH过高；";

	if (ErrNum & CRCFailed || ErrNum & Error_CRC)
		errstr += "CRC异常；";

	if (ErrNum & NoFan)
		errstr += "无风扇；";

	if (ErrNum & Lock)
		errstr += "";

	if (ErrNum & APIFIFOverflow)
		errstr += "";

	if (ErrNum & RBOverflow)
		errstr += "";

	if (ErrNum & TooHot)
		errstr += "温度过高；";

	if (ErrNum & HotBefore)
		errstr += "曾经温度过高；";

	if (ErrNum & LoopFailed)
		errstr += "";

	if (ErrNum & CoreTestFailed)
		errstr += "";

	if (ErrNum & InvaildPMU)
		errstr += "没有检测到PMU；";

	if (ErrNum & PGFailed)
		errstr += "供电异常；";

	if (ErrNum & NTCErr)
		errstr += "PMU温度传感异常；";

	if (ErrNum & VolErr)
		errstr += "模组电压输入异常；";

	if (ErrNum & VCoreErr)
		errstr += "模组电压输出异常；";

	if (ErrNum & PMUCrcFailed)
		errstr += "PMU通信CRC错误；";

	if (ErrNum & InvaildPLLValue)
		errstr += "PLL配置检测失败；";

	return errstr;
}

string Issue::Avalon_Error::strerror() {
	strerror(ErrNum);
}

uint64_t Issue::Avalon_Error::DetectExtErrs(char *ver, double wu, double dh, uint32_t crc) {

	uint64_t ret = 0;
	char vbuf[4];

	strncpy(vbuf, ver, 3);
	vbuf[3] = 0;

	if (strstr(vbuf, "721")) {
		if (dh > 3.5)
			ret |= Error_DH;
		if (wu < 79400)
			ret |= Error_WU;
	} else if (strstr(vbuf, "741")) {
		if (dh > 3.5)
			ret |= Error_DH;
		if (wu < 79400)
			ret |= Error_WU;
	}

	if (crc)
		ret |= CRCFailed;

	return ret;
}


Issue::Issue::Issue() {

}


Issue::Issue::Issue(Issue::IssueType type, vector<uint8_t> issue_desc) {
	Type = type;

	switch (Type) {
		case AvalonError:
			Error_Avalon = new Avalon_Error(issue_desc);
			break;
		default:
			break;
	}
}

string Issue::Issue::strerror() {
	return std::string();
}

vector<uint8_t> Issue::Issue::Desc() {
	switch (Type) {
		case AvalonError:
			return Error_Avalon->Desc();
		default:
			break;
	}
}

Issue::Issue::~Issue() {
	if (Error_Avalon)
		delete Error_Avalon;
}

string Issue::Issue::strerror(uint64_t ErrNum) {
	string ret;

	if (ErrNum == IssueType::ConnectionFailure)
		ret = "连接失败";
	else if (ErrNum == IssueType::ConnectionTimeout)
		ret = "连接超时";

	return ret;
}

