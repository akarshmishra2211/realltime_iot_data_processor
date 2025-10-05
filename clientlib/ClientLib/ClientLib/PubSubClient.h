#pragma once

#include <string>

class PubSubClient {
private:
    std::string projectId;
    std::string topicId;
    std::string token;

public:
    PubSubClient(const std::string& projectId, const std::string& topicId, const std::string& token);
    
    bool PublishMessage(const std::string& message);
};
