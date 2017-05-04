//
// Created by root on 17-4-1.
//

#ifndef AMSD_OPERATIONS_HPP
#define AMSD_OPERATIONS_HPP

#include "../amsd.hpp"

using namespace std;


namespace AMSD {
    class Operations : Reimu::SQLAutomator, Reimu::SQLAutomator::SQLite3 {
    public:
	static int Init();
	static std::pair<void *, bool> Get(std::string name);
	static bool Register(std::string name, int (*pfunc)(json_t*, json_t*&), bool auth_required=1);


	static int ascset(json_t *in_data, json_t *&out_data);
	static int config(json_t *in_data, json_t *&out_data);
	static int controller(json_t *in_data, json_t *&out_data);
	static int farmap(json_t *in_data, json_t *&out_data);
	static int fwver(json_t *in_data, json_t *&out_data);
	static int glimpse(json_t *in_data, json_t *&out_data);
	static int history(json_t *in_data, json_t *&out_data);
	static int issues(json_t *in_data, json_t *&out_data);
	static int login(json_t *in_data, json_t *&out_data);
	static int mailreport(json_t *in_data, json_t *&out_data);
	static int rawapi(json_t *in_data, json_t *&out_data);
	static int status(json_t *in_data, json_t *&out_data);
	static int supertac(json_t *in_data, json_t *&out_data);
	static int user(json_t *in_data, json_t *&out_data);
	static int poool(json_t *in_data, json_t *&out_data);
	static int version(json_t *in_data, json_t *&out_data);
    };
}

#endif //AMSD_OPERATIONS_HPP
