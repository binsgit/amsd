//
// Created by root on 17-2-23.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

char *socket_path = (char *)"/etc/.amsd_socket";

void help(){
	fprintf(stderr, "amsd_cli v0.1 - Command line interface for AMSD\n"
		"\n"
		"Usage: amsd_cli [-s <socket_path>] <operation> <sub-operation> [args...]\n"
		"\n"
		"Options:\n"
		"    -s        Specifies socket path instead of /tmp/.amsd_socket\n"
		"\n"
		"Operations:\n"
		"    controller:\n"
		"        list                            List controllers\n"
		"        add <ip:port> [<ip:port>...]    Add controller(s)\n"
		"        del <ip:port> [<ip:port>...]    Delete controller(s)\n"
		"\n"
		"Examples:\n"
		"    # amsd_cli controller list\n"
		"    # amsd_cli -s /tmp/niconico_socket fwver 192.168.1.233\n");
}

int main(int argc, char **argv){
	char **ops_argv = argv + sizeof(char **);

	if (argc < 3)
		goto badarg;

	if (0 == strcmp(argv[1], "-s")) {
		if (argc < 4)
			goto badarg;
		ops_argv += sizeof(char **) * 2;
	}

	if (0 == strcmp(ops_argv[0], "controller")) {

	}



	badarg:
	help();
	exit(1);

}