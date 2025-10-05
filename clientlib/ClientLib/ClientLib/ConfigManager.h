#pragma once

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

struct Config {
    // InfluxDB
    std::string influxdb_url;
    std::string influxdb_token;
    std::string influxdb_org;
    std::string influxdb_bucket;

    // GCP
    std::string gcp_project_id;
    std::string gcp_topic_id;
    std::string gcp_bucket_name;
    std::string gcp_key_file;

    // Export settings
    int export_interval_minutes;
    int max_records_per_export;
    bool local_backup;
    std::string export_directory;

    // MQTT
    std::string mqtt_server;
    std::string mqtt_client_id;
    std::string mqtt_topic;
};

class ConfigManager {
private:
    Config config;
    std::string configFile;

public:
    ConfigManager(const std::string& configFile = "config.json");

    bool LoadConfig();
    bool SaveConfig();
    const Config& GetConfig() const { return config; }
    void SetExportInterval(int minutes);
    void UpdateConfig(const Config& newConfig);
};