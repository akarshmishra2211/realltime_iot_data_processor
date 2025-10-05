#include <iostream>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

using json = nlohmann::json;

// Helper function for local time conversion (thread-safe for MSVC)
std::string EpochToLocalTimeString(int64_t epoch_ns) {
    std::time_t t = static_cast<std::time_t>(epoch_ns / 1000000000);
    std::tm local_tm;
    char buf[32];
    localtime_s(&local_tm, &t);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &local_tm);
    return std::string(buf);
}

// Get current system time in epoch nanoseconds
int64_t GetCurrentEpochNanoseconds() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

const std::string SERVER{ "tcp://localhost:1883" };
const std::string TOPIC{ "sensors/telemetry" };

int main() {
    try {
        mqtt::async_client client(SERVER, "ProcessorClient");
        client.connect()->wait();

        std::cout << "Processor started. Listening for messages...\n";

        client.set_message_callback(
            [&](mqtt::const_message_ptr msg) {
                try {
                    auto j = json::parse(msg->get_payload());
                    int sensorid = j.at("sensorid").get<int>();
                    double temperature = j.at("temperature").get<double>();
                    double humidity = j.at("humidity").get<double>();
                    int64_t timestamp_raw = j.at("timestamp").get<int64_t>();

                    std::string time_str = EpochToLocalTimeString(timestamp_raw);

                    std::cout << "Received: Sensor " << sensorid
                        << ", Temp: " << temperature
                        << ", Humidity: " << humidity
                        << ", Time: " << time_str << std::endl;

                    int64_t insertion_timestamp = GetCurrentEpochNanoseconds();

                    // For demonstration, show readable insertion time:
                    std::string insertion_str = EpochToLocalTimeString(insertion_timestamp);
                    std::cout << "Insertion timestamp as local time: " << insertion_str << std::endl;

                    // If sending, build a message with the insertion timestamp and readable string
                    // json pubsub_msg = {
                    //     {"sensorid", sensorid},
                    //     {"temperature", temperature},
                    //     {"humidity", humidity},
                    //     {"timestamp", insertion_timestamp},
                    //     {"timestring", insertion_str}
                    // };
                    // std::cout << "Sending: " << pubsub_msg.dump() << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Failed to parse/process message: " << e.what() << std::endl;
                }
            }
        );

        client.subscribe(TOPIC, 1)->wait();

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        client.disconnect()->wait();
    }
    catch (const mqtt::exception& e) {
        std::cerr << "MQTT error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
