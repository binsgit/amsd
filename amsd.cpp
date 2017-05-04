/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    AMSD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMSD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "amsd.hpp"

pthread_attr_t _pthread_detached;

uint8_t *amsd_shm = NULL;

using namespace AMSD;

void ShowHelp() {
	LogE( ""
		"Usage: amsd [-c <config_dir>] [-g <config_dir>] [-h|--help]\n"
		"\n"
		"Options:\n"
		"    -c        Override default configuration directory (/etc/ams)\n"
		"    -g        Generate example configuration file(s) in given directory\n"
		"    -h        Show this help\n"
		"");
}

int main(int argc, char **argv) {

	LogI("Avalon Management System Daemon v%.2f - Get things done rapidly!\n", AMSD_VERSION);


	signal(SIGPIPE, SIG_IGN);
	pthread_attr_init(&_pthread_detached);
	pthread_attr_setdetachstate(&_pthread_detached, PTHREAD_CREATE_DETACHED);

	if (libssh2_init(0) != 0) {
		LogE("amsd: Fatal: libssh2 initialization failed!!\n");
		exit(2);
	}

	sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0);
	sqlite3_initialize();

	Reimu::ProgramOptions po(argc, argv);

	if (po.Flag("h") || po.Flag("help")) {
		ShowHelp();
		exit(0);
	}

	char *arg_cfgpath = po.OptArg("c");

	if (arg_cfgpath)
		AMSD::Config::Path_ConfigDir = arg_cfgpath;

	char *arg_gencfgpath = po.OptArg("g");

	if (arg_gencfgpath) {
		AMSD::Config::Path_ConfigDir = arg_gencfgpath;
		AMSD::Config::Init();
		if (AMSD::Config::Save() != 0) {
			LogE("amsd: Failed to generate example configuration file(s) in %s", arg_gencfgpath);
			exit(1);
		} else {
			LogI("amsd: Generated example configuration file(s) in %s", arg_gencfgpath);
			exit(0);
		}
	}


	if (Config::Load() != 0) {
		LogE("amsd: Config::Load() failed\n");
		return 2;
	}

	if (Database::Init() != 0) {
		LogE("amsd: Database::Init() failed\n");
		return 2;
	}

	if (Operations::Init() != 0){
		LogE("amsd: Operations::Init() failed\n");
		return 2;
	}

	if (RuntimeData::Init() != 0){
		LogE("amsd: RuntimeData::Init() failed\n");
		return 2;
	}

	if (Services::Init() != 0){
		LogE("amsd: Services::Init() failed\n");
		return 2;
	}

	cerr << "Remember: Too many rules will ONLY make people STUPID. - Author\n";

	while (1)
		sleep(-1);

	return 0;
}