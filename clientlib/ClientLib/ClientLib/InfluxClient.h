#ifndef INFLUXCLIENT_H
#define INFLUXCLIENT_H

#include <string>

class InfluxClient {
public:
    InfluxClient(const std::string& url, const std::string& token,
        const std::string& org, const std::string& bucket);
    ~InfluxClient();

    bool WritePoint(const std::string& sensor, double temperature,
        double humidity, int64_t timestamp);

private:
    std::string url_;
    std::string token_;
    std::string org_;
    std::string bucket_;
};

#endif // INFLUXCLIENT_H
