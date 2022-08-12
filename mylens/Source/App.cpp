#include "pch.h"

#include "App.h"

void My::App::Initialize(
    winrt::Windows::ApplicationModel::Core::CoreApplicationView const & applicationView)
{
    using namespace winrt::Windows::UI::ViewManagement;

    applicationView.Activated(std::bind(&App::OnViewActivated, this, _1, _2));

    // Register event handlers for app lifecycle and corresponding release token functions
    _token_release.push_back(
        std::bind(static_cast<void (*)(const winrt::event_token &)>(&CoreApplication::Suspending),
                  CoreApplication::Suspending(bind(&App::OnSuspending, this, _1, _2))));

    _token_release.push_back(
        std::bind(static_cast<void (*)(const winrt::event_token &)>(&CoreApplication::Resuming),
                  CoreApplication::Resuming(bind(&App::OnResuming, this, _1, _2))));

    Utility::winrtUtility::LogMessage(L"Initialized App.");
}

void My::App::SetWindow(winrt::Windows::UI::Core::CoreWindow const & window)
{
    using namespace DirectX;

    auto token = window.KeyDown(bind(&App::OnKeyPressed, this, _1, _2));
    _token_release.push_back([=]() { window.KeyDown(token); });

    token = window.PointerPressed(bind(&App::OnPointerPressed, this, _1, _2));
    _token_release.push_back([=]() { window.PointerPressed(token); });

    token = window.Closed(bind(&App::OnWindowClosed, this, _1, _2));
    _token_release.push_back([=]() { window.Closed(token); });

    token = window.VisibilityChanged(bind(&App::OnVisibilityChanged, this, _1, _2));
    _token_release.push_back([=]() { window.VisibilityChanged(token); });

    _holographic_space = HolographicSpace::CreateForCoreWindow(window);

    _renderer = std::make_unique<Eye::Eye>();
    _renderer->holographic_space(_holographic_space);
    _renderer->initialize(); // Init D3D11

    InitializeScene();

    token = _holographic_space.CameraAdded(std::bind(&App::OnCameraAdded, this, _1, _2));
    _token_release.push_back([&]() { _holographic_space.CameraAdded(token); });
    token = _holographic_space.CameraRemoved(std::bind(&App::OnCameraRemoved, this, _1, _2));
    _token_release.push_back([&]() { _holographic_space.CameraRemoved(token); });

    Utility::winrtUtility::LogMessage(L"Window setup complete.");
}

void My::App::Load(winrt::hstring const & entryPoint) {}

winrt::Windows::Foundation::IAsyncAction My::App::InitializeScene()
{
    using namespace DirectX;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Pickers;

#ifdef USE_FILE_PICKER
    FileOpenPicker mtl_picker;
    mtl_picker.ViewMode(PickerViewMode::Thumbnail);
    mtl_picker.SuggestedStartLocation(PickerLocationId::Desktop);
    mtl_picker.FileTypeFilter().ReplaceAll({L".mtl"});
    StorageFile mtl_file = co_await mtl_picker.PickSingleFileAsync();

    FileOpenPicker obj_picker;
    obj_picker.ViewMode(PickerViewMode::Thumbnail);
    obj_picker.SuggestedStartLocation(PickerLocationId::Desktop);
    obj_picker.FileTypeFilter().ReplaceAll({L".obj"});
    StorageFile obj_file = co_await obj_picker.PickSingleFileAsync();
#else
    StorageFile mtl_file{co_await StorageFile::GetFileFromApplicationUriAsync(
        winrt::Windows::Foundation::Uri{L"ms-appx:///Assets/objects/suzanne.mtl"})};
    StorageFile obj_file{co_await StorageFile::GetFileFromApplicationUriAsync(
        winrt::Windows::Foundation::Uri{L"ms-appx:///Assets/objects/suzanne.obj"})};
#endif

    auto materials = co_await Utility::winrtUtility::load_from_mtl(mtl_file, _renderer->device(),
                                                                   _renderer->device_context());

    auto objects = co_await Utility::winrtUtility::load_from_obj(
        obj_file, materials, _renderer->device(), _renderer->device_context());

    auto light = std::make_shared<Eye::Light>();
    float strength = 100.f;
    light->position(XMVectorSet(1, 0, 1, 0));
    light->intensity(strength);

    _scene = std::make_shared<Eye::Scene>();
    _scene->light(light);
    for (auto & object : objects)
    {
        object->scale_x(1.2f);
        object->scale_y(1.2f);
        object->scale_z(1.2f);
        _scene->push_back(object);
    }

    _scene->push_back(std::make_shared<My::Eye::Environment>(_renderer->device()));
    _renderer->scene(_scene);

    _scene_initialized = true;

    Utility::winrtUtility::LogMessage(L"Scene setup complete.");
}

