#include <iostream>
#include <string>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include "InfluxClient.h"

// MQTT Config
const std::string MQTTSERVERADDRESS = "tcp://localhost:1883";
const std::string MQTTCLIENTID = "ProcessorClient";
const std::string MQTTTOPIC = "sensors/telemetry";

// InfluxDB Config
const std::string INFLUXDBURL = "http://127.0.0.1:8086";
const std::string INFLUXDBTOKEN = "w2xm26XtwwX6j0Yk-mZYIoqSy7CqPs2Roql9rkoGbsi5eu8TcK1B5A-ny2-HG4iviEgQ86d50TnuKdrh1rXx2g==";
const std::string INFLUXDBORG = "NA";
const std::string INFLUXDBBUCKET = "sensor_data";

InfluxClient influx(INFLUXDBURL, INFLUXDBTOKEN, INFLUXDBORG, INFLUXDBBUCKET);

class Callback : public virtual mqtt::callback {
public:
    void message_arrived(mqtt::const_message_ptr msg) override {
        try {
            auto json_msg = nlohmann::json::parse(msg->get_payload());

            std::string sensor_id = std::to_string(json_msg["sensor_id"].get<int>());
            double temperature = json_msg["temperature"].get<double>();
            double humidity = json_msg["humidity"].get<double>();

            int64_t timestamp = 0;
            if (json_msg.contains("timestamp")) {
                timestamp = json_msg["timestamp"].get<int64_t>();
            }

            influx.WritePoint(sensor_id, temperature, humidity, timestamp);

            std::cout << "Wrote point: sensor=" << sensor_id
                << " temp=" << temperature
                << " humidity=" << humidity
                << " timestamp=" << timestamp << std::endl;

        }
        catch (const std::exception& e) {
            std::cerr << "Failed to process MQTT message: " << e.what() << std::endl;
        }
    }

    void connection_lost(const std::string& cause) override {
        std::cerr << "MQTT connection lost: " << cause << std::endl;
    }
};

int main() {
    mqtt::async_client client(MQTTSERVERADDRESS, MQTTCLIENTID);
    Callback cb;
    client.set_callback(cb);

    mqtt::connect_options connOpts;
    try {
        client.connect(connOpts)->wait();
        client.subscribe(MQTTTOPIC, 1)->wait();

        std::cout << "Subscribed to topic: " << MQTTTOPIC << ", waiting for messages..." << std::endl;

        // Loop indefinitely to keep program alive and processing messages
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        client.disconnect()->wait();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
