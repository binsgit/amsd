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




		enum APIType {
		    Summary = 1, EStats = 2, EDevs = 3, Pools = 4
		};

		time_t TimeStamp = 0;
		APIType Type;
		Reimu::IPEndPoint RemoteEP;
		Reimu::SQLAutomator::SQLite3 DestDB;

		vector<uint8_t> RawAPIData;
		json_t *JsonAPIData = NULL;

		// Static functions
		static const char *ToString(CgMinerAPI::APIType v);
		static int RequestRaw(Reimu::IPEndPoint ep, string in_data, string &out_data);
		static unordered_map<string, vector<string>> CrapNemesis(char *crap);

		//
		void Process();

		// Constructor
		CgMinerAPI();
		CgMinerAPI(time_t timestamp, APIType t, Reimu::SQLAutomator::SQLite3 db, Reimu::IPEndPoint ep);

	    private:
		void ParseAPIData(const char *api_obj_name, vector<string> json_keys);
		void ParseCrap();

	    };
	};

	class Report {
	public:
	};

	class Issue {
	public:
	};
    };
}

#endif //AMSD_DATAPROCESSING_HPP
