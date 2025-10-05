#include "pch.h"
#include "GCPStorageClient.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback function for CURL to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

GCPStorageClient::GCPStorageClient(const std::string& projectId, const std::string& bucketName, const std::string& keyFilePath)
    : projectId(projectId), bucketName(bucketName), keyFilePath(keyFilePath) {
    getAccessToken();
}

bool GCPStorageClient::getAccessToken() {
    
    accessToken = getEnvVarSafe("GOOGLE_ACCESS_TOKEN");
    if (!accessToken.empty()) {
        return true;
    }

    std::cerr << "Access token not found. Set GOOGLE_ACCESS_TOKEN environment variable." << std::endl;
    return false;
}

bool GCPStorageClient::uploadFile(const std::string& localFilePath, const std::string& objectName) {
    // Read file content
    std::ifstream file(localFilePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << localFilePath << std::endl;
        return false;
    }

    std::string fileContent((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    return uploadTimeSeriesData(fileContent, objectName);
}

bool GCPStorageClient::uploadTimeSeriesData(const std::string& jsonData, const std::string& objectName) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL for GCP Storage" << std::endl;
        return false;
    }

    std::string url = "https://storage.googleapis.com/upload/storage/v1/b/" + bucketName + "/o?uploadType=media&name=" + objectName;
    std::string response;

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + accessToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || responseCode != 200) {
        std::cerr << "GCP Storage upload failed. Code: " << responseCode << std::endl;
        return false;
    }

    std::cout << "Successfully uploaded to GCP Storage: " << objectName << std::endl;
    return true;
}