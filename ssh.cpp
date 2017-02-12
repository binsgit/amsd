//
// Created by root on 17-2-4.
//

#include "amsd.hpp"


SSHConnection::SSHConnection() {
	SockAddr = malloc(sizeof(sockaddr_storage));
}

SSHConnection::~SSHConnection() {
	fprintf(stderr, "Finalizing SSHConnection %p\n", this);


	Log.str(string());
	ErrorMessage.str(string());

	if (ssh_knownhosts) {
		libssh2_knownhost_free(ssh_knownhosts);
		ssh_knownhosts = NULL;
	}

	if (ssh_channel) {
		libssh2_channel_free(ssh_channel);
		ssh_channel = NULL;
	}

	if (ssh_session) {
		libssh2_session_free(ssh_session);
		ssh_session = NULL;
	}

	free(SockAddr);
}


bool SSHConnection::Connect() {
	if ((HostName == "" && IP == "") || UserName == "" || Command == "")
		return false;

	return Connector(this);
}

bool SSHConnection::Execute() {
	return Executor(this);
}

void SSHConnection::NonBlockingExec() {
	pthread_create(&ThreadId, NULL, &NonblockingThread, this);
	pthread_detach(ThreadId);
}

void *SSHConnection::NonblockingThread(void *ctx) {
	if (Connector((SSHConnection *)ctx))
		Executor((SSHConnection *)ctx);
	pthread_exit(NULL);
}

vector<uint8_t> SSHConnection::GetReadBuffer() {
	pthread_mutex_lock(&ReadBufferMutex);
	vector<uint8_t> ret = ReadBuffer;
	ret.push_back(0);
	pthread_mutex_unlock(&ReadBufferMutex);
	return ret;
}

