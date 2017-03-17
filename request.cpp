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


int amsd_request_parse(char *inputstr, string &outputstr){
	json_error_t err;
	json_t *i_json_root = json_loads((const char *)inputstr, 0, &err);
	json_t *j_operation, *j_data, *o_json_root, *o_json_opdata, *o_json_rc;
	json_t *j_auth;
	std::pair<void *, bool> thisopt;
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

	try {
		thisopt = amsd_operation_get(operation);
	} catch (int e) {
		fprintf(stderr, "amsd: request %p: error: operation `%s' unregistered\n", (void *)inputstr, operation.c_str());
		goto parseerr;
	}

	op = (int (*)(json_t *, json_t *&))thisopt.first;

	j_auth = json_object_get(i_json_root, "auth");

	if (!json_is_string(j_auth)) {
		if (thisopt.second) {
			fprintf(stderr, "amsd: request %p: error: operation `%s' requires authentication\n", (void *)inputstr, operation.c_str());
			goto autherr;
		}
	} else {
		if (amsd_user_auth(string(json_string_value(j_auth)), NULL)) {
			fprintf(stderr, "amsd: request %p: error: operation `%s' authentication failed\n", (void *)inputstr, operation.c_str());
			goto autherr;
		}
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