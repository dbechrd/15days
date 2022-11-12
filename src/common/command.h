#pragma once
#include <vector>

enum CommandType {
    Command_None,
    Command_QuitRequested,
    //Command_MoveUp,
    //Command_MoveLeft,
    //Command_MoveDown,
    //Command_MoveRight,
    Command_Primary,
    //Command_Secondary,
    Command_Count
};

typedef std::vector<CommandType> CommandQueue;