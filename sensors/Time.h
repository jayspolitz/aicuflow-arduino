
#pragma once
#ifndef UnixTime_H
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"


inline void initSNTP(const char* ntp1 = "pool.ntp.org",
                     const char* ntp2 = "time.nist.gov",
                     const char* fallbackTZ = "UTC0") {
    WiFi.waitForConnectResult();
    esp_sntp_servermode_dhcp(1);      // allow DHCP to push TZ
    configTime(0, 0, ntp1, ntp2);     // UTC time, TZ handled separately
    setenv("TZ", fallbackTZ, 1);
    tzset();
    delay(200); // allow DHCP TZ to propagate
}

// returns the current local timezone string (env TZ)
inline const char* LocalTimeStr() {
    const char* tz = getenv("TZ");
    return (tz && tz[0]) ? tz : "UTC";
}

// returns current Unix timestamp in milliseconds
inline uint64_t UnixMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    // get microseconds from esp_timer
    int64_t micros_since_boot = esp_timer_get_time(); // microseconds since boot
    static int64_t tv_sec_at_boot = 0;

    // initialize tv_sec_at_boot on first call
    if(tv_sec_at_boot == 0) tv_sec_at_boot = tv.tv_sec - micros_since_boot / 1000000;

    return (tv_sec_at_boot * 1000ULL) + (micros_since_boot / 1000ULL);
}

inline void waitForTimeSync() {
    while (true) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        if (tv.tv_sec > 30000) break; // arbitrary sanity check: after 2020
        delay(200);
    }
}

#endif