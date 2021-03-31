#include "Chronograph.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <utility>
#include <tuple>
#include <assert.h>

#include <intrin.h>
#pragma intrinsic(__rdtsc)

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

unsigned Chronograph::_runningChronographsCount = 0;
static std::ofstream outputFileStream;
static std::ostream* outputStream = &std::cout;

Chronograph::~Chronograph()
{// This will allow not to have remember to call log(), when getting out of a scope.
    //TODO: If getting out of scope without logging for a certain action, send a warning.
    log();
    --_runningChronographsCount;
}

Chronograph::Chronograph(const std::string&& measuredTimeAction, const bool isLoggingEnabled) : _isLoggingEnabled(isLoggingEnabled)
{
    ++_runningChronographsCount;
    start(measuredTimeAction);
}

void Chronograph::setLoggingEnabled(bool isEnabled)
{
    _isLoggingEnabled = isEnabled;
}

void Chronograph::setOutputFile(std::string filePath)
{
	if (outputFileStream.is_open())
    {
		outputFileStream.close();
    }

	outputFileStream.open(filePath);
	if (!outputFileStream.is_open())
    {
        std::cout << " ### ERROR Chronograph:cannot open: " << filePath << std::endl;
        exit(-1);
    }
	outputStream = &outputFileStream;
	std::cout << " ### INFO Chronograph: output file set: " << filePath << std::endl;
}

Chronograph::TimePoint Chronograph::now() noexcept
{
//    unsigned lo, hi;
//	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return TimePoint(duration(__rdtsc()));
}

void Chronograph::start(const std::string& measuredTimeAction)
{
    if (!_isLoggingEnabled)
        return;

    ++_nesting;
    microseconds msStart = std::chrono::duration_cast<microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    _measuredTimeActions.push_back({measuredTimeAction, Chronograph::now(), msStart});
	*outputStream << " ### Chronograph::start() for action: ";
    for (unsigned i = 0; i < _nesting + _runningChronographsCount -1; i++)
    {
		*outputStream << "    ";
    }
	*outputStream << measuredTimeAction << std::endl;
}

void Chronograph::log(const std::string& measuredTimeAction)
{
    if (!_isLoggingEnabled || _measuredTimeActions.size() == 0)
        return;

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

        // If no such, ignore.
        if (revIt == _measuredTimeActions.rend())
            return;

        _measuredTimeActions.erase(std::next(revIt).base());
    }

    // Get the clock ticks since restart for given action.
    auto ticks = Chronograph::Cycle(t1 - t0);
	*outputStream << "     Chronograph::log(), time for:    ";
    for (unsigned i = 0; i < _nesting + _runningChronographsCount -1; i++)
    {
		*outputStream << "    ";
    }
	*outputStream << measuredTimeActionFetched << " # ms/ms(Chrono)/Mticks: "
               << double((msT1-msT0).count())/1000 << " / "
               << std::chrono::duration_cast<milliseconds>(ticks).count() << " / " << ticks.count()/1000000
               << std::endl;

    --_nesting;
}
