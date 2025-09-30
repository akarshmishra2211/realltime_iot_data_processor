#include "pch.h"
#include "PubSubClient.h"

#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#pragma comment(lib, "ws2_32.lib")

static std::string Base64Encode(const std::string& in) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

PubSubClient::PubSubClient(const std::string& project_id,
    const std::string& topic_id,
    const std::string& token)
    : project_id_(project_id), topic_id_(topic_id), token_(token)
{
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cerr << "WSAStartup failed: " << res << std::endl;
        // Handle error as appropriate
    }
}

bool PubSubClient::PublishMessage(const std::string& message) {
    json payload = {
        {"messages", {
            {{"data", Base64Encode(message)}}
        }}
    };

    std::string url = "https://pubsub.googleapis.com/v1/projects/" + project_id_ + "/topics/" + topic_id_ + ":publish";

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string body = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    return true;
}
