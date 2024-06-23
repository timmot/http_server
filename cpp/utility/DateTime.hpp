// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <string>
#include <time.h>

class DateTime {
public:
    static DateTime now()
    {
        return DateTime(time(NULL));
    }

    DateTime(time_t seconds_since_epoch)
        : m_seconds_since_epoch(seconds_since_epoch)
    {
    }

    std::string to_string()
    {
        // THOUGHT: Could store calendar date (year/month/day, etc) instead of timestamp?
        tm calendar_date_time = {};
        localtime_r(&m_seconds_since_epoch, &calendar_date_time);

        std::string output;
        output += std::to_string(calendar_date_time.tm_year + 1900) + "-";
        output += pad(std::to_string(calendar_date_time.tm_mon + 1)) + "-";
        output += pad(std::to_string(calendar_date_time.tm_mday)) + " ";

        output += pad(std::to_string(calendar_date_time.tm_hour)) + ":";
        output += pad(std::to_string(calendar_date_time.tm_min)) + ":";
        output += pad(std::to_string(calendar_date_time.tm_sec));

        return output;
    }

private:
    std::string pad(std::string to_pad)
    {
        if (to_pad.size() == 1)
            return std::string("0") + to_pad;

        return to_pad;
    }

    time_t m_seconds_since_epoch;
};