/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../amsd.hpp"


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

static int mailreporter_instance(){
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	time_t timenow = time(NULL);
	char datebuf[48];
	tm localtime_meow;
	string smtp_body;
	string smtp_server = "smtps://" + Config["MailReport"]["SMTP_Server"] + ":" + Config["MailReport"]["SMTP_Port"];
	string mail_from = Config["MailReport"]["Mail_From"];
	string mail_tos = Config["MailReport"]["Mail_To"];
	string farm_name = Config["Farm"]["Name"];


	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, Config["MailReport"]["SMTP_User"].c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, Config["MailReport"]["SMTP_Password"].c_str());
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

		Report::Report thisreport(farm_name);

		smtp_body= "Date: " + string(datebuf) + " +0800\r\n" +
		"To: " + mail_tos + "\r\n" +
		"From: " + rfc1342_encode_utf8("AMS Reporter - " + farm_name) + " <"+mail_from+">\r\n" +
		"Content-Type: text/html; charset=utf-8\r\n" +
		"Subject: " + rfc1342_encode_utf8("AMS报告：" + farm_name) + " [" +
				    hashrate_h(thisreport.Farm0.MHS) +
				    "][" + to_string(thisreport.Farm0.Controllers.size()) +
				    " Ctls][" + to_string(thisreport.Farm0.Modules) + " Mods]" + "\r\n" +
		"\r\n" +
		thisreport.HTMLReport() + "\r\n" +
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

	return (int)res;
}


static void *mailreporter_thread(void *meow){
	mailreporter_instance();
	pthread_exit(NULL);
}


void amsd_report_mail(){
	pthread_t tid;
	pthread_create(&tid, &_pthread_detached, &mailreporter_thread, NULL);
}