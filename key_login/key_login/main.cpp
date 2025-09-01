#include <iostream>
#include "auth_lib.h"
#include <thread>
#include <chrono>

using namespace std;
using namespace AuthLib;

int main() {
  
    Initialize();

    cout << "=== Authentication Test Program ===" << endl;

    string licenseKey;
    cout << "Enter license key: ";
    cin >> licenseKey;

  
    Authenticate(licenseKey, [](bool success, const string& message) {
        cout << "\nAuthentication result: " << message << endl;
        if (success) {
            cout << "Remaining days: " << GetRemainingDays() << endl;
        }
        });

  
    string loadingText = "Loading...";
    int index = 0;

    while (!IsAuthenticated() && GetAuthMessage().empty()) {
        cout << "\r" << loadingText.substr(0, index + 1) << flush;
        this_thread::sleep_for(chrono::milliseconds(1));
        index = (index + 1) % loadingText.size();
    }


    cout << "\r" << string(loadingText.size(), ' ') << "\r";

   
    if (!GetAuthMessage().empty() && !IsAuthenticated()) {
        cout << "Error: " << GetAuthMessage() << endl;
    }

    if (IsAuthenticated()) {
        cout << "=== ACCESS GRANTED ===" << endl;

    }
    else {
        cout << "=== ACCESS DENIED ===" << endl;
    }
    system("pause");
    return 0;
}
