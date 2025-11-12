#pragma once

#include <functional>
#include <unordered_map>

typedef std::function<void()> TimerDelegate;

// Returned when querying a non-existent timer for it's duration or elapsed time.
constexpr float INVALID_TIMER = -1.0f;

// Handle to a specific timer. Always generated when starting a timer, but not guaranteed to still be active.
struct TimerHandle
{
    TimerHandle();

    static int GenerateId();

    static int currentId;

    int id = 0;

    friend auto operator <=>(const TimerHandle& lhs, const TimerHandle& rhs) = default;

    bool operator==(const TimerHandle& other) const = default;
};

// For whatever wonderful reason, this needs to be placed EXACTLY here to work.
// We need to define a hasher for timer_handle in order to use it in an unordered map
namespace std
{
    template<> struct hash<TimerHandle>
    {
        size_t operator()(const TimerHandle& handle) const noexcept
        {
            return std::hash<int>()(handle.id);
        }
    };
}


struct DelegatedTimer
{
    DelegatedTimer() = delete;
    DelegatedTimer(TimerDelegate& callback, float duration);

    void Update(float deltaTime);

    bool IsFinished() const;

    void FireCallback();

	// Can return a negative value if past the duration.
    float GetTimeRemaining() const;

    TimerDelegate callback;

    float duration = 0.0f;

    float elapsedTime = 0.0f;
};

// Manages multiple delegated timers. The timers are updated per tick, and ensured to be called in the order they expired in, even when finishing in-between ticks.
class TimeManager
{

public:
    TimeManager() = default;

    // Increments all present timers and removes expired ones.
    void Update(float deltaTime);

    // Toggles ticking the active timers.
    void ToggleTicking(bool bInShouldTick);

    // Returns the started timers handle.
    TimerHandle StartTimer(TimerDelegate& callback, float duration);

    // Returns true if the timer was found and stopped.
    bool StopTimer(const TimerHandle& handle);

	// Returns the elapsed time of the timer, or INVALID_TIMER if not found.
    float GetTimerElapsed(const TimerHandle& handle) const;

    // Sets the elapsed time of the timer to inElapsedTime.
    void SetTimerElapsed(const TimerHandle& handle, float inElapsedTime);

private:
    typedef std::unordered_map<TimerHandle, DelegatedTimer>::iterator TimerPairIterator;

    std::unordered_map<TimerHandle, DelegatedTimer> timers;

    bool bIsTicking = true;
};

