#pragma once

#include <stdbool.h>

// Countdown timer — starts at a given duration and ticks towards 0.
// "Expired" means the timer reached 0 (or was never started).
typedef struct
{
    float t;
} Timer;

// Set the timer to run for `duration` seconds.
static inline void timer_set(Timer *tm, float duration) { tm->t = duration; }

// Clear the timer (immediately expired).
static inline void timer_clear(Timer *tm) { tm->t = 0.0f; }

// Advance the timer by dt seconds.
static inline void timer_tick(Timer *tm, float dt)
{
    if (tm->t > 0.0f)
        tm->t -= dt;
}

// True while the timer is still counting down.
static inline bool timer_running(const Timer *tm) { return tm->t > 0.0f; }

// True once the timer has reached (or passed) 0.
static inline bool timer_expired(const Timer *tm) { return tm->t <= 0.0f; }
