//
// Created by root on 17-4-11.
//

#include "Runtime.hpp"

using namespace std;
map<string, map<string, string>> ConfigList;
static json_t *j_cfg = NULL;
static json_t *j_newcfg, *j_newcfg_section;

shared_timed_mutex Lock_Config;


string AMSD::Config::Path_RuntimeDir = "/var/lib/ams";
string AMSD::Config::Path_ConfigDir = "/etc/ams";


void AMSD::Config::Init() {
	ConfigList["Farm"]["Name"] = "Miku's Farm";
	ConfigList["Farm"]["Description"] = "喵喵喵喵喵";

	ConfigList["API_Unix"]["Enabled"] = "1";

	ConfigList["API_TCP"]["Enabled"] = "0";
	ConfigList["API_TCP6"]["Enabled"] = "0";

	ConfigList["DataCollector"]["ConnTimeout"] = "30";
	ConfigList["DataCollector"]["CollectInterval"] = "600";

	ConfigList["Path"]["Runtime"] = "/var/lib/ams";

	/*
	 * SMTP STARTTLS isn't supported in this version due to a security flaw: Plaintext communication with the server
	 * will continue if STARTTLS failed. (e.g. The connection got MITMed)
	 *
	 * P.S: I'm not a paranoid personality disorder patient!! :P
	 */

	ConfigList["MailReport"]["SMTP_Use_SSL"] = "true";
	ConfigList["MailReport"]["SMTP_Server"] = "smtp.google.com";
	ConfigList["MailReport"]["SMTP_Port"] = "465";
	ConfigList["MailReport"]["SMTP_User"] = "reimu@gensokyo.moe";
	ConfigList["MailReport"]["SMTP_Password"] = "bakacirno233";

	ConfigList["MailReport"]["Mail_From"] = "reimu@gensokyo.moe";
	ConfigList["MailReport"]["Mail_To"] = "marisa@gensokyo.moe,flandre@gensokyo.moe";
}

int AMSD::Config::Load() {
	int ret = 0;

	string filename = AMSD::Config::Path_ConfigDir + "/config";

	if (j_cfg)
		return -1;

	json_error_t err;

	Lock_Config.lock();
	j_cfg = json_load_file(filename.c_str(), 0, &err);

	if (!j_cfg) {
		Init();
		Save();
		cerr << "\n\nNo config file found. Generated an example at " << filename << ".\n"
		     << "Please edit it to reflect your configuration and restart me.\n";
		return 1;
	} else {
		const char *key_l1, *key_l2;
		json_t *value_l1, *value_l2;

		json_object_foreach(j_cfg, key_l1, value_l1) {
			if (json_is_object(value_l1))
				json_object_foreach(value_l1, key_l2, value_l2) {
					if (json_is_string(value_l2)) {
						ConfigList[key_l1][key_l2] = json_string_value(value_l2);
						fprintf(stderr, "Config[%s][%s] = %s\n", key_l1, key_l2,
							json_string_value(value_l2));
					}
				}
		}

		AMSD::Config::Path_RuntimeDir = ConfigList["Path"]["Runtime"];

	}

	Lock_Config.unlock();

	return ret;
}

int AMSD::Config::Save() {

	string filename = AMSD::Config::Path_ConfigDir + "/config";

	Lock_Config.lock();

	j_newcfg = json_object();

	for (auto target_l1 = ConfigList.begin(); target_l1 != ConfigList.end(); ++target_l1) {

		j_newcfg_section = json_object();

		for (auto target_l2 = target_l1->second.begin(); target_l2 != target_l1->second.end(); ++target_l2) {
			json_object_set_new(j_newcfg_section, target_l2->first.c_str(), json_string(target_l2->second.c_str()));
		}

		json_object_set_new(j_newcfg, target_l1->first.c_str(), j_newcfg_section);
	}


	int ret = json_dump_file(j_newcfg, filename.c_str(), JSON_INDENT(4));
	json_decref(j_newcfg);

	Lock_Config.unlock();

	return ret;
}
