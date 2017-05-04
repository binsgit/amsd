//
// Created by root on 17-4-19.
//

#include "Services.hpp"

string socket_path = "/tmp/.amsd_socket";


void AMSD::Services::API::UnixSocket(void *userp) {
	/*
	 * We are not using an event-based loop because this implementation still needs one thread per conn to
	 * process a significant amount of data without letting the client wait for a long time. Also, in my opinion,
	 * the context-switching mechanism implemented by the OS is much more efficient than a hand-crafted one.
	 */

	struct sockaddr_un listen_addr;
	int fd_listener,fd_client,rc;

	if ( (fd_listener = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sun_family = AF_UNIX;
	strncpy(listen_addr.sun_path, socket_path.c_str(), sizeof(listen_addr.sun_path)-1);

	unlink(socket_path.c_str());

	if (bind(fd_listener, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) == -1) {
		perror("bind error");
		exit(2);
	}

	if (listen(fd_listener, 5) == -1) {
		perror("listen error");
		exit(2);
	}

	LogI("amsd: Services::API::UnixSocket: listening at unix domain socket `%s'", socket_path.c_str());

	chmod(socket_path.c_str(), 0666);

	while (1) {
		if ( (fd_client = accept(fd_listener, NULL, NULL)) == -1) {
			perror("accept error");
			continue;
		}

		LogI("amsd: Services::API::UnixSocket: new connection, fd %d", fd_client);

		ConCtx *newctx = (ConCtx *)calloc(1, sizeof(struct ConCtx));
		newctx->fd = fd_client;

		pthread_create(&newctx->tid, &_pthread_detached, ConnectionThread, newctx);

	}
}
