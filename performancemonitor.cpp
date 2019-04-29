#include "performancemonitor.h"

#include <algorithm>
#include <numeric>
#include <cassert>
#include <climits>
#include <limits>

using namespace std;
using namespace std::chrono;

PerformanceMonitor::PerformanceMonitor()
{
    m_countUsedTimes = 10;
    m_currentCommonTime.first = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    m_currentCommonTime.second = m_currentCommonTime.first;
}

size_t PerformanceMonitor::countUsedTimes() const
{
    return m_countUsedTimes;
}

void PerformanceMonitor::setCountUsedTimes(const size_t & countUsedTimes)
{
    m_countUsedTimes = countUsedTimes;
}

size_t PerformanceMonitor::countTimers() const
{
    return m_timers.size();
}

PerformanceMonitor::Timer PerformanceMonitor::timer(size_t index) const
{
    return m_timers[index];
}

milliseconds PerformanceMonitor::commonTime() const
{
    if (m_commonTimes.empty())
        return m_currentCommonTime.second - m_currentCommonTime.first;
    return accumulate(m_commonTimes.begin(), m_commonTimes.end(), milliseconds(0)) /
                      m_commonTimes.size();
}

void PerformanceMonitor::startTimer(const string & name)
{
    auto it = m_running_timers.find(name);
    if (it == m_running_timers.end())
    {
        m_running_timers.insert(pair<string, TimerInfo>(
                                    name,
                                    { vector<milliseconds>(),
                                      duration_cast<milliseconds>(system_clock::now().time_since_epoch()),
                                      m_currentIndex }));
    }
    else
    {
        it->second.last_start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        it->second.currentIndex = m_currentIndex;
    }
    ++m_currentIndex;
}

void PerformanceMonitor::endTimer(const string & name)
{
    auto it = m_running_timers.find(name);
    if (it != m_running_timers.end())
    {
        if (it->second.durations.size() > m_countUsedTimes)
        {
            it->second.durations.erase(it->second.durations.begin(),
                                       it->second.durations.begin() +
                                         ((it->second.durations.size() + 1) - m_countUsedTimes));
        }
        it->second.durations.push_back(duration_cast<milliseconds>(system_clock::now().time_since_epoch()) -
                                       it->second.last_start);
    }
    else
    {
        assert(false);
    }
}

void PerformanceMonitor::start()
{
    m_currentIndex = 0;
    m_timers.resize(0);
    for (auto it = m_running_timers.begin(); it != m_running_timers.end(); ++it)
    {
        it->second.currentIndex = numeric_limits<std::size_t>::max();
    }
    m_currentCommonTime.first = duration_cast<milliseconds>(
                system_clock::now().time_since_epoch());
}

void PerformanceMonitor::end()
{
    m_currentCommonTime.second = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    if (m_commonTimes.size() >= m_countUsedTimes)
    {
        m_commonTimes.erase(m_commonTimes.begin(),
                            m_commonTimes.begin() + ((m_commonTimes.size() + 1) - m_countUsedTimes));
    }
    m_commonTimes.push_back(m_currentCommonTime.second - m_currentCommonTime.first);
    vector<Timer> timers;
    vector<pair<size_t, size_t>> indices;
    for (auto it = m_running_timers.begin(); it != m_running_timers.end(); )
    {
        if (it->second.currentIndex == numeric_limits<size_t>::max())
        {
            it = m_running_timers.erase(it);
        }
        else
        {
            indices.push_back({ timers.size(), it->second.currentIndex });
            timers.push_back({ it->first, static_cast<size_t>(it->second.getAvgDuration().count()) });
            ++it;
        }
    }
    sort(indices.begin(), indices.end(), [] (const pair<size_t, size_t> & a,
                                             const pair<size_t, size_t> & b) {
        return (a.second < b.second);
    });
    m_timers.resize(timers.size());
    for (size_t i = 0; i < timers.size(); ++i)
    {
        m_timers[i] = timers[indices[i].first];
    }
}

string PerformanceMonitor::report() const
{
    string str = "Common time: " + to_string(commonTime().count()) + "\n";
    for (const Timer & timer : m_timers)
        str += "\r" + timer.name + " : " + to_string(timer.duration);
    return str;
}
