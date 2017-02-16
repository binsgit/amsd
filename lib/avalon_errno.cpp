//
// Created by root on 17-2-7.
//

#include "../amsd.hpp"
#include "avalon_errno.hpp"

string avalon_strerror(int e){
	string errstr = "";

	if (e & Idle)
		errstr += "空闲；";

	if (e & CRCFailed)
		errstr += "CRC异常；";

	if (e & NoFan)
		errstr += "无风扇；";

	if (e & Lock)
		errstr += "";

	if (e & APIFIFOverflow)
		errstr += "";

	if (e & RBOverflow)
		errstr += "";

	if (e & TooHot)
		errstr += "温度过高；";

	if (e & HotBefore)
		errstr += "曾经温度过高；";

	if (e & LoopFailed)
		errstr += "";

	if (e & CoreTestFailed)
		errstr += "";

	if (e & InvaildPMU)
		errstr += "没有检测到PMU；";

	if (e & PGFailed)
		errstr += "供电异常；";

	if (e & NTCErr)
		errstr += "PMU温度传感异常；";

	if (e & VolErr)
		errstr += "模组电压输入异常；";

	if (e & VCoreErr)
		errstr += "模组电压输出异常；";

	if (e & PMUCrcFailed)
		errstr += "PMU通信CRC错误；";

	if (e & InvaildPLLValue)
		errstr += "PLL配置检测失败；";

	return errstr;
}