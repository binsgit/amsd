//
// Created by root on 17-2-7.
//

#ifndef AMSD_API_PARSER_HPP
#define AMSD_API_PARSER_HPP

#include "../amsd.hpp"

using namespace std;

struct MM {
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
struct APIOutput {

};

extern int api_parse_crap(char *crap, size_t crap_len, MM *mm);

#endif //AMSD_API_PARSER_HPP
