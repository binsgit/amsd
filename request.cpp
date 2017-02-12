//
// Created by root on 17-2-4.
//

#include "amsd.hpp"


int amsd_request_parse(char *inputstr, string &outputstr){
	json_error_t err;
	json_t *i_json_root = json_loads((const char *)inputstr, 0, &err);
	json_t *j_operation, *j_data, *o_json_root, *o_json_opdata, *o_json_rc;
	string operation;
	int (*op)(json_t *in_data, json_t *&out_data);
	char *serialized_out;

	fprintf(stderr, "amsd: request %p: input: %s\n", (void *)inputstr, inputstr);

	if (!i_json_root) {
		fprintf(stderr, "amsd: request %p: json error: on line %d: %s\n", (void *)inputstr, err.line, err.text);
		goto loaderr;
	}

	if (!json_is_object(i_json_root)) {
		fprintf(stderr, "amsd: request %p: json parse error: root should be an object\n", (void *)inputstr);
		goto parseerr;
	}

	j_operation = json_object_get(i_json_root, "operation");

	if (!j_operation) {
		fprintf(stderr, "amsd: request %p: json parse error: key `operation' not found\n", (void *)inputstr);
		goto parseerr;
	}

	if (!json_is_string(j_operation)) {
		fprintf(stderr, "amsd: request %p: json parse error: key `operation' should be a string\n", (void *)inputstr);
		goto parseerr;
	}

	j_data = json_object_get(i_json_root, "data");

	if (!j_data) {
		fprintf(stderr, "amsd: request %p: json parse error: key `data' not found\n", (void *)inputstr);
		goto parseerr;
	}

	if (!json_is_object(j_data)) {
		fprintf(stderr, "amsd: request %p: json parse error: key `data' should be an object\n", (void *)inputstr);
		goto parseerr;
	}

	operation = string(json_string_value(j_operation));

	op = (int (*)(json_t *, json_t *&))amsd_operation_get(operation);

	if (!op) {
		fprintf(stderr, "amsd: request %p: error: operation `%s' unregistered\n", (void *)inputstr, operation.c_str());
		goto parseerr;
	}

	fprintf(stderr, "amsd: request %p: executing operation `%s' at %p\n", (void *)inputstr, operation.c_str(),
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


	fprintf(stderr, "amsd: request %p: output: %s\n", (void *)inputstr, outputstr.c_str());

	return 0;


	parseerr:
	json_decref(i_json_root);
	outputstr = "{\"rc\": 65534}";
	return -1;

	loaderr:
	json_decref(i_json_root);
	outputstr = "{\"rc\": 65535}";
	return -2;
}