bool SSHConnection::Connector(SSHConnection *ctx) {
	int rc;

	// TODO: IPv6

	ctx->Status = Connecting;

	if (1) {
		ctx->fd_socket = socket(AF_INET, SOCK_STREAM, 0);
		((sockaddr_in *)ctx->SockAddr)->sin_family = AF_INET;
		((sockaddr_in *)ctx->SockAddr)->sin_port = htons(ctx->Port);
		inet_pton(AF_INET, ctx->IP.c_str(), &((sockaddr_in *)ctx->SockAddr)->sin_addr);
	} else {
		ctx->fd_socket = socket(AF_INET6, SOCK_STREAM, 0);
		((sockaddr_in6 *)ctx->SockAddr)->sin6_family = AF_INET;
		((sockaddr_in6 *)ctx->SockAddr)->sin6_port = htons(ctx->Port);
	}

	if (connect(ctx->fd_socket, (struct sockaddr *)ctx->SockAddr, sizeof(struct sockaddr_in)) != 0) {
		ctx->ErrorMessage << "Failed to connect: " << strerror(errno) << "\n";
		ctx->Status = ConnectionFailure;
		ctx->ConnectionErrno = errno;
		return false;
	}

	ctx->ssh_session = libssh2_session_init();

	if (!ctx->ssh_session) {
		ctx->ErrorMessage << "libssh2_session_init() failed" << "\n";
		ctx->Status = SSHSessionFailure;
		return false;
	}

	if ((rc = libssh2_session_handshake(ctx->ssh_session, ctx->fd_socket))) {
		ctx->ErrorMessage << "Failure establishing SSH session: " << rc << "\n";
		ctx->Status = SSHSessionFailure;
		return false;
	}

	if (ctx->KnownHostsPath != "") {
		const char *fingerprint;
		int type;
		size_t len;
		ctx->ssh_knownhosts = libssh2_knownhost_init(ctx->ssh_session);

		if (!ctx->ssh_knownhosts){
			ctx->ErrorMessage << "libssh2_knownhost_init() failed\n";
			return false;
		}

		libssh2_knownhost_readfile(ctx->ssh_knownhosts, ctx->KnownHostsPath.c_str(), LIBSSH2_KNOWNHOST_FILE_OPENSSH);
		libssh2_knownhost_writefile(ctx->ssh_knownhosts, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

		fingerprint = libssh2_session_hostkey(ctx->ssh_session, &len, &type);

		if (fingerprint) {
			struct libssh2_knownhost *host;

			if (!libssh2_knownhost_checkp(ctx->ssh_knownhosts, ctx->HostName.c_str(), 22, fingerprint, len,
						      LIBSSH2_KNOWNHOST_TYPE_PLAIN |
						      LIBSSH2_KNOWNHOST_KEYENC_RAW,
						      &host)) {
				ctx->ErrorMessage << "Host key mismatch\n";
				return false;
			}

		} else {
			ctx->ErrorMessage << "Failed to examine known_hosts\n";
			return false;
		}

	}

	char *_aam = libssh2_userauth_list(ctx->ssh_session, ctx->UserName.c_str(), (uint)ctx->UserName.length());
	string aam = _aam ? string(_aam) : "none";

	ctx->Log << "Available authentication methods: " + aam << "\n";

	if (!libssh2_userauth_authenticated(ctx->ssh_session)) {
		if (ctx->PrivKeyPath == "") {
			// Authenticate via password
			if (libssh2_userauth_password_ex(ctx->ssh_session, ctx->UserName.c_str(),
							 (uint)ctx->UserName.length(),
							 ctx->Password.c_str(), (uint)ctx->Password.length(), NULL))
				ctx->ErrorMessage << "Password Authentication failed\n";
		} else {
			// Authenticate via pubkey
			if (libssh2_userauth_publickey_fromfile(ctx->ssh_session, ctx->UserName.c_str(),
								ctx->PubKeyPath.c_str(), ctx->PrivKeyPath.c_str(),
								ctx->Password.c_str()))
				ctx->ErrorMessage << "PublicKey Authentication failed\n";
		}

		if (libssh2_userauth_authenticated(ctx->ssh_session)) {
			ctx->Log << "Authenticated using ";
			if (ctx->PrivKeyPath != "")
				ctx->Log << "publickey";
			else
				ctx->Log << "username/password";
			ctx->Log << "\n";
		} else {
			ctx->Status = AuthFailure;
			return false;
		}
	} else {
		ctx->Log << "Authenticated using none";
	}

	ctx->Status = Connected;

	return true;
}

bool SSHConnection::Executor(SSHConnection *ctx) {
	if (ctx->Status != Connected)
		return false;

	ctx->Status = ExecInProgress;

	int rc;

	ctx->ssh_channel = libssh2_channel_open_session(ctx->ssh_session);

	if(ctx->ssh_channel == NULL) {
		ctx->Status = SSHSessionFailure;
		fprintf(stderr,"Error\n");
		return false;
	}

	rc = libssh2_channel_exec(ctx->ssh_channel, ctx->Command.c_str());

	if( rc != 0 ) {
		ctx->Status = SSHSessionFailure;
		ctx->ErrorMessage << "Unknown Error\n";
	}


	ssize_t rcs;
	do {
		char buffer[512];
		rcs = libssh2_channel_read(ctx->ssh_channel, buffer, sizeof(buffer));
		if (rcs > 0) {
			ctx->ReadBytes += rcs;
			pthread_mutex_lock(&ctx->ReadBufferMutex);
			ctx->ReadBuffer.insert(ctx->ReadBuffer.end(), buffer, buffer+rcs);
			pthread_mutex_unlock(&ctx->ReadBufferMutex);
		} else {
			if (rcs != LIBSSH2_ERROR_EAGAIN)
				fprintf(stderr, "libssh2_channel_read returned %zd\n", rcs);
		}

	} while (rcs > 0);


	ctx->CommandExitCode = 127;

	libssh2_channel_send_eof(ctx->ssh_channel);
	rc = libssh2_channel_close(ctx->ssh_channel);

	char *_exitsignal = NULL;

	if (rc == 0) {
		ctx->CommandExitCode = libssh2_channel_get_exit_status(ctx->ssh_channel);
		libssh2_channel_get_exit_signal(ctx->ssh_channel, &_exitsignal, NULL, NULL, NULL, NULL, NULL);
	}

	ctx->CommandExitSignal = _exitsignal ? string(_exitsignal) : "(null)";

	libssh2_channel_free(ctx->ssh_channel);
	ctx->ssh_channel = NULL;

	libssh2_session_disconnect_ex(ctx->ssh_session, SSH_DISCONNECT_CONNECTION_LOST, "", "");

	close(ctx->fd_socket);
	ctx->Status = Finished;

	return 0;
}

