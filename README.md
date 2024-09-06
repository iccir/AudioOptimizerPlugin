# AudioOptimizerPlugin

AudioOptimizerPlugin is a macOS kernel extension which flattens the frequency response of my Audioengine A2+ speakers.
It could be considered a modern version of Apple's [SampleUSBAudioPlugin](https://developer.apple.com/library/content/samplecode/SampleUSBAudioPlugin)
project from 2004.

All files except for `AppleUSBAudioPlugin.h` are dedicated to the public domain.

As of 2024, `IOAudio*` and kernel extensions are deprecated. That said, this still works 
in macOS Sonoma 14.6.1.

## Why not use AudioTap?

Sonoma introduced the concept of 
[system-wide audio taps](https://developer.apple.com/documentation/coreaudio/capturing_system_audio_with_core_audio_taps?language=objc).
In theory, this allow for system-wide equalizers ([see example code](https://gist.github.com/iccir/952b5de5579d22ed6d6e645f2122f5b7)).
Unfortuately, inter-process communication added a constant overhead of around ~2% CPU use from `coreaudiod`,
even when no audio is playing.

This is halfway solved by the `kAudioAggregateDeviceTapAutoStartKey` key.
Sadly, there is no concept of **auto stop**.

## Usage

Unlike the [original version](https://github.com/iccir/AudioOptimizerPlugin/releases/tag/OriginalVersion),
this version doesn't use hardcoded biquadratic coefficients.

You will need to provide coefficient data after system boot. You may also change coefficients while audio
is playing.

Find the `IccirOptimizerAudioPlugin` entry for the corresponding device in the `IOService` plane,
then use `IORegistryEntrySetCFProperty` to set the `BiquadsData` and `BiquadsEnabled` properties.
