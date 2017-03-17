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


int main() {

	fprintf(stderr, "Avalon Management System Daemon v%.2f - Get things done rapidly!\n", AMSD_VERSION);

	mkdir("/etc/ams/", 0755);
	mkdir(path_runtime.c_str(), 0755);

	amsd_load_config();

	signal(SIGPIPE, SIG_IGN);
	pthread_attr_init(&_pthread_detached);
	pthread_attr_setdetachstate(&_pthread_detached, PTHREAD_CREATE_DETACHED);

	if (libssh2_init(0) != 0) {
		fprintf (stderr, "amsd: Fatal: libssh2 initialization failed!!\n");
		exit(2);
	}

	sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0);
	sqlite3_initialize();

	int rc_dbinit = amsd_db_init();

	fprintf(stderr,"amsd: db_init() returned %d\n",rc_dbinit);

	if (rc_dbinit)
		return 2;

	string shm_path = path_runtime + "shm";

	int shm_fd = open(shm_path.c_str(), O_RDWR|O_CREAT|O_TRUNC);

	if (shm_fd < 1) {
		fprintf(stderr,"amsd: failed to open shared memory file: %s\n", strerror(errno));
		return 2;
	}

	if (posix_fallocate(shm_fd, 0, 8192)) {
		fprintf(stderr,"amsd: failed to allocate shared memory file: %s\n", strerror(errno));
		return 2;
	}

	amsd_shm = (uint8_t *)mmap(0, 8192, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

	if (!amsd_shm) {
		fprintf(stderr,"amsd: failed to map shared memory file: %s\n", strerror(errno));
		return 2;
	}

	fprintf(stderr,"amsd: 8KB shared memory at %p\n", (void *)amsd_shm);

	snprintf((char *)amsd_shm, 256, "%s", amsd_local_superuser_token.c_str());

	amsd_operation_register("glimpse", &amsd_operation_glimpse, 0);
	amsd_operation_register("farmap", &amsd_operation_farmap, 0);
	amsd_operation_register("user", &amsd_operation_user);
	amsd_operation_register("history", &amsd_operation_history, 0);
	amsd_operation_register("status", &amsd_operation_status, 0);
	amsd_operation_register("issues", &amsd_operation_issues, 0);
	amsd_operation_register("fwver", &amsd_operation_fwver, 0);
	amsd_operation_register("ascset", &amsd_operation_ascset);
	amsd_operation_register("mmupgrade", &amsd_operation_mmupgrade);
	amsd_operation_register("supertac", &amsd_operation_supertac);
	amsd_operation_register("controller", &amsd_operation_controller);
	amsd_operation_register("mailreport", &amsd_operation_mailreport);
	amsd_operation_register("config", &amsd_operation_config);
	amsd_operation_register("rawapi", &amsd_operation_rawapi);
	amsd_operation_register("login", &amsd_operation_login, 0);
	amsd_operation_register("version", &amsd_operation_version, 0);

	cerr << "Remember: Too many rules will ONLY make people STUPID. - Author\n";

	amsd_datacollector();
	amsd_server();



	return 0;
}