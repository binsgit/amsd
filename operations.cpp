//
// Created by Reimu on 2017/2/4.
//

#include "amsd.hpp"

std::map<std::string, void *> Operations;

void *amsd_operation_get(std::string name) {
	std::map<std::string, void *>::const_iterator target = Operations.find(name);

	if (target == Operations.end())
		return NULL;
	else
		return target->second;
}

bool amsd_operation_register(std::string name, int (*pfunc)(json_t*, json_t*&)) {
	if (Operations.insert(std::pair<std::string, void *>(name, (void *)pfunc)).second) {
		fprintf(stderr, "amsd: operations: Registered operation `%s' at %p\n", name.c_str(), pfunc);
		return true;
	} else {
		fprintf(stderr, "amsd: operations: Failed to registered operation `%s' at %p: operation exists\n",
			name.c_str(), pfunc);
		return false;
	}
}