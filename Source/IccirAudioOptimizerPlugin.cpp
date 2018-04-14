#include <IOKit/IOLib.h>

#include "IccirAudioOptimizerPlugin.h"
#include "OptimizerSettings.h"

OSDefineMetaClassAndStructors (IccirAudioOptimizerPlugin, AppleUSBAudioPlugin);


const UInt32 sFilterCount = sizeof(IccirAudioOptimizerSettingsFilterParam) / sizeof(IccirAudioOptimizerFilterParam);

    

IOReturn IccirAudioOptimizerPlugin::pluginInit(IOService * provider, UInt16 vendorID, UInt16 productID) 
{
	IOReturn result = AppleUSBAudioPlugin::pluginInit(provider, vendorID, productID);

	if (vendorID != 0x8bb || productID != 0x2704) {
		return kIOReturnNoDevice;
	}
	
	if (result == kIOReturnSuccess) {
		IOLog("IccirAudioOptimizerPlugin initialized successfully\n");

        mSampleRate = 0;
  	    mNumChannels = 0;
	}

	return result;
}


void IccirAudioOptimizerPlugin::_reset()
{
    double coefficients[sFilterCount * mNumChannels * 5];
    UInt32 channels = mNumChannels;

    for (vDSP_Length i = 0; i < sFilterCount; i++) {
        double Fc   = IccirAudioOptimizerSettingsFilterParam[i].frequency / mSampleRate;
        double gain = IccirAudioOptimizerSettingsFilterParam[i].gain * IccirAudioOptimizerGainMultiplier;
        double Q    = IccirAudioOptimizerSettingsFilterParam[i].Q;
        
        double A      = pow(10.0, gain / 40.0);
        double w0     = 2 * M_PI * Fc;
        double sin_w0 = sin(w0);
        double cos_w0 = cos(w0);
        double alpha  = sin_w0 / (2 * Q);

        const double b0 =   1.0 + alpha * A;
        const double b1 =  -2.0 * cos_w0;
        const double b2 =   1.0 - alpha * A;
        const double a0 =   1.0 + alpha / A;
        const double a1 =  -2.0 * cos_w0;
        const double a2 =   1.0 - alpha / A;

        for (vDSP_Length j = 0; j < channels; j++) {
            vDSP_Length offset = (i *channels * 5) + (j * 5);
        
            coefficients[offset + 0] = b0 / a0;
            coefficients[offset + 1] = b1 / a0;
            coefficients[offset + 2] = b2 / a0;
            coefficients[offset + 3] = a1 / a0;
            coefficients[offset + 4] = a2 / a0;
        }
    }

    if (mBiquadSetup) {
        vDSP_biquadm_DestroySetup(mBiquadSetup);
        mBiquadSetup = NULL;
    }
        
    mBiquadSetup = vDSP_biquadm_CreateSetup(coefficients, sFilterCount, mNumChannels);
    IOLog("IccirAudioOptimizerPlugin: Created biquad setup: %p", mBiquadSetup);
}


IOReturn IccirAudioOptimizerPlugin::pluginStart() 
{
	IOReturn result = AppleUSBAudioPlugin::pluginStart();

	if (result == kIOReturnSuccess) {
        _reset();
		IOLog("IccirAudioOptimizerPlugin starting.\n");
	}

	return result;
}


IOReturn IccirAudioOptimizerPlugin::pluginReset() 
{    
    _reset();
	return kIOReturnSuccess;
}


IOReturn IccirAudioOptimizerPlugin::pluginSetFormat(const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate) 
{
	IOLog("IccirAudioOptimizerPlugin changing format\n");
	
	if (newSampleRate) mSampleRate  = newSampleRate->whole;
	if (newFormat)     mNumChannels = newFormat->fNumChannels;
    
    _reset();
    
	return kIOReturnSuccess;
} 


IOReturn IccirAudioOptimizerPlugin::pluginProcess(float *mixBuf, UInt32 numSampleFrames, UInt32 numChannels) 
{
    if (mBiquadSetup) {
        const float *input[mNumChannels];
        float *output[mNumChannels];
        
        for (UInt32 i = 0; i < mNumChannels; i++) {
            input[i] = output[i] = &mixBuf[i];
        }
        
        vDSP_biquadm(mBiquadSetup, input, numChannels, output, numChannels, numSampleFrames);
    }

	return kIOReturnSuccess;
}


IOReturn IccirAudioOptimizerPlugin::pluginStop() 
{
	IOReturn result;

	IOLog("IccirAudioOptimizerPlugin stopping.\n");

    if (mBiquadSetup) {
        vDSP_biquadm_DestroySetup(mBiquadSetup);
        mBiquadSetup = NULL;
    }

	result = AppleUSBAudioPlugin::pluginStop();

	return result;
}

