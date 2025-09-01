# Auth-CPP-Example

A C++ authentication library example (`AuthLib`) for license key verification, session management, and subscription checking on Windows.

---

## Features

- Initialize authentication system (`Initialize()`)
- Authenticate with a license key (`Authenticate(key, callback)`)
- Callback-based async authentication results (`AuthCallback`)
- Check if the current session is authenticated (`IsAuthenticated()`)
- Retrieve the latest authentication message (`GetAuthMessage()`)
- Get remaining subscription days (`GetRemainingDays()`)

---

## AuthLib API

```cpp
namespace AuthLib {

    // Callback type for authentication results
    using AuthCallback = std::function<void(bool success, const std::string& message)>;

    // Initialize the library (call once at startup)
    void Initialize();

    // Authenticate with a license key
    void Authenticate(const std::string& key, AuthCallback callback);

    // Check if current session is authenticated
    bool IsAuthenticated();

    // Get authentication error or status message
    std::string GetAuthMessage();

    // Get remaining days for subscription (if applicable)
    int GetRemainingDays();

}
