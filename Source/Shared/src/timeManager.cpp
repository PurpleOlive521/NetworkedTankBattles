#include "timeManager.h"
#include "raylib.h"
#include <cassert>
#include "constants_shared.h"
#include <algorithm>


TimerHandle::TimerHandle()
{
    id = GenerateId();
}

int TimerHandle::GenerateId()
{
    currentId++;
    return currentId;
}

int TimerHandle::currentId = 0;

DelegatedTimer::DelegatedTimer(TimerDelegate& callback, float duration)
{
    this->callback = callback;
    this->duration = duration;
}

void DelegatedTimer::Update(float deltaTime)
{
    elapsedTime += deltaTime;
}

bool DelegatedTimer::IsFinished() const
{
    return elapsedTime >= duration;
}

void DelegatedTimer::FireCallback()
{
    if (callback)
    {
        callback();
    }
}

float DelegatedTimer::GetTimeRemaining() const
{
    return duration - elapsedTime;
}

void TimeManager::Update(float deltaTime)
{
	std::vector<TimerPairIterator> expiredTimers;

    // Update all timers
    for (TimerPairIterator it = timers.begin(); it != timers.end(); ++it)
    {
        it->second.Update(deltaTime);

        if (it->second.IsFinished())
        {
            expiredTimers.push_back(it);
        }
    }

    // Sort the timers in ascending order, based on when they expired during the last frame.
    std::ranges::sort(expiredTimers, [](const TimerPairIterator& a, const TimerPairIterator& b)
        {
            return a->second.GetTimeRemaining() < b->second.GetTimeRemaining();
		});

    // Fire callbacks and remove all expired timers in the order they expired.
    for (const TimerPairIterator& it : expiredTimers)
    {
        it->second.FireCallback();
        timers.erase(it);
	}
       
}

void TimeManager::ToggleTicking(bool bInShouldTick)
{
    if (bIsTicking == bInShouldTick)
    {
        return;
    }

    bIsTicking = bInShouldTick;
}

TimerHandle TimeManager::StartTimer(TimerDelegate& callback, float duration)
{
    assert(callback);

    if (duration <= 0.0f)
    {
        debug::warn("TimeManager: Started a timer with a duration of 0 or less.");
    }

    TimerHandle handle;
    timers.insert({ handle, DelegatedTimer(callback, duration) });

    return handle;
}

bool TimeManager::StopTimer(const TimerHandle& handle)
{
    if (timers.find(handle) != timers.end())
    {
        timers.erase(handle);
        return true;
    }

    return false;
}

float TimeManager::GetTimerElapsed(const TimerHandle& handle) const
{
    if (timers.find(handle) != timers.end())
    {
        return timers.at(handle).elapsedTime;
    }

    return INVALID_TIMER;

}

void TimeManager::SetTimerElapsed(const TimerHandle& handle, float inElapsedTime)
{
    if (timers.find(handle) != timers.end())
    {
        timers.at(handle).elapsedTime = inElapsedTime;
    }

}
