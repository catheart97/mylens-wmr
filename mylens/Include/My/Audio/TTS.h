#pragma once

#include "pch.h"

namespace My::Audio
{

namespace _implementation
{

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::SpeechRecognition;
using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::SpeechSynthesis;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Core;

class TTS
{
    // Data
private:
    CoreDispatcher & _dispatcher;

    // Constructors
public:
    TTS(CoreDispatcher & dispatcher) : _dispatcher{dispatcher} {}

    // Methods
public:
    IAsyncAction say(winrt::hstring message)
    {
        using namespace concurrency;
        using namespace winrt::Windows::Media::Playback;
        using namespace winrt::Windows::Media::Core;

        Utility::winrtUtility::LogMessage(L"TTS: " + message);

        SpeechSynthesizer synthesizer;

        // Never managed to get this to work
        SpeechSynthesisStream speech_stream =
            co_await synthesizer.SynthesizeTextToStreamAsync(message);

        co_await winrt::resume_foreground(_dispatcher, CoreDispatcherPriority::Normal);

        MediaPlayer player;
        player.Source(MediaSource::CreateFromStream(speech_stream, speech_stream.ContentType()));
        player.Play();
    }
};

} // namespace _implementation

using _implementation::TTS;

} // namespace My::Audio