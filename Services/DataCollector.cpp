//
// Created by root on 17-4-19.
//

#include "Services.hpp"

void AMSD::Services::DataCollector(void *userp) {
	LogD("amsd: Services::DataCollector: Started.");

	DataProcessing::Collector thisCollector;

	thisCollector.Collect();

	LogD("amsd: Services::DataCollector: Done.");

}