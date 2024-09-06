// (c) 2018-2024 Ricci Adams. Public Domain.

#include "AppleUSBAudioPlugin.h"
#include <vecLib/vDSP.h>

enum {
    BiquadTypePeaking,
    BiquadTypeLowpass,
    BiquadTypeHighpass,
    BiquadTypeBandpass,
    BiquadTypeLowshelf,
    BiquadTypeHighshelf
};

typedef struct {
    UInt32 type;
    double frequency;
    double gain;
    double Q;
} Biquad;

class IccirAudioOptimizerPlugin : public AppleUSBAudioPlugin {
    OSDeclareDefaultStructors(IccirAudioOptimizerPlugin)

private:
	vDSP_biquadm_Setup mBiquadSetup;
	double mSampleRate;	
	UInt32 mNumChannels;
    float **mChannelsArray;
    bool mNeedsRemake;
    
    OSData *mBiquadsData;
    bool mBiquadsEnabled;

    void _remakeIfNeeded();
    void _reset();
    vDSP_biquadm_Setup _createSetup();


public:
    virtual void free();
	virtual IOReturn setProperties(OSObject * properties);

	virtual	IOReturn pluginInit(IOService * provider, UInt16 vendorID, UInt16 productID);
	virtual	IOReturn pluginStart();
	virtual	IOReturn pluginReset();
	virtual	IOReturn pluginSetFormat(const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate);
	virtual	IOReturn pluginProcess(float *mixBuf, UInt32 numSampleFrames, UInt32 numChannels);
	virtual	IOReturn pluginStop();
};


