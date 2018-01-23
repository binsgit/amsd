//
// Created by root on 17-4-19.
//

#include "Services.hpp"

#define BUF_SIZE	128

int AMSD::Services::API::ParseRequest(char *inputstr, string &outputstr) {
	json_error_t err;
	json_t *i_json_root = json_loads((const char *)inputstr, 0, &err);
	json_t *j_operation, *j_data, *o_json_root, *o_json_opdata, *o_json_rc;
	json_t *j_auth;
	std::pair<void *, bool> thisopt;
	string operation;
	int (*op)(json_t *in_data, json_t *&out_data);
	char *serialized_out;

//	LogW("amsd: request %p: input: %s\n", (void *)inputstr, inputstr);

	if (!i_json_root) {
		LogW("amsd: Services::API::ParseRequest: request %p: json error: on line %d: %s", (void *)inputstr, err.line, err.text);
		goto loaderr;
	}

	if (!json_is_object(i_json_root)) {
		LogW("amsd: Services::API::ParseRequest: request %p: json parse error: root should be an object", (void *)inputstr);
		goto parseerr;
	}

	j_operation = json_object_get(i_json_root, "operation");

	if (!j_operation) {
		LogW("amsd: Services::API::ParseRequest: request %p: json parse error: key `operation' not found", (void *)inputstr);
		goto parseerr;
	}

	if (!json_is_string(j_operation)) {
		LogW("amsd: Services::API::ParseRequest: request %p: json parse error: key `operation' should be a string", (void *)inputstr);
		goto parseerr;
	}

	j_data = json_object_get(i_json_root, "data");

	if (!j_data) {
		LogW("amsd: Services::API::ParseRequest: request %p: json parse error: key `data' not found", (void *)inputstr);
		goto parseerr;
	}

	if (!json_is_object(j_data)) {
		LogW("amsd: Services::API::ParseRequest: request %p: json parse error: key `data' should be an object", (void *)inputstr);
		goto parseerr;
	}

	operation = string(json_string_value(j_operation));

	try {
		thisopt = AMSD::Operations::Get(operation);
	} catch (int e) {
		LogW("amsd: Services::API::ParseRequest: request %p: error: operation `%s' unregistered", (void *)inputstr, operation.c_str());
		goto parseerr;
	}

	op = (int (*)(json_t *, json_t *&))thisopt.first;

	j_auth = json_object_get(i_json_root, "auth");


	if (thisopt.second) {
		if (!json_is_string(j_auth)) {
			LogE("amsd: Services::API::ParseRequest: request %p: error: operation `%s' requires authentication", (void *)inputstr, operation.c_str());
			goto autherr;
		} else {
			if (AMSD::User::Auth(string(json_string_value(j_auth)), NULL)) {
				LogE("amsd: Services::API::ParseRequest: request %p: error: operation `%s' authentication failed", (void *)inputstr, operation.c_str());
				goto autherr;
			}
		}
	}


	LogD("amsd: Services::API::ParseRequest: request %p: executing operation `%s' at %p", (void *)inputstr, operation.c_str(),
		(void *)op);

	o_json_root = json_object();
	o_json_opdata = json_object();
	o_json_rc = json_integer(op(j_data, o_json_opdata));

	json_object_set_new(o_json_root, "rc", o_json_rc);
	json_object_set_new(o_json_root, "data", o_json_opdata);

	serialized_out = json_dumps(o_json_root, 0);
	outputstr = string(serialized_out);
	free(serialized_out);
	json_decref(o_json_root);
	json_decref(i_json_root);


//	LogW("amsd: request %p: output: %s\n", (void *)inputstr, outputstr.c_str());

	return 0;

	autherr:
	json_decref(i_json_root);
	outputstr = "{\"rc\": 65533}";
	return -1;

	parseerr:
	json_decref(i_json_root);
	outputstr = "{\"rc\": 65534}";
	return -1;

	loaderr:
	json_decref(i_json_root);
	outputstr = "{\"rc\": 65535}";
	return -2;
}

void Services::API::ConnectionHandler(Services::API::ConCtx *data) {
	int fd = data->fd;
	uint8_t buf_in[BUF_SIZE] = {0};
	uint8_t buf_out[BUF_SIZE] = {0};

	vector<uint8_t> input;
	string output;

	char *input_str;

	ssize_t rrc = 0;

	while (1) {
		if (rrc >= 1)
			if (buf_in[rrc-1] == '\n')
				break;

		rrc = read(fd, buf_in, BUF_SIZE);

//		LogW("amsd: server: read(%d, %p, %d) = %zd\n", fd, (void *)buf_in, BUF_SIZE, rrc);

		if (rrc < 0)
			goto ending;
		else if (rrc == 0)
			break;
		else
			input.insert(input.end(), buf_in, buf_in+rrc);

	}

	input.push_back(0);

//	LogW("amsd: server: read %zu bytes from fd %d: %s\n", input.size(), fd, input.begin());

	input_str = (char *)&input[0];

//	LogW("amsd: server: parsing input %p\n", (void *)input_str);
	ParseRequest(input_str, output);

//	cerr << "amsd: server: fd " << fd << " output: " << output << "\n";
	send(fd, output.c_str(), output.length(), MSG_WAITALL);

	ending:
	close(fd);
	free(data);
}

void *Services::API::ConnectionThread(void *data) {
	ConnectionHandler((ConCtx *)data);
	pthread_exit(NULL);
}


