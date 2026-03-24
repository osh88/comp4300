#include "Profiler.h"
#include "Helpers.h"

#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>

const size_t BUFF_SIZE = 20*1024*1024;

Profiler::Profiler() {
    m_outputStream = std::ofstream(m_outputFile);
    m_buffer = std::vector<char>(BUFF_SIZE);
    m_outputStream.rdbuf()->pubsetbuf(m_buffer.data(), m_buffer.size());
    writeHeader();
}

void Profiler::writeHeader() {
    m_outputStream << "{\"otherData\": {}, \"traceEvents\":[";
}

void Profiler::writeFooter() {
    m_outputStream << "\n]}";
}

Profiler & Profiler::Instance() {
    static Profiler instance;
    instance.m_outputFile = Helpers::getExecutableDirectory() + instance.m_outputFile;
    return instance;
}

Profiler::~Profiler() {
    writeFooter();
    m_outputStream.close();
}

void Profiler::write(const ProfileResult & r) {
    // ProfileTimer t("Profiler::write()", false);

    std::lock_guard<std::mutex> lock(m_lock);

    if (m_profileCount++ > 0) { m_outputStream << ","; }

    std::string name = r.name;
    std::replace(name.begin(), name.end(), '"', '\'');
    
    m_outputStream << "\n{";
    m_outputStream << "\"cat\":\"function\",";
    m_outputStream << "\"dur\":" << (r.end - r.start) << ",";
    m_outputStream << "\"name\":\"" << name << "\",";
    m_outputStream << "\"ph\":\"X\",";
    m_outputStream << "\"pid\":0,";
    m_outputStream << "\"tid\":" << r.threadID << ",";
    m_outputStream << "\"ts\":" << r.start;
    m_outputStream << "}";
}

ProfileTimer::ProfileTimer(const std::string & name, bool write)
    : m_result(ProfileResult{name,0,0,0})
    , m_write(write)
{
    start();
}

ProfileTimer::~ProfileTimer() {
    if (m_write) {
        stop();
    } else {
        stop2();
    }
}

void ProfileTimer::start() {
    // a static variable to store the last start time recorded
    static long long lastStartTime = 0;

    // grab the actual start time using std::chrono
    m_startTimepoint = std::chrono::high_resolution_clock::now();
    m_result.start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
    m_result.threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

    // if it's the same as the previous start time, add 1 to it
    m_result.start += (m_result.start == lastStartTime ? 1 : 0);

    // record the fixed time as the previous start time
    lastStartTime = m_result.start;

    m_stopped = false;
}

void ProfileTimer::stop() {
    if (m_stopped) { return; }
    m_stopped = true;

    auto endTimepoint = std::chrono::high_resolution_clock::now();
    m_result.end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

    Profiler::Instance().write(m_result);
}

void ProfileTimer::stop2() {
    if (m_stopped) { return; }
    m_stopped = true;

    auto endTimepoint = std::chrono::high_resolution_clock::now();
    m_result.end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

    std::cout << m_result.name << " " << m_result.end - m_result.start << std::endl;
}
