// timer.cpp

#include "timer.h"
#include "raylib.h"
#include <cassert>
#include "constants_shared.h"


Timer::Timer(float inDuration)
{
    duration = inDuration;
    Restart();
}

void Timer::Restart()
{
    start = GetTime();
    target = start + duration;
}

void Timer::Start(float inDuration)
{
	duration = inDuration;
    Restart();
}

void Timer::Stop()
{
    target = GetTime();
}

double Timer::GetDuration() const
{
    return duration;
}

bool Timer::IsFinished() const
{
    return target - GetTime() <= 0;
}

