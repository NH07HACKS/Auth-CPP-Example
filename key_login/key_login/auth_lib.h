// auth_lib.h
#pragma once

#include <string>
#include <functional>
#include <windows.h>
namespace AuthLib {
    // Callback type for authentication results
    using AuthCallback = std::function<void(bool success, const std::string& message)>;

    // Initialize the library (call once at startup)
    void Initialize();

    // Authenticate with a license key
    void Authenticate(const std::string& key, AuthCallback callback);

    // Check if current session is authenticated
    bool IsAuthenticated();

    // Get authentication error message
    std::string GetAuthMessage();

    // Get remaining days (if subscription based)
    int GetRemainingDays();
}