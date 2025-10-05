#pragma once

#include <string>
#include <vector>
#include <chrono>

struct SensorReading {
    std::string sensorId;
    double temperature;
    double humidity;
    int64_t timestamp;
};

class InfluxClient {
private:
    std::string url;
    std::string token;
    std::string org;
    std::string bucket;

public:
    InfluxClient(const std::string& url, const std::string& token, const std::string& org, const std::string& bucket);
    ~InfluxClient();

    // Write operations
    bool WritePoint(const std::string& sensor, double temperature, double humidity, int64_t timestamp);

    // Query operations
    std::vector<SensorReading> QueryDataRange(int64_t startTimestamp, int64_t endTimestamp);
    std::vector<SensorReading> QueryLastNMinutes(int minutes);
    int GetRecordCount(int64_t startTimestamp, int64_t endTimestamp);

    // Helper methods
    std::string ConvertToFluxQuery(int64_t startTimestamp, int64_t endTimestamp);
    std::vector<SensorReading> ParseQueryResponse(const std::string& response);
};