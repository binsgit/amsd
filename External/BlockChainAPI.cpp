//
// Created by root on 17-5-15.
//

#include "External.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>



static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	((string *)userp)->append((const char *)contents, realsize);
	return realsize;
}

unordered_map<string, double> External::BlockChainAPI() {
	unordered_map<string, double> ret;
	bool success = 0;

	std::string readBuffer;

	CURL *curl_handle;
	CURLcode res;

	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, "https://api.blockchain.info/stats");
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&readBuffer);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	res = curl_easy_perform(curl_handle);

	/* check for errors */
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
	else {
		printf("%lu bytes retrieved\n", (long)readBuffer.size());
		success = 1;
	}

	curl_easy_cleanup(curl_handle);

	cout << readBuffer << endl;

	json_error_t j_err;
	json_t *j_stats = json_loads(readBuffer.c_str(), 0, &j_err);
	json_t *j_sitem;
	const char *key;

	if (json_is_object(j_stats)) {
		json_object_foreach(j_stats, key, j_sitem) {
			if (json_is_real(j_sitem))
				ret.insert(pair<string, double>(key, json_real_value(j_sitem)));
			else if (json_is_integer(j_sitem))
				ret.insert(pair<string, double>(key, json_integer_value(j_sitem)));
		}

		json_decref(j_stats);
	}

	if (!success)
		throw Reimu::Exception(EBADF);

	return ret;
}