void My::App::Run()
{
    CoreWindow window = CoreWindow::GetForCurrentThread();
    CoreDispatcher dispatcher = window.Dispatcher();

    HolographicFrame previous_frame{nullptr};

     RequestPermissions();
    _tts = std::make_unique<My::Audio::TTS>(dispatcher);

    bool said = false;

    float phi = 0;
    while (!_window_closed)
    {
        window.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

        if (_audio_request && _audio_request.Completed() && !said)
        {
            bool result = _audio_request.get();
            _tts->say(L"Hello I'm the TTS Engine. " + result);
            said = false;
        }

        HolographicFrame current_frame = _renderer->update(previous_frame);

        if (_scene_initialized && _animate)
        {
            phi -= .005f;
            for (auto & object : *_scene)
            {
                object->rotation(DirectX::XMQuaternionRotationRollPitchYaw(0, phi, 0)); // Y Up
            }
        }

        if (_renderer->render(current_frame))
        {
            HolographicFramePresentResult present_result =
                current_frame.PresentUsingCurrentPrediction(
                    HolographicFramePresentWaitBehavior::DoNotWaitForFrameToFinish);

            if (present_result == HolographicFramePresentResult::DeviceRemoved)
            {
                Utility::winrtUtility::LogMessage(L"Direct3D device lost.");
            }
        }
    }

    Utility::winrtUtility::LogMessage(L"Run finished.");
}

void My::App::Uninitialize()
{
    for (auto & f : _token_release) f();
}

winrt::Windows::ApplicationModel::Core::IFrameworkView My::App::CreateView() { return *this; }

void My::App::RequestPermissions() { _audio_request = audio_perimission(); }

winrt::Windows::Foundation::IAsyncOperation<bool> My::App::audio_perimission()
{
    try
    {
        MediaCaptureInitializationSettings settings;

        settings.StreamingCaptureMode(StreamingCaptureMode::Audio); // TODO
        settings.MediaCategory(MediaCategory::Speech);

        _capture = std::make_shared<MediaCapture>();
        try
        {
            co_await _capture->InitializeAsync(settings);
        }
        catch (winrt::hresult_error const & ex)
        {
            // winrt::hresult hr = ex.code();
            winrt::hstring message = ex.message();
            Utility::winrtUtility::LogMessage(message.c_str());
        }
        co_return true;
    }
    catch (winrt::hresult_error const & ex)
    {
        // winrt::hresult hr = ex.code();
        winrt::hstring message = ex.message();
        Utility::winrtUtility::LogMessage(message.c_str());
        Utility::winrtUtility::LogMessage(L"Note: Media components might not be available.");
        co_return false;
    }
}

void My::App::OnViewActivated(
    winrt::Windows::ApplicationModel::Core::CoreApplicationView const & sender,
    winrt::Windows::ApplicationModel::Activation::IActivatedEventArgs const & args)
{
    CoreWindow::GetForCurrentThread().Activate();
}

void My::App::OnSuspending(winrt::Windows::Foundation::IInspectable const & sender,
                           winrt::Windows::ApplicationModel::SuspendingEventArgs const & args)
{
    // TODO: Save App State
}

void My::App::OnResuming(winrt::Windows::Foundation::IInspectable const & sender,
                         winrt::Windows::Foundation::IInspectable const & args)
{
    // TODO: Load App State
}

void My::App::OnVisibilityChanged(winrt::Windows::UI::Core::CoreWindow const & sender,
                                  winrt::Windows::UI::Core::VisibilityChangedEventArgs const & args)
{}

void My::App::OnWindowClosed(winrt::Windows::UI::Core::CoreWindow const & sender,
                             winrt::Windows::UI::Core::CoreWindowEventArgs const & args)
{
    _window_closed = true; // for "game" loop
}

void My::App::OnKeyPressed(winrt::Windows::UI::Core::CoreWindow const & sender,
                           winrt::Windows::UI::Core::KeyEventArgs const & args)
{
    using namespace winrt::Windows::System;

    // Requires a connected Bluetooth keyboard.

    if (args.VirtualKey() == VirtualKey::Space)
    {
        _animate = !_animate;
        if (_tts)
            _tts->say(L"Animation state changed.");
    }

    Utility::winrtUtility::LogMessage(L"Key pressed.");
}

void My::App::OnPointerPressed(winrt::Windows::UI::Core::CoreWindow const & sender,
                               winrt::Windows::UI::Core::PointerEventArgs const & args)
{
    using namespace winrt::Windows::System;

    _animate = !_animate;

    Utility::winrtUtility::LogMessage(L"Pointer pressed.");
}

void My::App::OnCameraAdded(
    winrt::Windows::Graphics::Holographic::HolographicSpace const & sender,
    winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const & args)
{
    winrt::Windows::Foundation::Deferral deferral = args.GetDeferral();
    HolographicCamera holographic_camera = args.Camera();
    create_task([this, deferral, holographic_camera]() {
        _renderer->add_camera(holographic_camera);
        deferral.Complete();
    });
}

void My::App::OnCameraRemoved(
    winrt::Windows::Graphics::Holographic::HolographicSpace const & sender,
    winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const & args)
{
    create_task([this]() {
        //
        // TODO: Asynchronously unload or deactivate content resources (not back buffer
        //       resources) that are specific only to the camera that was removed.
        //
    });
    _renderer->remove_camera(args.Camera());
}

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::init_apartment();
    winrt::Windows::ApplicationModel::Core::CoreApplication::Run(winrt::make<My::App>());
    return 0;
}