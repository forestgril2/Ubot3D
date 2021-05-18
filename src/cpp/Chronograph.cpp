#include "Chronograph.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <utility>
#include <tuple>
#include <assert.h>

#include <stdint.h>
#include <intrin.h>

static const unsigned logLineWidth = 30;

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

std::map<std::string, double> Chronograph::_totalActionTimes = {};
unsigned Chronograph::_runningChronographsCount = 0;
static std::ofstream outputFile;

Chronograph::~Chronograph()
{// This will allow not to have remember to call log(), when getting out of a scope.
#ifdef CHRONOGRAPH_TIME_LOGGING
    //TODO: If getting out of scope without logging for a certain action, send a warning.
    log();
    --_runningChronographsCount;
    if (0 == _runningChronographsCount)
    {
        Chronograph::dumpTotalTimeActions();
    }
#endif
}

Chronograph::Chronograph(const std::string&& measuredTimeAction, const bool isLoggingEnabled)
#ifdef CHRONOGRAPH_TIME_LOGGING
    : _isLoggingEnabled(isLoggingEnabled && outputFile.is_open())
#endif
{
#ifdef CHRONOGRAPH_TIME_LOGGING
    ++_runningChronographsCount;
    start(measuredTimeAction);
#endif
}

void Chronograph::setLoggingEnabled(bool isEnabled)
{
    _isLoggingEnabled = isEnabled;
}

void Chronograph::setLoggingDetailedOutput(bool isOutput)
{
    _isLoggingDetailedOutput = isOutput;
}

void Chronograph::setOutputFile(std::string filePath)
{
    if (outputFile.is_open())
    {
        outputFile.close();
    }

    outputFile.open(filePath);
    if (!outputFile.is_open())
    {
        std::cout << " ### ERROR Chronograph:cannot open: " << filePath << std::endl;
        exit(-1);
    }
    std::cout << " ### INFO Chronograph: output file set: " << filePath << std::endl;
}

Chronograph::TimePoint Chronograph::now() noexcept
{
//    unsigned lo, hi;
//    asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
//    return TimePoint(duration(static_cast<rep>(hi) << 32 | lo));
    return TimePoint(duration(__rdtsc()));
}

template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
    return std::pair<B,A>(p.second, p.first);
}

template<typename A, typename B, template<class,class,class...> class M, class... Args>
std::multimap<B,A> flip_map(const M<A,B,Args...> &src)
{
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(),
                   std::inserter(dst, dst.begin()),
                   flip_pair<A,B>);
    return dst;
}

void Chronograph::dumpTotalTimeActions()
{
    // TODO: Realize it with bracket syntax, so that nested actions get printed as nested.
    outputFile << " ### " << std::endl
               << " ### Chronograph::" <<  __FUNCTION__ << std::endl
               << " ### " << std::endl;

    const auto flippedTotalActionTimes = flip_map(_totalActionTimes);
    if (flippedTotalActionTimes.size() == 0)
        return;

    const double longestTime = (*flippedTotalActionTimes.rbegin()).first;
    for(auto backIt = flippedTotalActionTimes.rbegin(); backIt != flippedTotalActionTimes.rend(); ++backIt)
    {
        outputFile << std::setw(6) << std::setprecision(3)
                   << "   " << ((*backIt).first)*100/longestTime << "%"
                   << "   " << std::setprecision(5) << (*backIt).first << " [ms]: "
                   << (*backIt).second <<  std::endl;
    }
}

void Chronograph::start(const std::string& measuredTimeAction)
{
#ifdef CHRONOGRAPH_TIME_LOGGING
    if (!_isLoggingEnabled)
        return;

    ++_nesting;

    std::ostringstream out;
    if (_isLoggingDetailedOutput)
    {
        out << " ### Chronograph::" << __FUNCTION__ << "() for action:";
        // TODO: sort this out - how to align this elegangly.
        outputFile << std::setw(logLineWidth) << out.str();
        for (unsigned i = 0; i < _nesting + _runningChronographsCount -1; i++)
        {
            outputFile << "    ";
        }
        outputFile << measuredTimeAction << std::endl;
    }

    // Start timer after text streaming.
    microseconds msStart = std::chrono::duration_cast<microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    _measuredTimeActions.push_back({measuredTimeAction, Chronograph::now(), msStart});
#endif
}

void Chronograph::log(const std::string& measuredTimeAction)
{
#ifdef CHRONOGRAPH_TIME_LOGGING
    if (!_isLoggingEnabled || _measuredTimeActions.size() == 0)
        return;

    // End timer before text streaming.
    const TimePoint t1 = Chronograph::now();
    const microseconds msT1 = std::chrono::duration_cast<microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    TimePoint t0;
    microseconds msT0;
    std::string measuredTimeActionFetched;

    if (measuredTimeAction.empty())
    {// Get the last action time point.
        measuredTimeActionFetched.swap(std::get<0>(_measuredTimeActions.back()));
        t0 = std::get<1>(_measuredTimeActions.back());
        msT0 = std::get<2>(_measuredTimeActions.back());
        _measuredTimeActions.pop_back();
    }
    else
    {// Look for the last matching string (serving as action "bracket" marker).
        auto revIt = _measuredTimeActions.rbegin();
        for ( ; revIt != _measuredTimeActions.rend(); ++revIt)
        {
            if (std::get<0>(*revIt) == measuredTimeAction)
            {
                measuredTimeActionFetched.swap(std::get<0>(*revIt));
                t0 = std::get<1>(*revIt);
                msT0 = std::get<2>(*revIt);
                break;
            }
        }

        // If no such, post a warning to log file and std::cout.
        if (revIt == _measuredTimeActions.rend())
        {
            const std::string warning = " ### WARNING Chronograph action: " + measuredTimeAction + " has not been found. ";
            std::cout << warning << std::endl;
            outputFile << warning << std::endl;
            return;
        }

        _measuredTimeActions.erase(std::next(revIt).base());
    }


    // Get the clock ticks since restart for given action.
    auto ticks = Chronograph::Cycle(t1 - t0);
    std::ostringstream out;
    if (_isLoggingDetailedOutput)
    {
        out << " ### Chronograph::" << __FUNCTION__ << "() for action:  ";
        // TODO: sort this out - how to align this elegangly.
        outputFile << std::setw(logLineWidth) << out.str();
        for (unsigned i = 0; i < _nesting + _runningChronographsCount -1; i++)
        {
            outputFile << "    ";
            outputFile << measuredTimeActionFetched << " # ms/ms(Chrono)/Mticks: "
                       << double((msT1-msT0).count())/1000 << " / "
                       << std::chrono::duration_cast<milliseconds>(ticks).count() << " / " << ticks.count()/1000000
                       << std::endl;
        }
    }

    _totalActionTimes[measuredTimeActionFetched] += double((msT1-msT0).count())/1000;

    --_nesting;
#endif
}
