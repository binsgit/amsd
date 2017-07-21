//
// Created by root on 17-4-19.
//

#include "Services.hpp"


struct upload_status {
    size_t readpos = 0;
    string smtp_body;
};

struct report {
    string mailbody;
    string hashrate;
    string ctls;
    string mods;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct upload_status *upload_ctx = (struct upload_status *)userp;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	size_t totalsize = upload_ctx->smtp_body.size();
	size_t leftsize = totalsize - upload_ctx->readpos;
	size_t writesize;

	if (leftsize == 0)
		return 0;

	if (leftsize < size)
		writesize = leftsize;
	else
		writesize = size;

	memcpy(ptr, upload_ctx->smtp_body.c_str()+upload_ctx->readpos, writesize);

	upload_ctx->readpos += writesize;

	return writesize;

}

void AMSD::Services::MailReport(void *userp) {


	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	time_t timenow = time(NULL);
	char datebuf[48];
	tm localtime_meow;
	string smtp_body;
	string smtp_server = "smtps://" + ConfigList["MailReport"]["SMTP_Server"] + ":" + ConfigList["MailReport"]["SMTP_Port"];
	string mail_from = ConfigList["MailReport"]["Mail_From"];
	string mail_tos = ConfigList["MailReport"]["Mail_To"];
	string farm_name = ConfigList["Farm"]["Name"];


	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, ConfigList["MailReport"]["SMTP_User"].c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, ConfigList["MailReport"]["SMTP_Password"].c_str());
		curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail_from.c_str());


		if (strchr(mail_tos.c_str(),',')) {
			stringstream mail_to_parser(mail_tos);
			while(mail_to_parser.good()) {
				string substr;
				getline(mail_to_parser, substr, ',');

				recipients = curl_slist_append(recipients, strdupa(substr.c_str()));
			}
		} else {
			recipients = curl_slist_append(recipients, mail_tos.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);


		localtime_r(&timenow, &localtime_meow);
		strftime(datebuf, 46, "%a, %d %b %Y %X", &localtime_meow);

		DataProcessing::Report thisreport(farm_name);

		smtp_body= "Date: " + string(datebuf) + " +0800\r\n" +
			   "To: " + mail_tos + "\r\n" +
			   "From: " + rfc1342_encode_utf8("AMS Reporter ~ " + farm_name) + " <"+mail_from+">\r\n" +
			   "Content-Type: text/html; charset=utf-8\r\n" +
			   "Subject: " + rfc1342_encode_utf8("AMS报告：" + farm_name) + " [" +
			   hashrate_h(thisreport.Farm0.MHS) +
			   "][" + to_string(thisreport.Farm0.Controllers.size()) +
			   " Ctls][" + to_string(thisreport.Farm0.Modules) + " Mods]" + "\r\n" +
			   "\r\n" +
			   thisreport.HTML() + "\r\n" +
			   "\r\n";

		upload_ctx.smtp_body = smtp_body;

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);
	}

	return;
}


void AMSD::Services::MailReportSpecial(void *userp) {

	if (ConfigList["MailReport"]["SpecialMail_To"].empty()) {
		return;
	}

	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	time_t timenow = time(NULL);
	char datebuf[48];
	tm localtime_meow;
	string smtp_body;
	string smtp_server = "smtps://" + ConfigList["MailReport"]["SpecialSMTP_Server"] + ":" + ConfigList["MailReport"]["SpecialSMTP_Port"];
	string mail_from = ConfigList["MailReport"]["SpecialMail_From"];
	string mail_tos = ConfigList["MailReport"]["SpecialMail_To"];
	string farm_name = ConfigList["Farm"]["Name"];


	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, ConfigList["MailReport"]["SpecialSMTP_User"].c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, ConfigList["MailReport"]["SpecialSMTP_Password"].c_str());
		curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail_from.c_str());


		if (strchr(mail_tos.c_str(),',')) {
			stringstream mail_to_parser(mail_tos);
			while(mail_to_parser.good()) {
				string substr;
				getline(mail_to_parser, substr, ',');

				recipients = curl_slist_append(recipients, strdupa(substr.c_str()));
			}
		} else {
			recipients = curl_slist_append(recipients, mail_tos.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);


		localtime_r(&timenow, &localtime_meow);
		strftime(datebuf, 46, "%a, %d %b %Y %X", &localtime_meow);

		DataProcessing::Report thisreport(farm_name);

		smtp_body= "Date: " + string(datebuf) + " +0800\r\n" +
			   "To: " + mail_tos + "\r\n" +
			   "From: " + rfc1342_encode_utf8("AMS Reporter - " + farm_name) + " <"+mail_from+">\r\n" +
			   "Content-Type: text/html; charset=utf-8\r\n" +
			   "Subject: " + rfc1342_encode_utf8("AMS报告：" + farm_name) + " [" +
			   hashrate_h(thisreport.Farm0.MHS) +
			   "][" + to_string(thisreport.Farm0.Controllers.size()) +
			   " Ctls][" + to_string(thisreport.Farm0.Modules) + " Mods]" + "\r\n" +
			   "\r\n" +
			   thisreport.HTML_Special() + "\r\n" +
			   "\r\n";

		upload_ctx.smtp_body = smtp_body;

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);
	}

	return;
}
