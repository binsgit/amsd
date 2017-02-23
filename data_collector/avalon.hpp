//
// Created by root on 17-2-16.
//

#ifndef AMSD_AVALON_DATATYPES_HPP
#define AMSD_AVALON_DATATYPES_HPP

#include "../amsd.hpp"

using namespace std;

class CgMiner_APIBuf {
private:
    void ProcessData(const char *api_obj_name, sqlite3 *db, const char *sql_stmt, const vector<string> &jentries);

    void WriteDatabase();

    json_t *j_apidata_root = NULL;


public:
    enum CgMiner_APIBuf_Type {
	Summary = 1, EStats = 2, EDevs = 3, Pools = 4
    };

    CgMiner_APIBuf(CgMiner_APIBuf_Type t, time_t tm, uint64_t addr, uint16_t port);

    void Process();

    uint64_t Addr;
    uint16_t Port;
    time_t Time = 0;
    bool IsIPv6 = 0;
    sockaddr_storage RemoteAddr;
    bool CmdWritten = 0;
    CgMiner_APIBuf_Type Type;
    vector<uint8_t> Buf;
};


class Avalon_MM {
public:
    string Ver;
    uint64_t DNA;
    uint32_t Elapsed;
    uint32_t MW[4];
    uint32_t LW;
    uint32_t MH[4];
    uint32_t HW;
    float DH;
    uint8_t Temp;
    uint8_t TMax;
    uint16_t Fan;
    uint8_t FanR;
    uint16_t Vi[4];
    uint16_t Vo[4];
    uint16_t PLL[4][6];
    float GHSmm;
    float GHSmm_[4][18];
    float WU;
    float Freq;
    uint16_t PG;
    uint16_t Led;
    uint32_t MW_[4][18];
    uint16_t TA;
    uint32_t ECHU[4];
    uint32_t ECMM;
    uint16_t SF[4][6];
    uint16_t PMUV[4]; // Hex value
    double ERATIO[4][18];
    uint32_t C[4][5];
    uint16_t FM;
    uint32_t CRC[4];
    uint8_t PVT_T[4][3][2]; // [HashBoardId][Low/Hi/Avg Temp][ChipId/Temp]
};

struct Avalon_Summary {

};

class Avalon_Controller {
private:
    shared_timed_mutex _APIBufLock;
    vector<uint8_t> _APIBuf;
public:
    vector<Avalon_MM *> MM;

    void APIBuf(struct evbuffer *input);

};



#endif //AMSD_AVALON_DATATYPES_HPP
