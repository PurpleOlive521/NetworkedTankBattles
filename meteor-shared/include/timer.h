#pragma once


struct Timer 
{
    Timer() = default;
    Timer(float inDuration);

    void Restart();
    void Start(float inDuration);
    void Stop();
    double GetDuration() const;

    bool IsFinished() const;

    double target = 0;
    double start = 0;
    float duration = 0;
};
