# AudioOptimizerPlugin
Example kernel extension to flatten speaker response


# AudioOptimizerPlugin

AudioOptimizerPlugin is a macOS kernel extension which flattens the frequency response of my Audioengine A2+ speakers.
It could be considered a modern version of Apple's [SampleUSBAudioPlugin](https://developer.apple.com/library/content/samplecode/SampleUSBAudioPlugin)
project from 2004.

All files except for `AppleUSBAudioPlugin.h` are dedicated to the public domain.


## Usage

1. Use [FuzzMeasure](https://www.rodetest.com) or [REW](https://www.roomeqwizard.com) with a calibrated microphone like
the [UMIK-1](https://www.minidsp.com/products/acoustic-measurement/umik-1) to measure the frequency response
of your speakers and room.
2. Import the frequency response into REW.
3. Use REW's EQ tool to build a list of peaking filters for correction.
4. Export these filters as a text file.
5. Modify `OptimizerSettings.h` with the values from #4.
6. Change `idProduct` and `idVendor` in `Info.plist` to match your speakers.
7. Change `vendorID` and `productID` in `IccirAudioOptimizerPlugin::pluginInit` 
8. Build, install in `/Library/Extensions`, enjoy!
