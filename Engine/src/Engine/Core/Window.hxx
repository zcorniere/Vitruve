#pragma once

#include "Engine/Core/Events/Events.hxx"

struct GLFWwindow;

/// @brief Define a Window
struct FWindowDefinition
{
    using EventHandler = std::function<void(FEvent&)>;

    /// The X position on the screen
    float XPositionOnScreen = 0;
    /// The Y position on the screen
    float YPositionOnScreen = 0;

    /// Width of the window
    /// @note The window could end up smaller if it does not fit on screen
    float WidthDesiredOnScreen = 500;
    /// Height of the window
    /// @note The window could end up smaller if it does not fit on screen
    float HeightDesiredOnScreen = 500;

    /// Should the window as OS border ?
    bool HasOsWindowBorder = true;
    /// Should the window appears in the Taskbar ?
    bool AppearsInTaskbar = true;
    /// Should the window accept input ?
    bool AcceptInput = true;
    /// Should the window be resizable ?
    bool HasSizingFrame = true;

    /// The title of the window
    std::string Title = __FILE__;
    /// Should the resize preserve aspect ratio
    bool ShouldPreserveAspectRatio = true;

    EventHandler EventCallback;
};

/// @brief A holder for GLFW to make sure it stays initialized
struct ENGINE_API GLFWHolder
{
    GLFWHolder();
    ~GLFWHolder();
};

/// @class Window
///
/// @brief A class allowing some abstraction over the GLFW library
class RWindow : public RObject
{
    RTTI_DECLARE_TYPEINFO(RWindow, RObject);

public:
    /// Make sure GLFW is initialized or do it if it is not
    ENGINE_API static bool EnsureGLFWInit();

public:
    /// Default ctor
    RWindow();
    /// Default dtor
    virtual ~RWindow();

    /// @brief Open the window
    /// @param InDefinition The definition of the window
    ENGINE_API void Initialize(const FWindowDefinition& InDefinition);

    /// Reshape the window
    ENGINE_API void ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height);
    /// Move the window
    ENGINE_API void MoveWindow(int32 X, int32 Y);
    /// Bring the window to the front
    ENGINE_API void BringToFront(bool bForce = false);

    /// Destroy the window
    ENGINE_API void Destroy();
    /// Minimize the window
    ENGINE_API void Minimize();
    /// Maximize the window
    ENGINE_API void Maximize();
    /// Restore the window
    ENGINE_API void Restore();

    /// @brief Show the window
    ENGINE_API void Show();
    /// @brief Hide the window
    ENGINE_API void Hide();

    ENGINE_API bool IsMaximized() const;
    ENGINE_API bool IsMinimized() const;
    ENGINE_API bool IsVisible() const;

    ENGINE_API void AcceptInput(bool bEnable);
    ENGINE_API int32 GetWindowBorderSize() const;
    ENGINE_API int32 GetWindowTitleBarSize() const;

    ENGINE_API void SetText(const std::string_view Text);
    ENGINE_API void DrawAttention();

    ENGINE_API GLFWwindow* GetHandle() const;

    const FWindowDefinition& GetDefinition() const
    {
        return Definition;
    }

    void ProcessEvents();

private:
    static ENGINE_API bool InitializeGLFW();
    static ENGINE_API std::atomic_bool bGLFWInitialized;
    static ENGINE_API std::atomic_short GLFWInUseCount;

    void SetupGLFWCallbacks();

private:
    GLFWHolder holder;
    FWindowDefinition Definition;
    GLFWwindow* p_Handle;

    bool bIsVisible;

    friend struct GLFWHolder;
};
