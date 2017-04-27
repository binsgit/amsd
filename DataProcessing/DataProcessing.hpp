//
// Created by root on 17-4-1.
//

#ifndef AMSD_DATAPROCESSING_HPP
#define AMSD_DATAPROCESSING_HPP

#include "../amsd.hpp"

using namespace Reimu;
using Reimu::SQLAutomator;


namespace AMSD {
    class DataProcessing {
    public:
	class Collector;
	class CgMinerAPI;
	class Report;
	class Issue;
	class AvalonError;


	class Collector {
	public:

	    struct ConnectionContext {
		Collector *thisCollector;
		CgMinerAPI *thisAPIProcessor;
	    };

	    SQLAutomator::SQLite3 DBC_Controllers;
	    SQLAutomator::SQLite3 DBC_Issue;

	    vector<SQLAutomator::SQLite3 *> DBConnections;

	    time_t TimeStamp;
	    struct timeval TimeOut = {15, 0};

	    void Collect();


	private:
	    static void event_cb(struct bufferevent *bev, short events, void *ptr);
	    static void conn_readcb(struct bufferevent *bev, void *user_data);
	    static void conn_writecb(struct bufferevent *bev, void *user_data);



	};

	class CgMinerAPI {

	public:
	    static const vector<string> JsonKeys_Summary;
	    static const vector<string> JsonKeys_Pool;
	    static const vector<string> JsonKeys_Device;
	    static const string QueryString_Summary;
	    static const string QueryString_EStats;
	    static const string QueryString_EDevs;
	    static const string QueryString_Pools;

	    enum APIType {
		Summary = 1, EStats = 2, EDevs = 3, Pools = 4
	    };

	    time_t TimeStamp = 0;
	    APIType Type;
	    Reimu::IPEndPoint RemoteEP;
	    Reimu::SQLAutomator::SQLite3 *DestDB = NULL;
	    Reimu::SQLAutomator::SQLite3 *IssueDB = NULL;
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
	    CgMinerAPI(time_t timestamp, APIType t, Reimu::SQLAutomator::SQLite3 *db, Reimu::SQLAutomator::SQLite3 *issuedb, Reimu::IPEndPoint ep);

	private:
	    void ParseAPIData(const char *api_obj_name, vector<string> json_keys);
	    void ParseCrap();

	};

	class Report {
	public:
	    class Pool {
	    public:
		string URL;
		string User;
		double GHS = 0;
	    };

	    class Controller {
	    public:
		size_t Elapsed = 0;
		Reimu::IPEndPoint RemoteEP;
	    };

	    class Farm {
	    public:
		size_t Modules = 0;
		long double MHS = 0;

		vector<Controller> Controllers;
		map<pair<string, string>, Pool> Pools;
	    };

	public:
	    string Name;
	    Farm Farm0;
	    timeval ProcessTime;

	    Report(string farm_name, bool collect_pool=1);

	    string HTML();

	private:
	    void CollectData();
	    bool CollectPool;
	};

	class AvalonError {
	public:
	    enum AvalonErrNum {
		Idle = 1, CRCFailed = 2, NoFan = 4, Lock = 8, APIFIFOverflow = 16, RBOverflow = 32, TooHot = 64, HotBefore = 128,
		LoopFailed = 256, CoreTestFailed = 512, InvaildPMU = 1024, PGFailed = 2048, NTCErr = 4096, VolErr = 8192,
		VCoreErr = 16384, PMUCrcFailed = 32768, InvaildPLLValue = 65536,
		Error_WU = 0x20000, Error_MW = 0x40000, Error_CRC = 0x80000, Error_DH = 0x100000
	    };

	    uint64_t ErrNum;
	    uint16_t AUC, Module;
	    uint8_t DNA[8] = {0};
	    time_t Time;

	    AvalonError();
	    AvalonError(uint64_t errnum, uint16_t auc, uint16_t module, uint64_t dna);
	    AvalonError(uint64_t errnum, uint16_t auc, uint16_t module, uint8_t dna[8]);
	    AvalonError(vector<uint8_t> issue_desc);

	    vector<uint8_t> Desc();

	    static uint64_t DetectExtErrs(char *ver, double wu, double dh, uint32_t crc);
	    static string ToString(uint64_t ErrNum);
	    string ToString();

	};

	class Issue {
	public:

	    enum IssueType {
		Unknown = 0,
		ConnectionFailure = 0x10, ConnectionTimeout = 0x11,
		AvalonError = 0x20
	    };

	    time_t Time;
	    IssueType Type = Unknown;

	    Reimu::IPEndPoint RemoteEP;

	    DataProcessing::AvalonError *Error_Avalon = NULL;

	    Issue();
	    Issue(time_t time_now, IssueType type, Reimu::IPEndPoint remoteEP);
	    Issue(time_t time_now, Reimu::IPEndPoint remoteEP, uint64_t errnum, uint16_t AUC, uint16_t Module, uint64_t DNA);

	    ~Issue();

	    static const char *ToString(uint64_t errNum);

	    void WriteDatabase(Reimu::SQLAutomator::SQLite3 *db);

	};

    };
}

#endif //AMSD_DATAPROCESSING_HPP
