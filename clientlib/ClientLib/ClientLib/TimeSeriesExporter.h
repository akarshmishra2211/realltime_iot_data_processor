#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct SensorReading {
    std::string sensorId;
    double temperature;
    double humidity;
    int64_t timestamp;
};

class TimeSeriesExporter {
private:
    std::vector<SensorReading> readings;
    std::string outputDir;

public:
    TimeSeriesExporter(const std::string& outputDir = "./exports");

    void addReading(const std::string& sensorId, double temperature, double humidity, int64_t timestamp);
    std::string exportToJSON();
    std::string exportToCSV();
    bool saveToFile(const std::string& content, const std::string& filename);
    void clearReadings();
    size_t getReadingCount() const;
};

