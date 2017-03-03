/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMSD_ISSUE_HPP
#define AMSD_ISSUE_HPP

#include "../amsd.hpp"

namespace Issue {

    class Avalon_Error {
    public:

	enum AvalonErrNum {
	    Idle = 1, CRCFailed = 2, NoFan = 4, Lock = 8, APIFIFOverflow = 16, RBOverflow = 32, TooHot = 64, HotBefore = 128,
	    LoopFailed = 256, CoreTestFailed = 512, InvaildPMU = 1024, PGFailed = 2048, NTCErr = 4096, VolErr = 8192,
	    VCoreErr = 16384, PMUCrcFailed = 32768, InvaildPLLValue = 65536,
	    Error_WU = 0x20000, Error_MW = 0x40000, Error_CRC = 0x80000, Error_DH = 0x100000
	};

	uint64_t ErrNum = 0;
	uint16_t AUC, Module;
	uint8_t DNA;

	Avalon_Error();
	Avalon_Error(vector<uint8_t> issue_desc);
	vector<uint8_t> Desc();

	static uint64_t DetectExtErrs(char *ver, double wu, double dh, uint32_t crc);
	static string strerror(uint64_t ErrNum);
	string strerror();

    };

    class Issue {
    public:
	enum IssueType {
	    Unknown = 0,
	    ConnectionFailure = 0x10, ConnectionTimeout = 0x11,
	    AvalonError = 0x20
	};

	IssueType Type = Unknown;

	Avalon_Error *Error_Avalon = NULL;

	Issue();
	Issue(IssueType type, vector<uint8_t> issue_desc);
	vector<uint8_t> Desc();
	static string strerror(uint64_t ErrNum);
	string strerror();

	~Issue();
    };

}
#endif //AMSD_ISSUE_HPP
