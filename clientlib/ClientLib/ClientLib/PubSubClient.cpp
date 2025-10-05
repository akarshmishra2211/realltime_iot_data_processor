#include "pch.h"
#include "PubSubClient.h"
#include "Utils.h"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

PubSubClient::PubSubClient(const std::string& projectId, const std::string& topicId, const std::string& token)
    : projectId(projectId), topicId(topicId), token(token) {
    // If token is empty, try to get from environment safely
    if (this->token.empty()) {
        this->token = getEnvVarSafe("GOOGLE_ACCESS_TOKEN");
    }
}

bool PubSubClient::PublishMessage(const std::string& message) {
    if (token.empty()) {
        std::cerr << "No access token available for PubSub" << std::endl;
        return false;
    }

    json payload;
    payload["messages"] = json::array();
    json msgObj;
    msgObj["data"] = message;
    payload["messages"].push_back(msgObj);

    std::string url = "https://pubsub.googleapis.com/v1/projects/" + projectId + "/topics/" + topicId + ":publish";

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string body = payload.dump();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || responseCode != 200) {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res)
            << " (HTTP " << responseCode << ")" << std::endl;
        return false;
    }

    return true;
}