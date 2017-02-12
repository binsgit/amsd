//
// Created by root on 17-2-7.
//

#ifndef AMSD_CGMINER_API_HPP
#define AMSD_CGMINER_API_HPP

#include "../amsd.hpp"

using namespace std;

extern int cgminer_api_request_raw(string ip, uint16_t port, string in_data, string &out_data);


#endif //AMSD_CGMINER_API_HPP
