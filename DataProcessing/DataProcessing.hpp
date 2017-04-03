//
// Created by root on 17-4-1.
//

#ifndef AMSD_DATAPROCESSING_HPP
#define AMSD_DATAPROCESSING_HPP

#include "../amsd.hpp"

namespace AMSD {
    class DataProcessing {
    public:
	class Collector {
	public:



	private:
	    static void event_cb(struct bufferevent *bev, short events, void *ptr);
	    static void conn_readcb(struct bufferevent *bev, void *user_data);
	    static void conn_writecb(struct bufferevent *bev, void *user_data);



	};

	class CgMinerAPI : Reimu::SQLAutomator, Reimu::SQLAutomator::SQLite3 {

	public:
	    static const vector<string> JsonKeys_Summary = {"Elapsed", "MHS av", "MHS 5s", "MHS 1m", "MHS 5m", "MHS 15m",
							    "Found Blocks", "Getworks", "Accepted", "Rejected", "Hardware Errors",
							    "Utility", "Discarded", "Stale", "Get Failures", "Local Work",
							    "Remote Failures", "Network Blocks", "Total MH", "Work Utility",
							    "Difficulty Accepted", "Difficulty Rejected", "Difficulty Stale",
							    "Best Share", "Device Hardware%", "Device Rejected%", "Pool Rejected%",
							    "Pool Stale%", "Last getwork"};

	    static const vector<string> JsonKeys_Pool = {"POOL", "URL", "Status", "Priority", "Quota", "Long Poll", "Getworks",
							 "Accepted", "Rejected", "Works", "Discarded", "Stale", "Get Failures",
							 "Remote Failures", "User", "Last Share Time", "Diff1 Shares", "Proxy Type",
							 "Proxy", "Difficulty Accepted", "Difficulty Rejected", "Difficulty Stale",
							 "Last Share Difficulty", "Work Difficulty", "Has Stratum",
							 "Stratum Active", "Stratum URL", "Stratum Difficulty", "Has GBT",
							 "Best Share", "Pool Rejected%", "Pool Stale%", "Bad Work",
							 "Current Block Height", "Current Block Version"};

	    static const vector<string> JsonKeys_Device = {"ASC", "Name", "ID", "Enabled", "Status", "Temperature", "MHS av",
							   "MHS 5s", "MHS 1m", "MHS 5m", "MHS 15m", "Accepted", "Rejected",
							   "Hardware Errors", "Utility", "Last Share Pool", "Last Share Time",
							   "Total MH", "Diff1 Work", "Difficulty Accepted", "Difficulty Rejected",
							   "Last Share Difficulty", "No Device", "Last Valid Work",
							   "Device Hardware%", "Device Rejected%", "Device Elapsed"};

	    static const string QueryString_Summary = "{\"command\":\"summary\"}";
	    static const string QueryString_EStats = "{\"command\":\"estats\"}";
	    static const string QueryString_EDevs = "{\"command\":\"edevs\"}";
	    static const string QueryString_Pools = "{\"command\":\"pools\"}";

	    enum APIType {
		Summary = 1, EStats = 2, EDevs = 3, Pools = 4
	    };

	    time_t TimeStamp = 0;
	    APIType Type;
	    Reimu::IPEndPoint RemoteEP;
	    Reimu::SQLAutomator::SQLite3 DestDB;
	    int64_t UserData = 0;

	    vector<uint8_t> RawAPIData;
	    json_t *JsonAPIData = NULL;

	    // Static functions
	    static const char *ToString(CgMinerAPI::APIType v);
	    static const string QueryString(APIType v);
	    static int RequestRaw(Reimu::IPEndPoint ep, string in_data, string &out_data);
	    static unordered_map<string, vector<string>> CrapNemesis(char *crap);

	    //
	    void Process();
	    const string QueryString();

	    // Constructor
	    CgMinerAPI();
	    CgMinerAPI(time_t timestamp, APIType t, Reimu::SQLAutomator::SQLite3 db, Reimu::IPEndPoint ep);

	private:
	    void ParseAPIData(const char *api_obj_name, vector<string> json_keys);
	    void ParseCrap();

	};

	class Report {
	public:
	};

	class Issue {
	public:
	    class AvalonError : Reimu::SQLAutomator, Reimu::SQLAutomator::SQLite3 {
	    public:
		enum AvalonErrNum {
		    Idle = 1, CRCFailed = 2, NoFan = 4, Lock = 8, APIFIFOverflow = 16, RBOverflow = 32, TooHot = 64, HotBefore = 128,
		    LoopFailed = 256, CoreTestFailed = 512, InvaildPMU = 1024, PGFailed = 2048, NTCErr = 4096, VolErr = 8192,
		    VCoreErr = 16384, PMUCrcFailed = 32768, InvaildPLLValue = 65536,
		    Error_WU = 0x20000, Error_MW = 0x40000, Error_CRC = 0x80000, Error_DH = 0x100000
		};

		uint64_t ErrNum;
		uint16_t AUC, Module;
		uint64_t DNA;
		time_t Time;

		Reimu::IPEndPoint &RemoteEP;


		AvalonError();
		AvalonError(vector<uint8_t> issue_desc);

		vector<uint8_t> Desc();

		static uint64_t DetectExtErrs(char *ver, double wu, double dh, uint32_t crc);
		static string ToString(uint64_t ErrNum);
		string ToString();

		void WriteDatabase(SQLite3 &db);
	    };
	};
    };
}

#endif //AMSD_DATAPROCESSING_HPP
