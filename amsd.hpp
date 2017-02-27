//
// Created by root on 17-2-4.
//

#ifndef AMSD_AMSD_HPP
#define AMSD_AMSD_HPP

#include <iostream>
#include <vector>
#include <atomic>
#include <regex>
#include <shared_mutex>
#include <sstream>
#include <map>
#include <unordered_map>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <libssh2.h>
#include <jansson.h>
#include <sqlite3.h>
#include <b64/encode.h>
#include <curl/curl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>


#include "core/data_collector.hpp"

#include "lib/rfc1342.hpp"
#include "lib/rfc3339.hpp"
#include "lib/cgminer_api.hpp"
#include "lib/api_parser.hpp"
#include "lib/avalon_errno.hpp"

using namespace std;

#define db_open(p,s)	sqlite3_open_v2(p, &s, SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE, NULL);\
	if (s) sqlite3_exec(s, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL)

#define db_close(s)	sqlite3_close(s)

struct amsd_si_ctx {
    int fd;
    pthread_t tid;

    vector<uint8_t> *buf_in;
    vector<uint8_t> *buf_out;
};

struct ReimuInetAddr {

    uint64_t Addr = 0;
    uint16_t Port = 0;

    bool const operator==(const ReimuInetAddr &o) const{
	    return Addr == o.Addr && Port == o.Port;
    }

    bool const operator<(const ReimuInetAddr &o) const{
	    return Addr < o.Addr || (Addr == o.Addr && Port < o.Port);
    }

};

enum GeneralStatus {
    Uninitialized = 0,
    Connecting = 0x11, Connected = 0x12,
    ExecInProgress = 0x31,
    ConnectionFailure = 0x91, AuthFailure = 0x92,  SSHSessionFailure = 0x93,
    Finished = 0x101
};


class MMUpgrade;
class SSHConnection;
class SuperRTACSession;

extern string path_runtime;
extern map<string, map<string, string>> Config;
extern pthread_attr_t _pthread_detached;

extern time_t last_collect_time;
extern shared_timed_mutex lock_datacollector;

extern const char *dbpath_controller;
extern const char *dbpath_mod_policy;
extern const char *dbpath_summary;
extern const char *dbpath_pool;
extern const char *dbpath_device;
extern const char *dbpath_module_avalon7;

// Server
extern int amsd_server();

// Core
extern void amsd_datacollector();
extern void amsd_report_mail();

// Config
extern int amsd_save_config(const char *filename="/etc/ams/config.json", bool nolock=false);
extern int amsd_load_config(const char *filename="/etc/ams/config.json");

// Database
extern int amsd_db_init();

// Utils
extern char *strrnchr(char *s, int c, size_t len, size_t n);
extern char *strrnchr(char *s, int c, size_t n);
extern string amsd_random_string();
extern string amsd_strerror(GeneralStatus status);
extern string amsd_strerror(GeneralStatus status, int _errnooo);
extern string amsd_strerror(GeneralStatus status, string xmsg);

// Request
extern int amsd_request_parse(char *inputstr, string &outputstr);

// Operations
extern void *amsd_operation_get(string name);
extern bool amsd_operation_register(string name, int (*pfunc)(json_t*, json_t*&));

extern int amsd_operation_fwver(json_t *in_data, json_t *&out_data);
extern int amsd_operation_mmupgrade(json_t *in_data, json_t *&out_data);
extern int amsd_operation_supertac(json_t *in_data, json_t *&out_data);
extern int amsd_operation_controller(json_t *in_data, json_t *&out_data);
extern int amsd_operation_issues(json_t *in_data, json_t *&out_data);


class MMUpgrade {
private:
    static void MMUpgradeInstance(MMUpgrade *m);
    static void *MMUpgradeThread(void *p);
    pthread_t ThreadId;
    pthread_mutex_t mutex_msg = PTHREAD_MUTEX_INITIALIZER;
    string _Message = "";
public:
    enum MMUpdStatus {
	Uninitialized = 0,
	DownloadInProgress = 0x11, UpgradeInProgress = 0x12,
	DownloadFinished = 0x21, UpgradeFinished = 0x22,
	DownloadError = 0x31, UpgradeError = 0x32,
	WaitingFinish = 0x41,
	Finished = 0x51
    };

    string IP = "";
    string URL_MCS = "";

    MMUpdStatus Status = Uninitialized;
    string Message();
    static void Message(MMUpgrade *mu, string m);

    int DownloadPercent = 0;
    int UpgradePercent = 0;

    void Exec();
};

class SSHConnection {
private:
    int fd_socket;

    LIBSSH2_SESSION *ssh_session = NULL;
    LIBSSH2_CHANNEL *ssh_channel = NULL;
    LIBSSH2_KNOWNHOSTS *ssh_knownhosts = NULL;

    pthread_mutex_t ReadBufferMutex = PTHREAD_MUTEX_INITIALIZER;
    vector<uint8_t> ReadBuffer;

    pthread_t ThreadId;

    static void *NonblockingThread(void *ctx);

    static bool Connector(SSHConnection *ctx);
    static bool Executor(SSHConnection *ctx);

public:

    int ConnectionErrno = 0;

    GeneralStatus Status = Uninitialized;

    string HostName = "";
    string IP = "";
    uint16_t Port = 22;
    string UserName = "";
    string Password = "";
    string Command = "";
    string PrivKeyPath = "";
    string PubKeyPath = "";
    string KnownHostsPath = "";

    ostringstream Log;
    ostringstream ErrorMessage;

    size_t ReadBytes = 0;

    int CommandExitCode = -1;
    string CommandExitSignal = "";

    void *SockAddr = NULL;

    SSHConnection();
    ~SSHConnection();

    void NonBlockingExec();

    bool Connect();
    bool Execute();

    vector<uint8_t> GetReadBuffer();

};


class SuperRTACSession {
private:
    static void ExecInstance(SuperRTACSession *m);
    static void *ExecThread(void *p);
    pthread_t ThreadId;
    pthread_mutex_t mutex_msg = PTHREAD_MUTEX_INITIALIZER;
    string _Message = "";

    SSHConnection RTACSession;

public:
    string IP = "";
    string Script = "";
    string UserName = "";
    string Password = "";


    string ScriptName = "";
    time_t StartTime = time(NULL);
    string UUID = amsd_random_string();

    GeneralStatus Status();
    string Message();

    string GetOutput();
    string GetLastOutputLine();

    void Exec();
};



#endif //AMSD_AMSD_HPP
