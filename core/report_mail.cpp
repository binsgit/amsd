//
// Created by root on 17-2-24.
//

#include "../amsd.hpp"


struct upload_status {
    int lines_read;
    vector<string> smtp_body;
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
	const char *data;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	auto it = upload_ctx->smtp_body[upload_ctx->lines_read];

	if (upload_ctx->lines_read >= (upload_ctx->smtp_body.size()-1))
		return 0;

	size_t len = it.length();
	memcpy(ptr, it.c_str(), len);
	cerr << it;
	upload_ctx->lines_read++;
	return len;

}

static struct report mailreport_generator(string farm_name){

	timeval tv_begin, tv_end, tv_diff;
	report ret;

	char sbuf16[16];
	double dbuf;
	int64_t ibuf;

	gettimeofday(&tv_begin, NULL);

	ret.mailbody = "<html>"
			       "<head>"
			       "<meta charset=\"UTF-8\">"
			       "<style>"
			       "th {"
			       "    border: 1px solid black;"
			       "    text-align: left;"
			       "    font-family: Microsoft Yahei;"
			       "}"
			       "table {"
			       "    border-collapse: collapse;"
			       "    border: 1px solid black;"
			       "}"
			       "</style>"
			       "</head>"
			       "<body>"
			       "<h3><b>AMS报告：" + farm_name + "</b></h3>"
			       "<h4><b>当前概况</b></h4>";

	lock_datacollector.lock();

	ret.mailbody += "<table><tbody>"
				"<tr><th>数据采集时间</th><th>" + rfc3339_strftime(last_collect_time) + "</th></tr>"
				"<tr><th>总算力</th><th>";

	sqlite3 *thissummarydb, *thismoduledb, *thispooldb;

	sqlite3_stmt *stmtbuf;

	db_open(dbpath_summary, thissummarydb);


	sqlite3_prepare_v2(thissummarydb, "SELECT SUM(MHSav), Count(*) FROM summary WHERE Time = ?1", -1, &stmtbuf, NULL);
	sqlite3_bind_int64(stmtbuf, 1, last_collect_time);
	sqlite3_step(stmtbuf);
	dbuf = sqlite3_column_double(stmtbuf, 0);
	snprintf(sbuf16, 14, "%.2f", dbuf/1000000000);
	ret.hashrate += string(sbuf16);
	ret.hashrate += " PH/s";
	snprintf(sbuf16, 14, "%.2f", dbuf/1000);
	ret.mailbody += string(sbuf16);
	ret.ctls += to_string(sqlite3_column_int64(stmtbuf, 1));
	ret.mailbody += " GH/s</th></tr><tr><th>"
				"控制器数量</th><th>" + ret.ctls
			+ "</th></tr><tr><th>模组数量</th><th>";
	sqlite3_finalize(stmtbuf);


	db_open(dbpath_module_avalon7, thismoduledb);

	sqlite3_prepare_v2(thismoduledb, "SELECT Count(*) FROM module_avalon7 WHERE Time = ?1", -1, &stmtbuf, NULL);
	sqlite3_bind_int64(stmtbuf, 1, last_collect_time);
	sqlite3_step(stmtbuf);
	ret.mods += to_string(sqlite3_column_int64(stmtbuf, 0));
	ret.mailbody += ret.mods + "</th></tr></tbody></table><h4><b>矿池信息</b></h4>"
					   "<table><thead>"
					   "<tr><th>URL</th><th>用户</th><th>算力（GH/s）</th>"
//					   "<th>关联的控制器数量</th><th>关联的模组数量</th>"
					   "</tr>"
					   "</thead><tbody>";
	sqlite3_finalize(stmtbuf);

	db_open(dbpath_pool, thispooldb);

	sqlite3_prepare_v2(thispooldb, "SELECT URL, User, AVG(DifficultyAccepted) FROM pool WHERE LENGTH(URL) > 7 AND Time > ((SELECT Max(Time) FROM pool) - 86400) GROUP BY URL", -1, &stmtbuf, NULL);

	while ( sqlite3_step(stmtbuf) == SQLITE_ROW ) {
		ret.mailbody += "<tr><th>";
		ret.mailbody += (char *)sqlite3_column_text(stmtbuf,0);
		ret.mailbody += "</th><th>";
		ret.mailbody += (char *)sqlite3_column_text(stmtbuf,1);
		ret.mailbody += "</th><th>";
		snprintf(sbuf16, 14, "%.2f", sqlite3_column_double(stmtbuf, 2)/1000);
		ret.mailbody += sbuf16;
//		ret.mailbody += "</th><th>";
//
//		ret.mailbody += "</th><th>";

		ret.mailbody += "</th></tr>";
	}

	db_close(thismoduledb);
	db_close(thissummarydb);
	db_close(thispooldb);

	lock_datacollector.unlock();

	ret.mailbody += "</tbody></table><br><i>Processed in ";

	gettimeofday(&tv_end, NULL);
	timersub(&tv_end, &tv_begin, &tv_diff);

	snprintf(sbuf16, 14, "%zu.%zu", tv_diff.tv_sec, tv_diff.tv_usec);
	ret.mailbody += sbuf16;
	ret.mailbody += " seconds.</i></body></html>";

	return ret;
}

static int mailreporter_instance(){
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	time_t timenow = time(NULL);
	char datebuf[48];
	tm localtime_meow;
	vector<string> smtp_body;
	string smtp_server = "smtps://" + Config["MailReport"]["SMTP_Server"] + ":" + Config["MailReport"]["SMTP_Port"];
	string mail_from = Config["MailReport"]["Mail_From"];
	string mail_tos = Config["MailReport"]["Mail_To"];
	string farm_name = Config["Farm"]["Name"];

	upload_ctx.lines_read = 0;


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

		report thisrpt = mailreport_generator(farm_name);

		smtp_body.push_back("Date: " + string(datebuf) + " +0800\r\n");
		smtp_body.push_back("To: " + mail_tos + "\r\n");
		smtp_body.push_back("From: " + rfc1342_encode_utf8("AMS Reporter - " + farm_name) + " <"+mail_from+">\r\n");
		smtp_body.push_back("Content-Type: text/html; charset=utf-8\r\n");
		smtp_body.push_back("Subject: " + rfc1342_encode_utf8("AMS报告：" + farm_name) + " [" + thisrpt.hashrate +
				    "][" + thisrpt.ctls + " Ctls][" + thisrpt.mods + " Mods]\r\n");
		smtp_body.push_back("\r\n");
		smtp_body.push_back(thisrpt.mailbody + "\r\n");
		smtp_body.push_back("\r\n");

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