#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>

#include "ConfigManager.h"
#include "InfluxClient.h"
#include <mqtt/async_client.h>

class PubSubClient;
class GCPStorageClient;

class InfluxDBMonitor {
public:
    InfluxDBMonitor();

    void StartMQTTProcessor();
    void StartDataMonitor();
    void Stop();

    void ChangeExportInterval(int minutes);
    void PrintStatus();

private:
    bool exportToGCP(const std::vector<SensorReading>& data, int64_t timestamp);

    ConfigManager configManager;
    Config config;

    std::unique_ptr<InfluxClient> influxClient;
    std::unique_ptr<PubSubClient> pubsubClient;
    std::unique_ptr<GCPStorageClient> storageClient;

    std::unique_ptr<mqtt::async_client> mqttClient;

    int64_t lastExportTimestamp;
    std::atomic<bool> running;
    std::atomic<bool> monitoring;
};