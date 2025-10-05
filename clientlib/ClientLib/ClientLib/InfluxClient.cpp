#include "pch.h"
#include "InfluxClient.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback function for CURL responses
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

InfluxClient::InfluxClient(const std::string& url, const std::string& token, const std::string& org, const std::string& bucket)
    : url(url), token(token), org(org), bucket(bucket) {
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

    std::string data = "telemetry,sensor=" + sensor + " temperature=" + std::to_string(temperature) +
        ",humidity=" + std::to_string(humidity) + " " + std::to_string(timestamp);

    std::string writeUrl = url + "/api/v2/write?org=" + org + "&bucket=" + bucket + "&precision=ns";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Token " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_URL, writeUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || response_code != 204) {
        std::cerr << "InfluxDB write failed: " << response_code << std::endl;
        return false;
    }

    return true;
}

std::string InfluxClient::ConvertToFluxQuery(int64_t startTimestamp, int64_t endTimestamp) {
    return "from(bucket:\"" + bucket + "\")"
        " |> range(start: time(v: " + std::to_string(startTimestamp) + "),"
        " stop: time(v: " + std::to_string(endTimestamp) + "))"
        " |> filter(fn: (r) => r[\"_measurement\"] == \"telemetry\")"
        " |> pivot(rowKey:[\"_time\"], columnKey: [\"_field\"], valueColumn: \"_value\")";
}

std::vector<SensorReading> InfluxClient::QueryDataRange(int64_t startTimestamp, int64_t endTimestamp) {
    std::vector<SensorReading> results;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl for query" << std::endl;
        return results;
    }

    std::string fluxQuery = ConvertToFluxQuery(startTimestamp, endTimestamp);
    std::string queryUrl = url + "/api/v2/query?org=" + org;
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Token " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/vnd.flux");
    headers = curl_slist_append(headers, "Accept: application/csv");

    curl_easy_setopt(curl, CURLOPT_URL, queryUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fluxQuery.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && response_code == 200) {
        results = ParseQueryResponse(response);
    }
    else {
        std::cerr << "InfluxDB query failed: " << response_code << std::endl;
    }

    return results;
}

std::vector<SensorReading> InfluxClient::QueryLastNMinutes(int minutes) {
    auto now = std::chrono::system_clock::now();
    auto startTime = now - std::chrono::minutes(minutes);

    int64_t endTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    int64_t startTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(startTime.time_since_epoch()).count();

    return QueryDataRange(startTimestamp, endTimestamp);
}

int InfluxClient::GetRecordCount(int64_t startTimestamp, int64_t endTimestamp) {
    std::vector<SensorReading> data = QueryDataRange(startTimestamp, endTimestamp);
    return static_cast<int>(data.size());
}

std::vector<SensorReading> InfluxClient::ParseQueryResponse(const std::string& response) {
    std::vector<SensorReading> results;

    // Parse CSV response from InfluxDB
    std::istringstream stream(response);
    std::string line;
    bool skipHeaders = true;

    while (std::getline(stream, line)) {
        if (skipHeaders && line.find("#datatype") == 0) {
            continue;
        }

        if (skipHeaders && line.find("table,result,_time") == 0) {
            skipHeaders = false;
            continue;
        }

        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse CSV line: table,result,_time,sensor,temperature,humidity
        std::istringstream lineStream(line);
        std::string field;
        std::vector<std::string> fields;

        while (std::getline(lineStream, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() >= 6) {
            SensorReading reading;
            reading.sensorId = fields[3];
            try {
                reading.temperature = std::stod(fields[4]);
                reading.humidity = std::stod(fields[5]);
                // Parse timestamp (RFC3339 format from InfluxDB)
                // For simplicity, using current timestamp - should parse RFC3339
                reading.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                results.push_back(reading);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing query result: " << e.what() << std::endl;
            }
        }
    }

    return results;
}