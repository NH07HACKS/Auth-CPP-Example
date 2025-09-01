// auth_lib.cpp
#include "auth_lib.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <ctime>
#include <atomic>
#include <mutex>

using json = nlohmann::json;

namespace {
    // Thread-safe variables
    std::atomic<bool> g_authenticated{ false };
    std::string g_auth_message;
    int g_remaining_days = 0;
    std::mutex g_auth_mutex;

    size_t writer(void* contents, size_t size, size_t nmemb, std::string* output) {
        output->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string generateHWID() {
        char computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(computerName);
        if (GetComputerNameA(computerName, &size)) {
            return std::string(computerName);
        }
        return "UnknownHWID";
    }

    int daysLeft(time_t subEndTime) {
        time_t now = time(nullptr);
        double seconds = difftime(subEndTime, now);
        int days = static_cast<int>(seconds / (60 * 60 * 24));
        return (days > 0) ? days : 0;
    }
}

namespace AuthLib {
    void Initialize() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    void Authenticate(const std::string& key, AuthCallback callback) {
        std::thread([key, callback]() {
            CURL* curl = curl_easy_init();
            if (!curl) {
                callback(false, " Failed to initialize CURL");
                return;
            }

            std::string response;
            std::string post_data = "type=auth&key=" + key + "&hwid=" + generateHWID();
            const char* api_url = "http://huy0903.name.vn/api/api.php";

            curl_easy_setopt(curl, CURLOPT_URL, api_url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            std::lock_guard<std::mutex> lock(g_auth_mutex);
            if (res != CURLE_OK) {
                g_auth_message = " CURL error: " + std::string(curl_easy_strerror(res));
                g_authenticated = false;
                callback(false, g_auth_message);
                return;
            }

            if (response.empty()) {
                g_auth_message = " Empty response from server";
                g_authenticated = false;
                callback(false, g_auth_message);
                return;
            }

            try {
                json json_response = json::parse(response);
                std::string status = json_response["Status"];

                if (status == "Authorized" || status == "Activated") {
                    if (json_response.contains("SubEndTime")) {
                        time_t endTimestamp = std::stoll(json_response["SubEndTime"].get<std::string>());
                        g_remaining_days = daysLeft(endTimestamp);
                        g_auth_message = " Authenticated! " + std::to_string(g_remaining_days) + " day(s) left";
                    }
                    else {
                        g_auth_message = " Authenticated successfully!";
                    }
                    g_authenticated = true;
                    callback(true, g_auth_message);
                }
                else {
                    g_authenticated = false;
                    if (status == "WrongKey") {
                        g_auth_message = " Invalid license key!";
                    }
                    else if (status == "WrongHWID") {
                        g_auth_message = " HWID does not match!";
                    }
                    else if (status == "Banned") {
                        g_auth_message = " License key has been banned!";
                    }
                    else if (status == "SubEnded") {
                        g_auth_message = " Subscription has expired!";
                    }
                    else {
                        g_auth_message = " Unknown status: " + status;
                    }
                    callback(false, g_auth_message);
                }
            }
            catch (const std::exception& e) {
                g_auth_message = " JSON error: " + std::string(e.what());
                g_authenticated = false;
                callback(false, g_auth_message);
            }
            }).detach();
    }

    bool IsAuthenticated() {
        return g_authenticated;
    }

    std::string GetAuthMessage() {
        std::lock_guard<std::mutex> lock(g_auth_mutex);
        return g_auth_message;
    }

    int GetRemainingDays() {
        return g_remaining_days;
    }
}