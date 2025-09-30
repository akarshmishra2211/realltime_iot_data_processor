#pragma once
#include <string>

class PubSubClient {
    std::string project_id_;
    std::string topic_id_;
    std::string token_;  // OAuth2 Access Token

public:
    PubSubClient(const std::string& project_id,
        const std::string& topic_id,
        const std::string& token);

    bool PublishMessage(const std::string& message);
};


