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

#include "amsd.hpp"

string path_runtime = "/var/lib/ams/";

map<string, map<string, string>> Config;
static json_t *j_cfg = NULL;
shared_timed_mutex Lock_Config;
static json_t *j_newcfg, *j_newcfg_section;

void amsd_init_config(){
	Config["Farm"]["Name"] = "Miku's Farm";
	Config["Farm"]["Description"] = "喵喵喵喵喵";

	Config["DataCollector"]["ConnTimeout"] = "15";
	Config["DataCollector"]["CollectInterval"] = "600";

	/*
	 * SMTP STARTTLS isn't supported in this version due to a security flaw: Plaintext communication with the server
	 * will continue if STARTTLS failed. (e.g. The connection got MITMed)
	 *
	 * P.S: I'm not a paranoid personality disorder patient!! :P
	 */

	Config["MailReport"]["SMTP_Use_SSL"] = "true";
	Config["MailReport"]["SMTP_Server"] = "smtp.google.com";
	Config["MailReport"]["SMTP_Port"] = "465";
	Config["MailReport"]["SMTP_User"] = "reimu@gensokyo.moe";
	Config["MailReport"]["SMTP_Password"] = "bakacirno233";

	Config["MailReport"]["Mail_From"] = "reimu@gensokyo.moe";
	Config["MailReport"]["Mail_To"] = "marisa@gensokyo.moe,flandre@gensokyo.moe";
}

int amsd_load_config(const char *filename){
	int ret = 0;

	if (j_cfg)
		return -1;

	json_error_t err;

	Lock_Config.lock();
	j_cfg = json_load_file(filename, 0, &err);

	if (!j_cfg) {
		amsd_init_config();
		amsd_save_config(filename, true);
		ret = 1;
		cerr << "\n\nNo config file found. Generated an example at " << filename << ".\n"
		     << "Please edit it to reflect your configuration and restart me.\n";
		exit(1);
	} else {
		const char *key_l1, *key_l2;
		json_t *value_l1, *value_l2;

		json_object_foreach(j_cfg, key_l1, value_l1) {
			if (json_is_object(value_l1))
				json_object_foreach(value_l1, key_l2, value_l2) {
					if (json_is_string(value_l2)) {
						Config[key_l1][key_l2] = json_string_value(value_l2);
						fprintf(stderr, "Config[%s][%s] = %s\n", key_l1, key_l2,
							json_string_value(value_l2));
					}
				}
		}
	}

	Lock_Config.unlock();

	return ret;
}

int amsd_save_config(const char *filename, bool nolock){
	if (!nolock)
		Lock_Config.lock();

	j_newcfg = json_object();

	for (auto target_l1 = Config.begin(); target_l1 != Config.end(); ++target_l1) {

		j_newcfg_section = json_object();

		for (auto target_l2 = target_l1->second.begin(); target_l2 != target_l1->second.end(); ++target_l2) {
			json_object_set_new(j_newcfg_section, target_l2->first.c_str(), json_string(target_l2->second.c_str()));
		}

		json_object_set_new(j_newcfg, target_l1->first.c_str(), j_newcfg_section);
	}


	int ret = json_dump_file(j_newcfg, filename, JSON_INDENT(4));
	json_decref(j_newcfg);

	if (!nolock)
		Lock_Config.unlock();

	return ret;
}