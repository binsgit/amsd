//
// Created by root on 17-2-4.
//

#include "amsd.hpp"

#define BUF_SIZE	128

string socket_path = "/tmp/.amsd_socket";


void amsd_server_instance(amsd_si_ctx *data){
	int fd = data->fd;
	uint8_t buf_in[BUF_SIZE] = {0};
	uint8_t buf_out[BUF_SIZE] = {0};

	vector<uint8_t> input;
	string output;

	char *input_str;

	ssize_t rrc = 0;

	while (1) {
		if (rrc > 1)
			if (buf_in[rrc-1] == '\n')
				break;

		rrc = read(fd, buf_in, BUF_SIZE);

		fprintf(stderr, "amsd: server: read(%d, %p, %d) = %zd\n", fd, (void *)buf_in, BUF_SIZE, rrc);

		if (rrc < 0)
			goto ending;
		else if (rrc == 0)
			break;
		else
			input.insert(input.end(), buf_in, buf_in+rrc);

	}

	input.push_back(0);

	fprintf(stderr, "amsd: server: read %zu bytes from fd %d: %s\n", input.size(), fd, input.begin());

	input_str = (char *)&input[0];

	fprintf(stderr, "amsd: server: parsing input %p\n", (void *)input_str);
	amsd_request_parse(input_str, output);

	cerr << "amsd: server: fd " << fd << " output: " << output << "\n";
	send(fd, output.c_str(), output.length(), MSG_WAITALL);

	ending:
	close(fd);
	free(data);
}

void *amsd_server_instance_thread(void *data){
	amsd_server_instance((amsd_si_ctx *)data);
	pthread_exit(NULL);
}

int amsd_server(){

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

	fprintf(stderr, "amsd: server: listening at unix domain socket `%s'\n", socket_path.c_str());

	chmod(socket_path.c_str(), 0666);

	while (1) {
		if ( (fd_client = accept(fd_listener, NULL, NULL)) == -1) {
			perror("accept error");
			continue;
		}

		fprintf(stderr, "amsd: server: new connection, fd %d\n", fd_client);

		amsd_si_ctx *newctx = (amsd_si_ctx *)calloc(1, sizeof(struct amsd_si_ctx));
		newctx->fd = fd_client;

		pthread_create(&newctx->tid, &_pthread_detached, amsd_server_instance_thread, newctx);

	}

}

