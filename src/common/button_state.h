#pragma once

// Input plan:
// - UI elements process input in topmost-first order
// - If controller input goes unprocessed, it populates its InputWorld facet
struct ButtonState {
    bool   held       {};  // input is currently held
    bool   heldPrev   {};  // input was held last frame
    double changedAt  {};  // time of last state change in milliseconds
    bool   handled    {};  // true if already handled by something this frame

    inline void BeginFrame(void)
    {
        heldPrev = held;
        handled = false;
    }

    inline void Set(bool isDown, double now)
    {
        if (isDown != held) {
            heldPrev = held;
            held = isDown;
            changedAt = now;
            handled = false;
        }
    }

    inline bool Pressed(void)
    {
        return held && Changed();
    }
    inline bool Active(bool includeHandled = false)
    {
        return held && (!handled || includeHandled);
    }
    inline bool Released(void)
    {
        return !held && Changed();
    }

private:
    inline bool Changed(void)
    {
        return held != heldPrev;
    }
};