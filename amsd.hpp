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

#ifndef AMSD_AMSD_HPP
#define AMSD_AMSD_HPP

#include <iostream>
#include <vector>
#include <atomic>
#include <regex>
#include <shared_mutex>
#include <sstream>
#include <map>
#include <set>
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
#include <sys/mman.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <libssh2.h>
#include <jansson.h>
#include <sqlite3.h>
#include <b64/encode.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <libReimu/Tasker/Tasker.hpp>
#include <libReimu/UniversalType/UniversalType.hpp>
#include <libReimu/IPEndPoint/IPEndPoint.hpp>
#include <libReimu/SQLAutomator/SQLAutomator.hpp>

#include "compatibility.hpp"

#include "Runtime/Runtime.hpp"
#include "Operations/Operations.hpp"
#include "DataProcessing/DataProcessing.hpp"


#include "lib/rfc1342.hpp"
#include "lib/rfc3339.hpp"

#define AMSD_VERSION		2.01


using namespace std;
using namespace AMSD;



struct amsd_si_ctx {
    int fd;
    pthread_t tid;

    vector<uint8_t> *buf_in;
    vector<uint8_t> *buf_out;
};


enum GeneralStatus {
    Uninitialized = 0,
    Connecting = 0x11, Connected = 0x12,
    ExecInProgress = 0x31,
    ConnectionFailure = 0x91, AuthFailure = 0x92,  SSHSessionFailure = 0x93,
    Finished = 0x101
};

class User;
class MMUpgrade;
class SSHConnection;
class SuperRTACSession;

extern string path_runtime;
extern map<string, map<string, string>> Config;
extern pthread_attr_t _pthread_detached;

extern time_t Timestamp_LastFinishedCollection;

extern shared_timed_mutex Lock_DataCollector;
extern shared_timed_mutex Lock_Config;



extern string amsd_local_superuser_token;

// Server
extern int amsd_server();

// Core

// Config
extern int amsd_save_config(const char *filename="/etc/ams/config.json", bool nolock=false);
extern int amsd_load_config(const char *filename="/etc/ams/config.json");

// Database
extern int amsd_db_init();

// Utils
extern char *strrnchr(char *s, int c, size_t len, size_t n);
extern char *strrnchr(char *s, int c, size_t n);
extern string hashrate_h(long double mhs);
extern double diffaccept2ghs(double diffaccept, size_t elapsed);
extern string amsd_random_string();
extern uint8_t *reimu_shm_open(string path, size_t size, bool trunc=0);
extern string amsd_strerror(GeneralStatus status);
extern string amsd_strerror(GeneralStatus status, int _errnooo);
extern string amsd_strerror(GeneralStatus status, string xmsg);
extern string strbinaddr(void *addr, size_t addrlen);
extern string strbinaddr(void *addr, size_t addrlen, uint16_t port);
extern uint64_t bindna2int(void *dna);
extern string strbindna(void *dna);
extern bool isOperationNoAuthPermitted(void *func);

// User
extern int amsd_user_login(string user, string passwd, string &token);
extern int amsd_user_auth(string token, User *userinfo);

// Request
extern int amsd_request_parse(char *inputstr, string &outputstr);

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
