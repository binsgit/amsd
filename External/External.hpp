//
// Created by root on 17-5-15.
//

#ifndef AMSD_EXTERNAL_HPP
#define AMSD_EXTERNAL_HPP

#include "../amsd.hpp"

namespace AMSD {
    class External {
    public:
	static unordered_map<string, double> BlockChainAPI();
    };
}
#endif //AMSD_EXTERNAL_HPP
