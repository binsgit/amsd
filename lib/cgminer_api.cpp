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

#include "cgminer_api.hpp"

int cgminer_api_request_raw(string ip, uint16_t port, string in_data, string &out_data){
	int fd, ret = 0;
	bool IPv6 = false;
	ssize_t rrc = 0;
	uint8_t buf_in[512] = {0};
	uint8_t remote_addr[sizeof(struct sockaddr_in6)];

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -2;

	// TODO: IPv6
	if (IPv6) {
		((struct sockaddr_in6 *) remote_addr)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *) remote_addr)->sin6_port = htons(port);
		inet_pton(AF_INET6, ip.c_str(), &((struct sockaddr_in6 *) remote_addr)->sin6_addr);
	} else {
		((struct sockaddr_in *) remote_addr)->sin_family = AF_INET;
		((struct sockaddr_in *) remote_addr)->sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &((struct sockaddr_in *) remote_addr)->sin_addr);
	}

	if (connect(fd, (sockaddr *)&remote_addr, IPv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)) < 0)
		goto badending;

	in_data.push_back('\n');
	send(fd, in_data.c_str(), in_data.length(), MSG_WAITALL);

	while (1) {
		if (rrc > 1)
			if (buf_in[rrc-1] == '\n')
				break;

		rrc = read(fd, buf_in, 512);

		fprintf(stderr, "amsd: cgminer_api_request_raw: read(%d, %p, %d) = %zd\n", fd, (void *)buf_in, 512, rrc);

		if (rrc < 0)
			goto badending;
		else if (rrc == 0)
			break;
		else
			out_data.insert(out_data.end(), buf_in, buf_in+rrc);
	}

	ending:
	close(fd);
	return ret;

	badending:
	ret = -1;
	goto ending;
}