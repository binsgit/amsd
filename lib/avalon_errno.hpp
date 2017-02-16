//
// Created by root on 17-2-7.
//

#ifndef AMSD_AVALON_ERRNO_HPP
#define AMSD_AVALON_ERRNO_HPP

enum avalon_errno {
    Idle = 1, CRCFailed = 2, NoFan = 4, Lock = 8, APIFIFOverflow = 16, RBOverflow = 32, TooHot = 64, HotBefore = 128,
    LoopFailed = 256, CoreTestFailed = 512, InvaildPMU = 1024, PGFailed = 2048, NTCErr = 4096, VolErr = 8192,
    VCoreErr = 16384, PMUCrcFailed = 32768, InvaildPLLValue = 65536
};

extern string avalon_strerror(avalon_errno e);

#endif //AMSD_AVALON_ERRNO_HPP
