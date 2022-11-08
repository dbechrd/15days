#pragma once
#include "../facets/depot.h"

struct InputSystem {
public:
    void BeginFrame(Depot &depot, GameState gameState);
    void Enqueue(int scancode, bool isDown);
    void Update(Depot &depot, double now, GameState gameState);

    inline const std::vector<CommandType> CommandQueue(void) const
    {
        return commandQueue;
    }

private:
    struct InputEvent {
        int  scancode {};
        bool down     {};  // true for KEYDOWN, false for KEYUP
    };

    std::vector<InputEvent>  inputQueue   {};
    std::vector<CommandType> commandQueue {};

    void CheckHotkeys(Depot &depot, double now, GameState gameState);
};

extern InputSystem g_inputSystem;