#pragma once
#include <string>

class NotificationObserver {
public:
    virtual ~NotificationObserver() {}
    virtual void OnNotify(const std::string& message) = 0;
};
