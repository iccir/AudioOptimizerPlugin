#include "AppleUSBAudioPlugin.h"
#include <vecLib/vecLib.h>

class IccirAudioOptimizerPlugin : public AppleUSBAudioPlugin {

    OSDeclareDefaultStructors (IccirAudioOptimizerPlugin)

private:
	vDSP_biquadm_Setup      mBiquadSetup;
	double 					mSampleRate;	
	UInt32					mNumChannels;

    void _reset();

public:
	virtual	IOReturn	pluginInit (IOService * provider, UInt16 vendorID, UInt16 productID);
	virtual	IOReturn	pluginStart ();
	virtual	IOReturn	pluginReset ();
	virtual	IOReturn	pluginSetFormat (const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate);
	virtual	IOReturn	pluginProcess (float * mixBuf, UInt32 numSampleFrames, UInt32 numChannels);
	virtual	IOReturn	pluginStop ();
};
