#pragma once
#include "SDL/SDL_scancode.h"
#include <vector>
#include <unordered_map>
#include <cassert>

//-----------------------------------------------------------------------------
// Basic Types
//-----------------------------------------------------------------------------

typedef uint32_t UID;

struct Vec2 {
    float x{};
    float y{};
};

struct Vec3 {
    float x{};
    float y{};
    float z{};

    inline bool IsZero(void)
    {
        return x == 0 && y == 0 && z == 0;
    }
    inline bool IsTiny(float epsilon = 0.0001f)
    {
        return fabsf(x) <= epsilon
            && fabsf(y) <= epsilon
            && fabsf(z) <= epsilon;
    }
};

//-----------------------------------------------------------------------------
// Facets
//-----------------------------------------------------------------------------

enum FacetType {
    Facet_Position,
    Facet_Count,
};
struct Facet {
    UID       uid  {};
    FacetType type {};
};

// TODO: If these don't share sensible names across different entities, we
// should probably just use a dynamic list of string -> Vec2
enum AttachPoint {
    AttachPoint_Gut,
    AttachPoint_Count
};
struct Attach {
    Facet facet  {};
    Vec2  points [AttachPoint_Count]{};
};

// Input plan:
// - UI elements process input in topmost-first order
// - If controller input goes unprocessed, it populates its InputWorld facet
struct ButtonState {
    bool   active     {};      // input is currently active
    bool   activePrev {};  // input was active last frame
    double changedAt  {};   // time of last state change in milliseconds
    bool   handled    {};     // true if already handled by something this frame

    inline void BeginFrame(void)
    {
        activePrev = active;
        handled = false;
    }

    inline void Set(bool activeNow, double now)
    {
        if (activeNow != active) {
            activePrev = active;
            active = activeNow;
            changedAt = now;
            handled = false;
        }
    }

    inline bool Triggered(void)
    {
        return active && Changed();
    }
    inline bool Active(bool includeHandled = false)
    {
        return active && (!handled || includeHandled);
    }
    inline bool Released(void)
    {
        return !active && Changed();
    }

private:
    inline bool Changed(void)
    {
        return active != activePrev;
    }
};

// NOTE(dlb): Bit of a clever trick here to add some custom scancodes that
// represent mouse input to the SDL scancode enum so that we can treat mouse
// buttons the same as any other kind of key.
enum {
    // Mouse buttons
    FDOV_SCANCODE_MOUSE_LEFT = SDL_NUM_SCANCODES,
    FDOV_SCANCODE_MOUSE_RIGHT,
    FDOV_SCANCODE_MOUSE_MIDDLE,
    FDOV_SCANCODE_MOUSE_X1,
    FDOV_SCANCODE_MOUSE_X2,
    // Window "X" button (or Alt+F4, etc.)
    FDOV_SCANCODE_QUIT,
    InputButton_Count
};
struct InputButtons {
    Facet       facet   {};
    ButtonState buttons [InputButton_Count]{};
};

// Location of mouse cursor in screen coords (and world coords if relevant)
struct InputCursor {
    Facet facet       {};
    Vec2  screen      {};  // position of cursor in screen coordinates
    // TODO: CameraSystem should probably populate this?
    bool  worldActive {};  // if false, world position is undefined
    Vec2  world       {};  // position of cursor in world coordinates
};

enum GameState {
    GameState_Cutscene,
    GameState_LoadingGame,
    GameState_LoadingLevel,
    GameState_Play,
    GameState_UIMainMenu,
    GameState_UIPlayMenu,
    GameState_UITextInput,
    GameState_UITextInputChat,
    GameState_Count
};

enum CommandType {
    Command_QuitRequested,
    Command_MoveUp,
    Command_MoveLeft,
    Command_MoveDown,
    Command_MoveRight,
    Command_Primary,
    Command_Secondary,
    Command_Count
};

enum HotkeyTriggerFlags {
    HotkeyTriggerFlag_Trigger = 1 << 0,  // trigger when all keys first pressed
    HotkeyTriggerFlag_Active  = 1 << 1,  // trigget when all keys held down
    HotkeyTriggerFlag_Release = 1 << 2,  // trigger when all keys released after being active
};

struct InputHotkey {
    int                keys    [3]{};  // key scancodes that make up the hotkey (0 if unused)
    HotkeyTriggerFlags flags   {};     // when to treat the hotkey as active
    CommandType        command {};     // the command that this hotkey triggers
    ButtonState        state   {};     // tracks hotkey state as if it were a button

    inline InputHotkey(int key0, int key1, int key2, HotkeyTriggerFlags flags, CommandType command) {
        this->keys[0] = key0;
        this->keys[1] = key1;
        this->keys[2] = key2;
        this->flags = flags;
        this->command = command;
    }
};
struct InputKeymap {
    Facet                    facet   {};
    std::vector<InputHotkey> hotkeys [GameState_Count]{};
};

struct PhysicsBody {
    Facet facet    {};
    float friction {};
    float gravity  {};
};

struct Position {
    Facet facet    {};
    Vec3  position {};

    inline bool OnGround(void)
    {
        return position.z == 0;
    }
};

struct Sprite {
    Facet facet {};
    float scale {};
};

struct Velocity {
    Facet facet    {};
    Vec3  velocity {};

    inline bool Moving(void)
    {
        return !velocity.IsZero();
    }
};

//-----------------------------------------------------------------------------
// Facet Depot
//-----------------------------------------------------------------------------

struct Depot {
    // Map of facet types for each active UID
    std::unordered_map<UID, FacetType> typeByUid[Facet_Count]{};

    // List of active UIDs for each facet type
    std::vector<UID> uidsByType[Facet_Count]{};

    // Dense facet data arrays
    std::vector<Attach>        attach        {};
    std::vector<InputButtons>  inputButtons  {};  // could be useful for multiplayer (net or local)
    std::vector<InputCursor>   inputCursor   {};  // could be useful for multiplayer (net or local)
    std::vector<InputKeymap>   inputKeymap   {};  // not a facet?
    std::vector<PhysicsBody>   body          {};
    std::vector<Position>      position      {};
    std::vector<Sprite>        sprite        {};
    std::vector<Velocity>      velocity      {};
};

//-----------------------------------------------------------------------------
