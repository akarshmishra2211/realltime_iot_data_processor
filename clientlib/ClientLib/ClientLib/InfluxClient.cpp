#include "pch.h"
#include "InfluxClient.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>

InfluxClient::InfluxClient(const std::string& url, const std::string& token,
    const std::string& org, const std::string& bucket)
    : url_(url), token_(token), org_(org), bucket_(bucket) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

InfluxClient::~InfluxClient() {
    curl_global_cleanup();
}

bool InfluxClient::WritePoint(const std::string& sensor, double temperature, double humidity, int64_t timestamp) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return false;
    }

    std::ostringstream data;
    data << std::fixed << std::setprecision(2);
    data << "telemetry,sensor=" << sensor
        << " temperature=" << temperature
        << ",humidity=" << humidity
        << " " << timestamp; // no trailing newline or spaces

    std::string data_str = data.str();
    std::cout << "Prepared InfluxDB line protocol data:\n" << data_str << std::endl;

    std::ostringstream write_url;
    write_url << url_ << "/api/v2/write?org=" << org_ << "&bucket=" << bucket_ << "&precision=ns";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Token " + token_).c_str());
    headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_URL, write_url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_str.c_str());

    CURLcode res = curl_easy_perform(curl);
    bool success = true;
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        success = false;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
}
