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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cgicc/Cgicc.h>


using namespace std;

char *socket_path = (char *)"/tmp/.amsd_socket";

string req(char *req){

	string ret;
	struct sockaddr_un remote_addr;
	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sfd < 1) {
		return "{\"rc\": 240, \"msg\":\"socket() failed\"}";
	}

	memset(&remote_addr, 0, sizeof(struct sockaddr_un));

	remote_addr.sun_family = AF_UNIX;
	strncpy(remote_addr.sun_path, socket_path, sizeof(remote_addr.sun_path)-1);

	int cr = connect(sfd, (sockaddr *)&remote_addr, sizeof(struct sockaddr_un));

	if (cr != 0) {
		return "{\"rc\": 241, \"msg\":\"connect() failed\"}";
	}

	send(sfd, req, strlen(req), MSG_WAITALL);
	send(sfd, "\n", 1, MSG_WAITALL);

	ssize_t n = 1;
	char rcvbuf[512] = {0};

	while (n > 0) {
		n = recv(sfd, rcvbuf, 511, 0);

		if (n < 1)
			break;

		rcvbuf[n] = 0;
		ret += rcvbuf;

	}

	close(sfd);
	return ret;

}

int main(int argc, char **argv){

	cout << "Content-type: text/json\n\n";

	char *buf = (char *)malloc(16384);

	size_t n = fread(buf, 1, 16382, stdin);

	if (n < 1) {
		cout << "{\"rc\": 255, \"msg\":\"no post data\"}" << endl;
		return 0;
	}

	buf[n] = 0;

	string ret = req(buf);

	cout << ret << endl;

	return 0;
}