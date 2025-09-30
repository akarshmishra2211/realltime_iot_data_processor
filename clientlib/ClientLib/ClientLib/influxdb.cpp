#include "pch.h"
#include "influxdb.hpp"
#include <curl/curl.h>
#include <iostream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}

namespace influxdb_cpp {
    namespace detail {

        int ts_caller::post_http(const server_info& si, std::string* response, unsigned int timeout_ms) {
            CURL* curl = curl_easy_init();
            if (!curl) {
                std::cerr << "Failed to initialize curl\n";
                return -1;
            }

            std::string url = "http://" + si.host + ":" + std::to_string(si.port) + "/api/v2/write?org=" + si.usr + "&bucket=" + si.db + "&precision=" + si.precision;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // Add your Post data here - for demo, use empty body
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

            if (response) {
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
            }

            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
                return -2;
            }

            return 0;
        }

    } // namespace detail
} // namespace influxdb_cpp
