//
// Created by root on 17-2-28.
//

#include "../amsd.hpp"

int amsd_operation_mailreport(json_t *in_data, json_t *&out_data){
	amsd_report_mail();
	return 0;
}