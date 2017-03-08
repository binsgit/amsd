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

	fprintf(stderr, "Avalon Management System Daemon v0.1 - Get things done rapidly!\n");

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

	snprintf((char *)amsd_shm, 256, "{\"user\":\"%s\", \"passwd\":\"%s\"}", amsd_local_superuser_name.c_str(),
	amsd_local_superuser_passwd.c_str());

//	msync()

	amsd_operation_register("glimpse", &amsd_operation_glimpse);
	amsd_operation_register("farmap", &amsd_operation_farmap);
	amsd_operation_register("history", &amsd_operation_history);
	amsd_operation_register("status", &amsd_operation_status);
	amsd_operation_register("issues", &amsd_operation_issues);
	amsd_operation_register("fwver", &amsd_operation_fwver);
	amsd_operation_register("ascset", &amsd_operation_ascset);
	amsd_operation_register("mmupgrade", &amsd_operation_mmupgrade);
	amsd_operation_register("supertac", &amsd_operation_supertac);
	amsd_operation_register("controller", &amsd_operation_controller);
	amsd_operation_register("mailreport", &amsd_operation_mailreport);
	amsd_operation_register("config", &amsd_operation_config);
	amsd_operation_register("rawapi", &amsd_operation_rawapi);



//	char c[] = "Ver[7211612-bc82ec0] DNA[01350bb2b48ac249] Elapsed[1190724] MW[10831824 10831860 10831860 10831968] LW[43327512] MH[5144619 2377 2602 2391] HW[5151989] DH[15.406%] Temp[11] TMax[86] Fan[2250] FanR[21%] Vi[1193 1188 1193 1194] Vo[4439 4428 4449 4449] PLL0[1300 88 63 128 103 129] PLL1[1189 680 294 46 4 91] PLL2[247 62 248 599 699 449] PLL3[195 33 150 683 1038 205] GHSmm[5899.33] WU[31191.26] Freq[640.12] PG[15] Led[0] MW0[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] MW1[10884 23504 23284 24074 22895 23404 23277 20066 21272 23536 21022 23472 21156 23330 20790 23253 22051 19901] MW2[26451 26176 26625 26264 25937 25421 24505 24551 24578 24349 23871 24917 23580 24485 24081 25081 23789 24979] MW3[24847 24698 24266 23926 23633 23251 22945 22257 21726 21853 21084 20444 20477 21317 20488 20108 20292 19583] TA[72] ECHU[0 0 0 0] ECMM[0] SF0[600 636 672 708 744 780] SF1[600 636 672 708 744 780] SF2[600 636 672 708 744 780] SF3[600 636 672 708 744 780] PMUV[75d0 75d0] ERATIO0[0.66% 5.01% 3.44% 2.81% 4.93% 0.00% 8.40% 13.05% 0.00% 17.34% 1.78% 1.31% 59.61% 0.00% 0.00% 99.84% 4.54% 2.30%] ERATIO1[99.95% 26.38% 36.39% 7.92% 22.31% 30.62% 22.23% 79.19% 76.95% 18.01% 64.82% 16.14% 53.77% 13.37% 41.06% 13.30% 32.18% 62.19%] ERATIO2[0.90% 12.20% 7.31% 13.79% 5.18% 9.15% 3.87% 68.35% 60.01% 4.82% 4.28% 24.35% 4.09% 24.87% 71.20% 46.14% 45.76% 19.78%] ERATIO3[2.32% 2.28% 2.55% 1.61% 3.02% 2.06% 2.36% 3.77% 1.73% 2.77% 1.55% 2.38% 5.51% 1.86% 3.05% 2.71% 2.11% 4.20%] C_0_00[20263 20001 20072 19945 19013    62 18343 19790  1431 16750 50464 19951 43483   364     3     2 19981 19854] C_1_00[   14 15179 12856 21233 17480 15155 16345  3705  4355 18016  7027 18420  9070 20251 12223 20568 14093  7816] C_2_00[44709 32980 34844 31923 35047 32484 29724  1882  2499 65187 64663  7654 62376  7009  1577  5403  2368  9983] C_3_00[36474 38213 37652 37845 36842 36656 36241 36314 36162 36107 36082 36288 34897 36708 34984 35763 37522 35997] C_0_01[  134  1054   716   577   987     0  1683  2969     0  3513   912   265 64163     0     0  1267   951   467] C_1_01[28415  5439  7354  1826  5019  6689  4671 14095 14540  3957 12947  3544 10549  3125  8514  3156  6688 12858] C_2_01[  406  4583  2747  5107  1914  3272  1198  4065  3750  3298  2892  2464  2660  2320  3899  4628  1998  2461] C_3_01[  865   893   984   621  1149   772   877  1423   637  1028   569   886  2033   697  1102   996   810  1578] C_0_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_1_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_2_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_3_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_0_03[  156   156   156   156   156   157   156   156   157   156   156   156   156   157   157   157   156   156] C_1_03[  157   157   157   157   157   157   157   157   157   157   157   157   157   157   157   157   157   157] C_2_03[  157   157   157   157   157   157   157   313   313   313   313   313   313   313   313   313   156   156] C_3_03[  157   157   157   157   157   157   157   157   157   157   157   157   157   157   157   157   157   157] C_0_04[    0    51   978   668   809   723  1308   215     0     0     0     0     0   270   284   271   619   891] C_1_04[   46     3     4     0     3     3     5     4     3     1     8     2     3     2     4     2     3     0] C_2_04[    8     2     4     3     4     2    10     9    10     7    15     5     8     7     2    10     2     5] C_3_04[    6    12     2     3     6     6     5     6     3     8     4    11     2     9     5     8     5     7] GHSmm00[61.80 63.00 63.60 63.60 63.00 60.18 62.40 55.20 55.80 54.61 65.02 63.48 67.56 65.34 64.70 63.29 76.80 76.80] GHSmm01[81.62 84.14 79.86 79.90 79.50 78.78 89.94 79.18 77.77 78.96 83.42 79.90 78.78 80.18 78.17 79.93 84.58 78.02] GHSmm02[89.76 92.17 91.92 91.74 91.34 90.88 96.96 88.82 89.40 87.60 86.52 90.48 93.18 93.65 89.69 92.64 97.93 93.43] GHSmm03[94.19 93.72 93.29 92.60 92.71 91.67 90.59 90.62 89.00 90.26 91.67 89.18 90.01 92.03 95.05 92.10 92.68 92.86] FM[1] CRC[0 0 0 0] PVT_T[0-8/10-29/14 17-43/10-55/49 17-65/9-85/76 0-68/8-86/77]";
//
//	Avalon_MM mm;
//
//	api_parse_crap(c, strlen(c)+1, &mm);
//
//	amsd_datacollector_instance();

//	string aaa;
//
//	cgminer_api_request_raw("127.0.0.1", 4028, "{\"command\":\"estats\"}", aaa);
//

	amsd_datacollector();
	amsd_server();



	return 0;
}