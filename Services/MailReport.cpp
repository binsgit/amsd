//
// Created by root on 17-4-19.
//

#include "Services.hpp"

#include <libReimu/SimpleSMTP/SimpleSMTP.hpp>

static const char loghdr[] = "amsd: Services::MailReport: ";

static int debug_cb(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr){

	std::string parsed_data;

	switch (type) {
		case CURLINFO_TEXT:
			parsed_data = std::string(data, data+size);
			LogD("%sSMTP: %p: %s", loghdr, userptr, parsed_data.c_str());
			break;
		case CURLINFO_HEADER_IN:
			parsed_data = std::string(data, data+size);
			LogD("%sSMTP: %p: <- %s", loghdr, userptr, parsed_data.c_str());
			break;
		case CURLINFO_HEADER_OUT:
			parsed_data = std::string(data, data+size);
			LogD("%sSMTP: %p: -> %s", loghdr, userptr, parsed_data.c_str());
			break;
		case CURLINFO_DATA_OUT:
			break;
		case CURLINFO_SSL_DATA_OUT:
			break;
		case CURLINFO_DATA_IN:
			break;
		case CURLINFO_SSL_DATA_IN:
			break;
		default:
			LogD("%sSMTP: %p: 喵喵喵？？ %d %p %zu", loghdr, userptr, type, data, size);
			break;
	}

	return 0;
}

void AMSD::Services::MailReport(void *userp) {



	string farm_name = ConfigList["Farm"]["Name"];
	string mail_tos = ConfigList["MailReport"]["Mail_To"];

	SimpleSMTP smtp;
	smtp.Flag = SimpleSMTP::Verbose;
	smtp.SMTP_Server = "smtps://" + ConfigList["MailReport"]["SMTP_Server"] + ":" + ConfigList["MailReport"]["SMTP_Port"];
	smtp.SMTP_User = ConfigList["MailReport"]["SMTP_User"];
	smtp.SMTP_Password = ConfigList["MailReport"]["SMTP_Password"];
	smtp.Sender = ConfigList["MailReport"]["Mail_From"];
	smtp.SenderName = rfc1342_encode_utf8("AMS-Reporter - " + farm_name);
	smtp.DebugCallback = &debug_cb;


	if (strchr(mail_tos.c_str(),',')) {
		stringstream mail_to_parser(mail_tos);
		while(mail_to_parser.good()) {
			string substr;
			getline(mail_to_parser, substr, ',');

			smtp.Recipients.push_back(substr);
		}
	} else {
		smtp.Recipients.push_back(mail_tos);
	}



	DataProcessing::Report thisreport(farm_name);

	LogD("%sCollecting data...\n", loghdr);

	smtp.Subject = rfc1342_encode_utf8("AMS报告：" + farm_name) + " [" +
		       hashrate_h(thisreport.Farm0.MHS) +
		       "][" + to_string(thisreport.Farm0.Controllers.size()) +
		       " Ctls][" + to_string(thisreport.Farm0.Modules) + " Mods]";


	smtp.Body = thisreport.HTML();

	LogD("%sMail body:\n%s", loghdr, smtp.Body.c_str());

	try {
		smtp.Send();
		LogI("%sMail report successfully sent.", loghdr);
	} catch (exception e) {
		LogE("%sFailed to send mail report: %s", loghdr, e.what());
	}

	mail_tos = ConfigList["MailReport"]["SpecialMail_To"];

	if (mail_tos.empty()) {
		LogW("%sSpecial mail report not enabled.", loghdr);
		return;
	}

	smtp.Recipients.clear();

	smtp.SMTP_Server = "smtps://" + ConfigList["MailReport"]["SpecialSMTP_Server"] + ":" + ConfigList["MailReport"]["SpecialSMTP_Port"];
	smtp.Sender = ConfigList["MailReport"]["SpecialMail_From"];
	smtp.SMTP_User = ConfigList["MailReport"]["SpecialSMTP_User"];
	smtp.SMTP_Password = ConfigList["MailReport"]["SpecialSMTP_Password"];
	smtp.SenderName = rfc1342_encode_utf8("AMS Reporter - " + farm_name);

	smtp.Body = thisreport.HTML_Special();


	if (strchr(mail_tos.c_str(),',')) {
		stringstream mail_to_parser(mail_tos);
		while(mail_to_parser.good()) {
			string substr;
			getline(mail_to_parser, substr, ',');

			smtp.Recipients.push_back(substr);
		}
	} else {
		smtp.Recipients.push_back(mail_tos);
	}

	return;
}


void AMSD::Services::MailReportSpecial(void *userp) {


}