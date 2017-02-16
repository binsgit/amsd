//
// Created by root on 17-2-7.
//

#include "api_parser.hpp"

#define next(name)		buf = _next(name, &holycrap, holycrap_len);if (!buf) return -2;

static inline char *_next(const char *name, char **p, size_t &totalen) {
	char *orig, *ret, *nextbrace, *nextspace;

	if (!totalen)
		return NULL;

	orig = *p;
	size_t namelen = strlen(name);

	ret = (char *)memmem(*p, totalen, name, namelen);
	if (!ret)
		return NULL;
	ret += namelen + 1;

	nextbrace = strchr(ret, ']');
	if (!nextbrace)
		return NULL;
	nextspace = nextbrace+1;
	*nextbrace = 0;

	*p = nextspace+1;
	totalen -= (*p - orig);

	return ret;
}

static inline bool str2num(char *str, void *num, size_t numsize, bool issigned=0){

	switch (numsize) {
		case 1:
			if (issigned)
				*((int8_t *)num) = (int8_t)strtoll(str, NULL, 10);
			else
				*((uint8_t *)num) = (uint8_t)strtoull(str, NULL, 10);
			break;

		case 2:
			if (issigned)
				*((int16_t *)num) = (int16_t)strtoll(str, NULL, 10);
			else
				*((uint16_t *)num) = (uint16_t)strtoull(str, NULL, 10);
			break;

		case 4:
			if (issigned)
				*((int32_t *)num) = (int32_t)strtoll(str, NULL, 10);
			else
				*((uint32_t *)num) = (uint32_t)strtoull(str, NULL, 10);
			break;

		case 8:
			if (issigned)
				*((int64_t *)num) = strtoll(str, NULL, 10);
			else
				*((uint64_t *)num) = strtoull(str, NULL, 10);
			break;

		case 128:
			*((float *)num) = strtof(str, NULL);
			break;

		case 256:
			*((double *)num) = strtod(str, NULL);
			break;
	}
}

static inline bool strs2nums(char *strs, void *nums, size_t numsize, bool issigned=0){

	char *sbuf = strdupa(strs);
	char *buf_start = sbuf;
	size_t numsize_real;
	size_t num_pos = 0;

	if (numsize > 8) {
		if (numsize == 128)
			numsize_real = sizeof(float);
		else if (numsize == 256)
			numsize_real = sizeof(double);
	} else
		numsize_real = numsize;

	while (*sbuf) {
		if (*sbuf == ' ') {
			*sbuf = 0;
			if (*(sbuf+1) != ' ') {
				str2num(buf_start, (uint8_t *) nums + num_pos, numsize, issigned);
				num_pos += numsize_real;
				buf_start = sbuf + 1;
			}
		}
		sbuf++;
	}

	if (!buf_start)
		return false;
	str2num(buf_start, (uint8_t *)nums+num_pos, numsize, issigned);

	return true;
}

int api_parse_crap(char *crap, size_t crap_len, Avalon7_MM *mm) {

	char *holycrap = crap;
	size_t holycrap_len = crap_len;

	char *buf;

	next("Ver");
	mm->Ver = string(buf);

	next("DNA");
	mm->DNA = strtoull(buf, NULL, 16);

	next("Elapsed");
	mm->Elapsed = (uint32_t)strtoul(buf, NULL, 10);

	next("MW");
	strs2nums(buf, mm->MW, 4);

	next("LW");
	mm->LW = (uint32_t)strtoul(buf, NULL, 10);

	next("MH");
	strs2nums(buf, mm->MH, 4);

	next("HW");
	mm->HW = (uint32_t)strtoul(buf, NULL, 10);

	next("DH");
	mm->DH = strtof(buf, NULL);

	next("Temp");
	mm->Temp = (uint8_t)strtoul(buf, NULL, 10);

	next("TMax");
	mm->TMax = (uint8_t)strtoul(buf, NULL, 10);

	next("Fan");
	mm->Fan = (uint16_t)strtoul(buf, NULL, 10);

	next("FanR");
	mm->FanR = (uint8_t)strtoul(buf, NULL, 10);

	next("Vi");
	strs2nums(buf, mm->Vi, 2);

	next("Vo");
	strs2nums(buf, mm->Vo, 2);

	next("PLL0");
	strs2nums(buf, mm->PLL[0], 2);

	next("PLL1");
	strs2nums(buf, mm->PLL[1], 2);

	next("PLL2");
	strs2nums(buf, mm->PLL[2], 2);

	next("PLL3");
	strs2nums(buf, mm->PLL[3], 2);

	next("GHSmm");
	mm->GHSmm = strtof(buf, NULL);

	next("WU");
	mm->WU = strtof(buf, NULL);

	next("Freq");
	mm->Freq = strtof(buf, NULL);

	next("PG");
	mm->PG = (uint16_t)strtoul(buf, NULL, 10);

	next("Led");
	mm->Led = (uint16_t)strtoul(buf, NULL, 10);

	next("MW0");
	strs2nums(buf, mm->MW_[0], 4);

	next("MW1");
	strs2nums(buf, mm->MW_[1], 4);

	next("MW2");
	strs2nums(buf, mm->MW_[2], 4);

	next("MW3");
	strs2nums(buf, mm->MW_[3], 4);

	next("TA");
	mm->TA = (uint16_t)strtoul(buf, NULL, 10);

	next("ECHU");
	strs2nums(buf, mm->ECHU, 4);

	next("ECMM");
	mm->ECMM = (uint32_t)strtoul(buf, NULL, 10);

	next("SF0");
	strs2nums(buf, mm->SF[0], 2);

	next("SF1");
	strs2nums(buf, mm->SF[1], 2);

	next("SF2");
	strs2nums(buf, mm->SF[2], 2);

	next("SF3");
	strs2nums(buf, mm->SF[3], 2);

	next("ERATIO0");
	strs2nums(buf, mm->ERATIO[0], 128);

	next("ERATIO1");
	strs2nums(buf, mm->ERATIO[1], 128);

	next("ERATIO2");
	strs2nums(buf, mm->ERATIO[2], 128);

	next("ERATIO3");
	strs2nums(buf, mm->ERATIO[3], 128);

	next("GHSmm00");
	strs2nums(buf, mm->GHSmm_[0], 128);

	next("GHSmm01");
	strs2nums(buf, mm->GHSmm_[1], 128);

	next("GHSmm02");
	strs2nums(buf, mm->GHSmm_[2], 128);

	next("GHSmm03");
	strs2nums(buf, mm->GHSmm_[3], 128);

	next("FM");
	mm->FM = (uint16_t)strtoul(buf, NULL, 10);

	next("CRC");
	strs2nums(buf, mm->CRC, 4);

	return 0;

}