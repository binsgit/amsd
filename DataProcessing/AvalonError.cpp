//
// Created by root on 17-4-4.
//

#include "DataProcessing.hpp"

DataProcessing::Issue::AvalonError::AvalonError() {
	ErrNum = 0;
	AUC = 0;
	Module = 0;
	Time = 0;
}

DataProcessing::Issue::AvalonError::AvalonError(uint64_t errnum, uint16_t auc, uint16_t module, uint64_t dna) {
	ErrNum = errnum;
	AUC = auc;
	Module = module;

#if __BYTE_ORDER == __BIG_ENDIAN
	dna = le32toh(dna);
#endif

	memcpy(DNA, &dna, 8);
}

DataProcessing::Issue::AvalonError::AvalonError(uint64_t errnum, uint16_t auc, uint16_t module, uint8_t *dna) {
	ErrNum = errnum;
	AUC = auc;
	Module = module;
	memcpy(DNA, dna, 8);
}

DataProcessing::Issue::AvalonError::AvalonError(vector<uint8_t> issue_desc) {
	/*
	 * issue_desc for AvalonError
	 * +---------+---------+---------+---------+
	 * | 2 bytes | 2 bytes | 8 bytes | 8 bytes |
	 * +---------+---------+---------+---------+
	 * |   AUC   |  Module |   DNA   | ErrCode |
	 * +---------+---------+---------+---------+
	 *
	 * All data are LITTLE ENDIAN.
	 *
	 */

	if (issue_desc.size() != 20) {
		throw Reimu::Exception(EINVAL);
	}

	uint8_t *p = &issue_desc[0];

	memcpy(&AUC, p, 2);
	memcpy(&Module, p+2, 2);
	memcpy(DNA, p+4, 8);
	memcpy(&ErrNum, p+12, 8);

	AUC = le16toh(AUC);
	Module = le16toh(Module);
	ErrNum = le64toh(ErrNum);
}

vector<uint8_t> DataProcessing::Issue::AvalonError::Desc() {
	vector<uint8_t> ret;

	ret.reserve(20);

	uint8_t *p = &ret[0];

	*((uint16_t *)p) = htole16(AUC);
	*((uint16_t *)(p+2)) = htole16(Module);
	memcpy(p+4, DNA, 8);
	*((uint64_t *)(p+12)) = htole64(ErrNum);

	return ret;
}

uint64_t DataProcessing::Issue::AvalonError::DetectExtErrs(char *ver, double wu, double dh, uint32_t crc) {
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

string DataProcessing::Issue::AvalonError::ToString(uint64_t ErrNum) {
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

string DataProcessing::Issue::AvalonError::ToString() {
	return ToString(ErrNum);
}