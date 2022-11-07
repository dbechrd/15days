#pragma once
#include "SDL/SDL_scancode.h"
#include <vector>
#include <unordered_map>

//-----------------------------------------------------------------------------
// Basic Types
//-----------------------------------------------------------------------------

typedef uint32_t UID;

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;

    bool IsZero() { return x == 0 && y == 0 && z == 0; }
    bool IsTiny(float epsilon = 0.0001f) {
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
    UID uid;
    FacetType type;
};

// TODO: If these don't share sensible names across different entities, we
// should probably just use a dynamic list of string -> Vec2
enum AttachPoint {
    AttachPoint_Gut,
    AttachPoint_Count
};
struct Attach {
    Facet facet;
    Vec2 points[AttachPoint_Count];
};

// Input plan:
// - UI elements process input in topmost-first order
// - If controller input goes unprocessed, it populates its InputWorld facet

struct InputState {
    bool   active;      // input is currently active
    bool   activePrev;  // input was active last frame
    double changed_at;  // time of last state change in milliseconds
    bool   handled;     // true if already handled by something this frame

    void NewFrame(void) {
        activePrev = active;
        handled = false;
    }
    void Trigger(void) {
        active = true;
    }
    void Release(void) {
        active = false;
    }
    void Update(double now) {
        if (Changed()) {
            changed_at = now;
        }
    }
    inline bool Changed() { return active != activePrev; }
    inline bool Triggered() { return active && Changed(); }
    inline bool Released() { return !active && Changed(); }
};

// NOTE(dlb): Bit of a clever trick here to add some custom scancodes that
// represent mouse input to the SDL scancode enum so that we can treat mouse
// buttons the same as any other kind of key.
enum {
    FDOV_SCANCODE_MOUSE_LEFT = SDL_NUM_SCANCODES,
    FDOV_SCANCODE_MOUSE_RIGHT,
    FDOV_SCANCODE_MOUSE_MIDDLE,
    FDOV_SCANCODE_MOUSE_X1,
    FDOV_SCANCODE_MOUSE_X2,
    InputButton_Count
};
struct InputButtons {
    Facet facet;
    InputState buttons[InputButton_Count];

    void NewFrame(void) {
        for (int i = 0; i < InputButton_Count; i++) {
            buttons[i].NewFrame();
        }
    }
    void Trigger(int scancode) {
        buttons[scancode].Trigger();
    }
    void Release(int scancode) {
        buttons[scancode].Release();
    }
    void Update(double now) {
        for (int i = 0; i < InputButton_Count; i++) {
            buttons[i].Update(now);
        }
    }
    bool Triggered(int scancode) {
        return buttons[scancode].Triggered();
    }
    bool Active(int scancode) {
        return buttons[scancode].active;
    }
    bool Released(int scancode) {
        return buttons[scancode].Released();
    }
};

// TODO(dlb): Make a ControllerSystem that processes raw InputButton states
// using a KeyMap and converts them into commands based on the game state.
enum InputCommandType {
    InputCommand_QuitRequested,
    InputCommand_MoveUp,
    InputCommand_MoveLeft,
    InputCommand_MoveDown,
    InputCommand_MoveRight,
    InputCommand_Primary,
    InputCommand_Secondary,
    InputCommand_Count
};
struct InputCommands {
    Facet facet;
    InputState commands[InputCommand_Count];

    void NewFrame(void) {
        for (int i = 0; i < InputCommand_Count; i++) {
            commands[i].NewFrame();
            commands[i].Release();
        }
    }
    void Trigger(InputCommandType command) {
        commands[command].Trigger();
    }
    void Release(InputCommandType command) {
        commands[command].Release();
    }
    void Update(double now) {
        for (int i = 0; i < InputCommand_Count; i++) {
            commands[i].Update(now);
        }
    }
    bool Triggered(InputCommandType command) {
        return commands[command].Triggered();
    }
    bool Active(InputCommandType command) {
        return commands[command].active;
    }
    bool Released(InputCommandType command) {
        return commands[command].Released();
    }
};

// Location of mouse cursor in screen coords (and world coords if relevant)
struct InputCursor {
    Facet facet;
    Vec2 screen;       // position of cursor in screen coordinates

    // TODO: CameraSystem should probably populate this?
    bool worldActive;  // if false, world position is undefined
    Vec2 world;        // position of cursor in world coordinates
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
struct InputHotkey {
    InputCommandType command;
    int    keys[3];    // keys that make up the hotkey (or -1 if unused)
    bool   onPress;    // trigger when all keys first pressed
    bool   onHold;     // trigget when all keys held down
    bool   onRelease;  // trigger when all keys released after being active
    bool   active;     // remember whether all keys were down last frame to enabled release triggers
    bool   changed;    // state changed since last frame
    double changed_at; // time of last state change in milliseconds
};
struct InputKeymap {
    Facet facet;
    std::vector<InputHotkey> hotkeys[GameState_Count];
};

struct PhysicsBody {
    Facet facet;
    float friction;
    float gravity;
};

struct Position {
    Facet facet;
    Vec3 position;

    bool OnGround() { return position.z == 0; }
};

struct Sprite {
    Facet facet;
    float scale;
};

struct Velocity {
    Facet facet;
    Vec3 velocity;

    bool Moving() { return !velocity.IsZero(); }
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
    std::vector<InputCommands> inputCommands {};  // could be useful for multiplayer (net or local)
    std::vector<InputCursor>   inputCursor   {};  // could be useful for multiplayer (net or local)
    std::vector<InputKeymap>   inputKeymap   {};  // not a facet?
    std::vector<PhysicsBody>   body          {};
    std::vector<Position>      position      {};
    std::vector<Sprite>        sprite        {};
    std::vector<Velocity>      velocity      {};
};

//-----------------------------------------------------------------------------
