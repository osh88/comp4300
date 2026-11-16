#pragma once

#include <fstream>
#include <chrono>
#include <mutex>

#include "Helpers.h"

//#define PROFILING

#ifdef PROFILING
    #ifdef _WIN32
        #define PROFILE_SCOPE(name) ProfileTimer timer##__LINE__(name)
        #define PROFILE_FUNCTION()  PROFILE_SCOPE(__PRETTY_FUNCTION__)
    #else
        #define PROFILE_SCOPE(name) ProfileTimer timer##__LINE__(name)
        #define PROFILE_FUNCTION()  PROFILE_SCOPE(__PRETTY_FUNCTION__)
    #endif
#else
    #define PROFILE_SCOPE(name)
    #define PROFILE_FUNCTION()
#endif

struct ProfileResult {
    const std::string name = "Default";
    long long start        = 0;
    long long end          = 0;
    size_t threadID        = 0;
};

class Profiler {
    std::string       m_outputFile = "profile.json";
    size_t            m_profileCount = 0;

    std::ofstream     m_outputStream;
    std::vector<char> m_buffer;

    std::mutex        m_lock;

    Profiler();
    void writeHeader();
    void writeFooter();

public:
    static Profiler & Instance();
    ~Profiler();
    void write(const ProfileResult & r);
};

class ProfileTimer {
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> ClockType;

    ClockType     m_startTimepoint;
    ProfileResult m_result;
    bool          m_stopped = false;
    bool          m_write = true;

public:

    ProfileTimer(const std::string & name, bool write = true);

    ~ProfileTimer();

    // this function is used to set the start time
    // it adds 1 time unit if it's the same start time as the previous start time
    // this solves a display issue in chrome://tracing for identical start times
    // this is a 2/10 on the janky fix scale but it has worked for me in practice
    void start();

    void stop();
    void stop2();
};
