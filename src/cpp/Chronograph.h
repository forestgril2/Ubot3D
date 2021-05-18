#pragma once

#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <utility>
#include <tuple>
#include <assert.h>

#define CHRONOGRAPH_TIME_LOGGING true

struct Chronograph
{
    using rep = unsigned long long;
    using period =  std::ratio<1, 190000000u>; // My machine is 1.6-1.9 GHz

    // Define real time units
    using picoseconds = std::chrono::duration<unsigned long long, std::pico>;
    using nanoseconds = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds =  std::chrono::milliseconds;
    using duration = std::chrono::duration<rep, period>;
    using Cycle = std::chrono::duration<double, period>;
    // Define double-based unit of clock tick
    using TimePoint = std::chrono::time_point<Chronograph>;
    using ActionTime = std::tuple<std::string, TimePoint, microseconds>;
    static const bool isSteady = true;

    Chronograph(const std::string&& measuredTimeAction, const bool isLoggingEnabled = true);
    ~Chronograph();

	static void setOutputFile(std::string filePath);
    static TimePoint now() noexcept;
    static void dumpTotalTimeActions();

    void setLoggingEnabled(bool isEnabled);
    void setLoggingDetailedOutput(bool isOutput);
    void start(const std::string& measuredTimeAction);
    void log(const std::string& measuredTimeAction = std::string());

private:
    std::list<ActionTime> _measuredTimeActions;
    static std::map<std::string, double> _totalActionTimes;
    bool _isLoggingEnabled = false;
    bool _isLoggingDetailedOutput = false;
    unsigned _nesting = 0;
    static unsigned _runningChronographsCount;
};
