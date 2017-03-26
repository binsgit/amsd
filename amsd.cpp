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

unordered_map<string, vector<string>> api_parser_v2(char *crap);

int main() {

	unordered_map<string, vector<string>> miaoda = api_parser_v2("Ver[7411703-a6ef750] DNA[013f7756984af176] Elapsed[55683] MW[615538 615582 615626 615758] LW[2462504] MH[3200 3095 2879 3062] HW[12236] DH[1.838%] Temp[30] TMax[87] Fan[4170] FanR[66%] Vi[1211 1211 1207 1207] Vo[4429 4424 4392 4401] PLL0[164 20 87 118 830 1597] PLL1[147 18 85 138 898 1530] PLL2[337 32 118 380 1111 838] PLL3[332 14 85 219 864 1302] GHSmm[8362.06] WU[106623.65] Freq[742.37] PG[15] Led[0] MW0[2399 2321 2322 2363 2369 2181 2232 2111 2390 2296 2267 2179 2284 2263 2209 2258 2362 2290 2411 2274 2254 2344] MW1[2471 2368 2335 2321 2266 2333 2242 2386 2233 2273 2282 2244 2305 2182 2165 2261 2325 2304 2258 2299 2389 2344] MW2[2318 2321 2030 2229 2171 2230 2190 2305 2147 2158 2071 2139 2200 2142 2237 2127 2204 2152 2114 2164 2137 2340] MW3[2112 2345 2275 2148 2134 2205 2127 2252 2336 2171 2161 2129 2271 2210 2311 2291 2121 2342 2231 2187 2253 2202] TA[88] ECHU[0 512 0 0] ECMM[0] SF0[600 636 672 708 744 780] SF1[600 636 672 708 744 780] SF2[600 636 672 708 744 780] SF3[600 636 672 708 744 780] PMUV[12c0 12c0] ERATIO0[0.95% 0.80% 1.79% 0.41% 0.63% 2.02% 2.17% 1.85% 1.07% 1.84% 1.95% 2.23% 1.62% 1.67% 2.14% 1.92% 1.16% 1.26% 0.64% 1.24% 0.87% 1.38%] ERATIO1[0.71% 1.63% 1.57% 1.19% 2.01% 0.27% 1.81% 0.58% 1.50% 0.62% 2.13% 1.65% 1.08% 2.03% 1.84% 1.74% 1.62% 2.12% 2.00% 1.83% 0.54% 0.29%] ERATIO2[2.09% 1.48% 1.59% 1.96% 2.76% 2.17% 1.36% 2.71% 2.66% 2.41% 2.30% 3.08% 2.25% 3.29% 2.82% 2.55% 3.15% 0.61% 1.98% 3.44% 2.57% 2.12%] ERATIO3[2.05% 1.52% 0.97% 3.38% 3.69% 2.22% 1.22% 2.16% 1.69% 2.34% 4.44% 2.39% 1.50% 2.35% 1.23% 1.98% 3.02% 1.10% 2.40% 3.85% 2.36% 1.03%] C_0_00[53896 54429 52103 55207 54551 52372 52148 50681 53960 52875 52007 51059 52939 52572 51477 51975 51944 53253 54771 12968 54249 52781] C_1_00[54197 52443 52825 53514 51407 54993 52298 54679 52643 52503 50938 53138 52951 51003 51484 52873 51693 51111 52048 52483 54983 55324] C_2_00[51461 52668 47194 50762 49841 50495 50610 49847 49383 50226 48991 49308 50164 47605 49955 50002 49536 50047 50000 49799 51093 51448] C_3_00[50582 53156 53826 50378 49308 50355 48961 50995 52059 50020 49069 50211 52887 51066 53478 51823 49890 53680 51394 49994 51638 52153] C_0_01[  516   439   949   229   344  1077  1154   957   585   989  1033  1164   873   895  1128  1015   610   677   352   163   477   741] C_1_01[  390   867   844   644  1053   151   965   318   799   329  1108   891   577  1056   966   939   851  1105  1063   978   300   160] C_2_01[ 1099   793   763  1017  1416  1120   697  1387  1347  1238  1153  1566  1153  1617  1450  1306  1610   305  1009  1774  1346  1112] C_3_01[ 1058   819   528  1761  1887  1144   605  1124   896  1201  2279  1231   806  1231   667  1049  1551   597  1263  2003  1248   544] C_0_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_1_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_2_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_3_02[    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0] C_0_03[  153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   312   312   312   159   159   159] C_1_03[  156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156   156] C_2_03[  158   158   158   158   158   158   158   158   158   158   158   158   158   158   158   158   158   158   158   153   153   153] C_3_03[  153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153   153] C_0_04[    2     4     1     7     7     9     4     7    14     5     5     4     9     5     5    10    11    11    13     4     2     5] C_1_04[    3     5     3     4     3     6     8     5     5     2     7     5     7     9     9     8     6     4     1     9     3     6] C_2_04[    4     8     2     2     4     5     7     4     8     4     2     0     5     8     3     3     4     3    12     7     3     4] C_3_04[    9     6     0     8     5     4     4     1     4     5     5     3     8     4     6     7     1     2     4     5     5     8] GHSmm00[97.00 97.97 94.48 98.80 97.82 95.27 94.58 95.30 97.07 96.49 94.91 93.40 95.74 95.27 94.19 94.55 97.43 96.31 98.11 98.98 97.64 95.70] GHSmm01[98.15 95.66 96.49 97.43 94.73 99.23 95.66 97.93 95.92 98.76 93.83 95.70 96.56 93.25 93.50 96.35 95.23 93.83 94.80 95.66 98.54 99.48] GHSmm02[94.58 96.13 96.02 93.58 92.39 93.25 94.44 92.46 91.24 92.14 89.80 91.27 92.32 90.41 92.75 92.71 92.28 98.11 93.76 91.88 93.97 94.44] GHSmm03[95.30 96.24 96.92 93.11 91.56 94.15 97.97 93.40 95.09 91.88 92.24 92.82 95.52 93.58 96.96 94.73 91.49 96.82 93.90 92.17 94.62 98.11] FM[1] CRC[0 0 0 0] PVT_T[0-73/14-83/78 21-76/13-87/81 0-72/10-82/77 0-72/11-82/75]");

	for (auto const &aaa : miaoda) {
		vector<string> dadada = aaa.second;
		cout << aaa.first << ":" << endl;
		for (auto const &bbb : dadada) {
			cout << bbb << " ";
		}
		cout << endl;
	}

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

	if (RuntimeData::Init() != 0){
		fprintf(stderr,"amsd: RuntimeData::Init() failed\n");
		return 2;
	}


	string shm_path = path_runtime + "shm";


	amsd_shm = reimu_shm_open(shm_path, 8192, 1);

	if (!amsd_shm) {
		fprintf(stderr,"amsd: reimu_shm_open() failed\n");
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