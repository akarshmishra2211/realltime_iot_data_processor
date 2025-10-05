#include "pch.h"
#include "TimeSeriesExporter.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <iostream>

using json = nlohmann::json;

TimeSeriesExporter::TimeSeriesExporter(const std::string& outputDir) : outputDir(outputDir) {
    // Create output directory if it doesn't exist
    std::filesystem::create_directories(outputDir);
}

void TimeSeriesExporter::addReading(const std::string& sensorId, double temperature, double humidity, int64_t timestamp) {
    readings.push_back({ sensorId, temperature, humidity, timestamp });
}

std::string TimeSeriesExporter::exportToJSON() {
    json jsonArray = json::array();

    for (const auto& reading : readings) {
        json jsonReading;
        jsonReading["sensorId"] = reading.sensorId;
        jsonReading["temperature"] = reading.temperature;
        jsonReading["humidity"] = reading.humidity;
        jsonReading["timestamp"] = reading.timestamp;
        jsonArray.push_back(jsonReading);
    }

    json exportData;
    exportData["export_timestamp"] = time(nullptr);
    exportData["total_readings"] = readings.size();
    exportData["data"] = jsonArray;

    return exportData.dump(2);  // Pretty print
}

std::string TimeSeriesExporter::exportToCSV() {
    std::stringstream csv;
    csv << "sensorId,temperature,humidity,timestamp\n";

    for (const auto& reading : readings) {
        csv << reading.sensorId << ","
            << std::fixed << std::setprecision(2) << reading.temperature << ","
            << std::fixed << std::setprecision(2) << reading.humidity << ","
            << reading.timestamp << "\n";
    }

    return csv.str();
}

bool TimeSeriesExporter::saveToFile(const std::string& content, const std::string& filename) {
    std::string filepath = outputDir + "/" + filename;
    std::ofstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << filepath << std::endl;
        return false;
    }

    file << content;
    file.close();

    std::cout << "Saved export to: " << filepath << std::endl;
    return true;
}

void TimeSeriesExporter::clearReadings() {
    readings.clear();
}

size_t TimeSeriesExporter::getReadingCount() const {
    return readings.size();
}
