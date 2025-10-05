#pragma once
#include <string>
#include <fstream>

class GCPStorageClient {
private:
    std::string projectId;
    std::string bucketName;
    std::string keyFilePath;  // Path to GCP service account JSON key
    std::string accessToken;  // OAuth2 access token

    bool getAccessToken();  // Get OAuth2 token using service account key

public:
    GCPStorageClient(const std::string& projectId, const std::string& bucketName, const std::string& keyFilePath);

    bool uploadFile(const std::string& localFilePath, const std::string& objectName);
    bool uploadTimeSeriesData(const std::string& jsonData, const std::string& objectName);
};

