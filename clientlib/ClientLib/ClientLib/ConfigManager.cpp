#include "pch.h"
#include "ConfigManager.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

ConfigManager::ConfigManager(const std::string& configFile) : configFile(configFile) {
    // Set default values
    config.influxdb_url = "http://localhost:8086";
    config.export_interval_minutes = 1;
    config.max_records_per_export = 10000;
    config.local_backup = true;
    config.export_directory = "./exports";
}

bool ConfigManager::LoadConfig() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Config file not found, using defaults" << std::endl;
        return SaveConfig(); // Create default config file
    }

    try {
        json j;
        file >> j;

        // Load InfluxDB config
        if (j.contains("influxdb")) {
            auto influx = j["influxdb"];
            config.influxdb_url = influx.value("url", config.influxdb_url);
            config.influxdb_token = influx.value("token", "");
            config.influxdb_org = influx.value("org", "");
            config.influxdb_bucket = influx.value("bucket", "sensor_data");
        }

        // Load GCP config
        if (j.contains("gcp")) {
            auto gcp = j["gcp"];
            config.gcp_project_id = gcp.value("project_id", "");
            config.gcp_topic_id = gcp.value("topic_id", "sensor-updates");
            config.gcp_bucket_name = gcp.value("bucket_name", "");
            config.gcp_key_file = gcp.value("key_file", "./gcp-key.json");
        }

        // Load export config
        if (j.contains("export")) {
            auto exp = j["export"];
            config.export_interval_minutes = exp.value("interval_minutes", 1);
            config.max_records_per_export = exp.value("max_records_per_export", 10000);
            config.local_backup = exp.value("local_backup", true);
            config.export_directory = exp.value("export_directory", "./exports");
        }

        // Load MQTT config
        if (j.contains("mqtt")) {
            auto mqtt = j["mqtt"];
            config.mqtt_server = mqtt.value("server", "tcp://localhost:1883");
            config.mqtt_client_id = mqtt.value("client_id", "ProcessorClient");
            config.mqtt_topic = mqtt.value("topic", "sensors/telemetry");
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::SaveConfig() {
    json j;

    // Save InfluxDB config
    j["influxdb"]["url"] = config.influxdb_url;
    j["influxdb"]["token"] = config.influxdb_token;
    j["influxdb"]["org"] = config.influxdb_org;
    j["influxdb"]["bucket"] = config.influxdb_bucket;

    // Save GCP config
    j["gcp"]["project_id"] = config.gcp_project_id;
    j["gcp"]["topic_id"] = config.gcp_topic_id;
    j["gcp"]["bucket_name"] = config.gcp_bucket_name;
    j["gcp"]["key_file"] = config.gcp_key_file;

    // Save export config
    j["export"]["interval_minutes"] = config.export_interval_minutes;
    j["export"]["max_records_per_export"] = config.max_records_per_export;
    j["export"]["local_backup"] = config.local_backup;
    j["export"]["export_directory"] = config.export_directory;

    // Save MQTT config
    j["mqtt"]["server"] = config.mqtt_server;
    j["mqtt"]["client_id"] = config.mqtt_client_id;
    j["mqtt"]["topic"] = config.mqtt_topic;

    std::ofstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Failed to save config file" << std::endl;
        return false;
    }

    file << j.dump(2);
    return true;
}

void ConfigManager::SetExportInterval(int minutes) {
    config.export_interval_minutes = minutes;
    SaveConfig();
}

void ConfigManager::UpdateConfig(const Config& newConfig) {
    config = newConfig;
    SaveConfig();
}