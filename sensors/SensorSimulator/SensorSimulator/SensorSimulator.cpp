#include <iostream>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>

using json = nlohmann::json;

//const std::string SERVER{ "tcp://localhost:1883" };
const std::string SERVER{ "tcp://host.docker.internal:1883" };

const std::string TOPIC{ "sensors/telemetry" };

int main() {
    try {
        mqtt::async_client client(SERVER, "SensorSimulator");
        client.connect()->wait();

        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> tempDist(20, 30), humDist(30, 60);

        std::cout << "Sensor Simulator started. Press Ctrl+C to stop.\n";

        while (true) {
            json j{
                {"sensor_id", gen() % 10 + 1},
                {"temperature", std::round(tempDist(gen) * 100) / 100},
                {"humidity", std::round(humDist(gen) * 100) / 100},
                {"timestamp", static_cast<int64_t>(std::time(nullptr)) * 1000}
            };

            client.publish(TOPIC, j.dump(), 1, false);
            std::cout << "Published: " << j.dump() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }

        client.disconnect()->wait();
    }
    catch (const mqtt::exception& e) {
        std::cerr << "MQTT error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
