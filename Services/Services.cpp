//
// Created by root on 17-4-19.
//

#include "Services.hpp"

int Services::Init() {
	Services["API"] = Tasker("API", 1, &AMSD::Services::API_UnixSocket, NULL);


	return 0;
}

