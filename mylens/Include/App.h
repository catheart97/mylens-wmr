#pragma once

#include "pch.h"

/* BUILD CONFIGURATION */
//#define USE_FILE_PICKER
/* */

#include "My/Eye/Eye.h"
#include "My/Eye/Environment.h"
#include "My/Utility/Utility.h"

#include "My/Audio/TTS.h"

namespace My
{

namespace _implementation
{

using namespace winrt;
using namespace winrt::Windows;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Media::SpeechRecognition;
using namespace winrt::Windows::Media::Capture;
using namespace std::placeholders;
using namespace concurrency;

class App : public implements<App, IFrameworkViewSource, IFrameworkView>
{
    // Data //
private:
    bool _window_closed{false};
    bool _scene_initialized{false};

    bool _animate{true};

    std::unique_ptr<Eye::Eye> _renderer;
    std::shared_ptr<Eye::Scene> _scene;

    std::vector<std::function<void()>> _token_release;

    // Windows Mixed Reality API
    HolographicSpace _holographic_space{nullptr};

    // Audio
    std::shared_ptr<MediaCapture> _capture{nullptr};

    // Permission request
    IAsyncOperation<bool> _audio_request;

    std::unique_ptr<My::Audio::TTS> _tts{nullptr};

    // Methods //
public:
    // IFrameworkView methods. (Windows Universal Runtime)
    void Initialize(CoreApplicationView const & applicationView);
    IAsyncAction InitializeScene();
    void SetWindow(CoreWindow const & window);
    void Load(winrt::hstring const & entryPoint);
    void Run();
    void Uninitialize();

    // IFrameworkViewSource method. (Windows Universal Runtime)
    IFrameworkView CreateView();

protected:
    // Permission handling
    void RequestPermissions();
    IAsyncOperation<bool> audio_perimission();

    // Application lifecycle event handlers.
    void OnViewActivated(CoreApplicationView const & sender,
                         Activation::IActivatedEventArgs const & args);
    void OnSuspending(IInspectable const & sender, SuspendingEventArgs const & args);
    void OnResuming(IInspectable const & sender, IInspectable const & args);
    void OnVisibilityChanged(CoreWindow const & sender, VisibilityChangedEventArgs const & args);
    void OnWindowClosed(CoreWindow const & sender, CoreWindowEventArgs const & args);
    void OnKeyPressed(CoreWindow const & sender, KeyEventArgs const & args);
    void OnPointerPressed(CoreWindow const & sender, PointerEventArgs const & args);

    // Holographic Camera Context
    void OnCameraAdded(HolographicSpace const & sender,
                       HolographicSpaceCameraAddedEventArgs const & args);

    void OnCameraRemoved(HolographicSpace const & sender,
                         HolographicSpaceCameraRemovedEventArgs const & args);
};

} // namespace _implementation

using _implementation::App;

} // namespace My
