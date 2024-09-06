// (c) 2018-2024 Ricci Adams. Public Domain.

#include <IOKit/IOLib.h>

#include "IccirAudioOptimizerPlugin.h"
#include <math.h>
#include <IOKit/IOUserClient.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/audio/IOAudioStream.h>
#pragma clang diagnostic pop

#define kBiquadsDataKey	   "BiquadsData"
#define kBiquadsEnabledKey "BiquadsEnabled"


#define super AppleUSBAudioPlugin
OSDefineMetaClassAndStructors(IccirAudioOptimizerPlugin, AppleUSBAudioPlugin);


void IccirAudioOptimizerPlugin::free()
{
    if (mBiquadSetup) {
        vDSP_biquadm_DestroySetup(mBiquadSetup);
        mBiquadSetup = NULL;
    }
    
    if (mChannelsArray) {
        delete [] mChannelsArray;
        mChannelsArray = NULL;
    }
    
    if (mBiquadsData) {
        mBiquadsData->release();
        mBiquadsData = NULL;
    }
}


IOReturn IccirAudioOptimizerPlugin::setProperties(OSObject *properties)
{
    if (!properties) return kIOReturnBadArgument;

    OSDictionary *dictionary = OSDynamicCast(OSDictionary, properties);
    if (!dictionary) return kIOReturnBadArgument;

    OSObject *biquadsDataObject = dictionary->getObject(kBiquadsDataKey);
    if (biquadsDataObject) {
        OSData *biquadsData = OSDynamicCast(OSData, biquadsDataObject);
        if (!biquadsData) return kIOReturnBadArgument;
        
        if (!mBiquadsData || !mBiquadsData->isEqualTo(biquadsData)) {
            if (mBiquadsData) mBiquadsData->release();
            biquadsData->retain();
            mBiquadsData = biquadsData;
            
            setProperty(kBiquadsDataKey, mBiquadsData);
           
            mNeedsRemake = true;
        }
    }

    OSObject *biquadsEnabledObject = dictionary->getObject(kBiquadsEnabledKey);
    if (biquadsEnabledObject) {
        OSBoolean *biquadsEnabledBoolean = OSDynamicCast(OSBoolean, biquadsEnabledObject);
        if (!biquadsEnabledBoolean) return kIOReturnBadArgument;

        mBiquadsEnabled = biquadsEnabledBoolean->isTrue();
        
        setProperty(kBiquadsEnabledKey, mBiquadsEnabled);
    }
    
    _remakeIfNeeded();
    
    return kIOReturnSuccess;
}


IOReturn IccirAudioOptimizerPlugin::pluginInit(IOService * provider, UInt16 vendorID, UInt16 productID) 
{
	IOReturn result = super::pluginInit(provider, vendorID, productID);

	if (result == kIOReturnSuccess) {
        mSampleRate = 0;
  	    mNumChannels = 0;
       
        setProperty(kBiquadsEnabledKey, false);
        
        OSData *emptyData = OSData::withBytes(NULL, 0);
        setProperty(kBiquadsDataKey, emptyData);
        emptyData->release();
	}

	return result;
}


vDSP_biquadm_Setup IccirAudioOptimizerPlugin::_createSetup()
{
    if (!mBiquadsData || !mSampleRate || !mNumChannels) {
        return NULL;
    }

    const Biquad *biquad = (const Biquad *)mBiquadsData->getBytesNoCopy();

    unsigned int biquadCount = mBiquadsData->getLength() / sizeof(Biquad);
    if (!biquadCount) return NULL;
    
    double *coefficients = new double[biquadCount * mNumChannels * 5];

    for (UInt32 i = 0; i < biquadCount; i++) {
        UInt32 type = biquad->type;
        double Fc   = biquad->frequency / mSampleRate;
        double gain = biquad->gain;
        double Q    = biquad->Q;
    
        double b0 = 0;
        double b1 = 0;
        double b2 = 0;
        double a0 = 1.0;
        double a1 = 0;
        double a2 = 0;

        double A      = pow(10, gain / 40.0);
        double w0     = 2 * M_PI * Fc;
        double sin_w0 = sin(w0);
        double cos_w0 = cos(w0);
        double alpha  = sin_w0 / (2 * Q);

        if (type == BiquadTypeLowpass) {
            b0 =  (1.0 - cos_w0) / 2.0;
            b1 =   1.0 - cos_w0;
            b2 =  (1.0 - cos_w0) / 2.0;
            a0 =   1.0 + alpha;
            a1 =  -2.0 * cos_w0;
            a2 =   1.0 - alpha;

        } else if (type == BiquadTypeHighpass) {
            b0 =  (1.0 + cos_w0) / 2.0;
            b1 = -(1.0 + cos_w0);
            b2 =  (1.0 + cos_w0) / 2.0;
            a0 =   1.0 + alpha;
            a1 =  -2.0 * cos_w0;
            a2 =   1.0 - alpha;

        } else if (type == BiquadTypeBandpass) {
            b0 =   alpha;
            b1 =   0.0;
            b2 =  -alpha;
            a0 =   1.0 + alpha;
            a1 =  -2.0 * cos_w0;
            a2 =   1.0 - alpha;

        } else if (type == BiquadTypeLowshelf || type == BiquadTypeHighshelf) {
            double Aplus1      = A + 1.0;
            double Aminus1     = A - 1.0;
            double Aplus1_cos  = Aplus1  * cos_w0;
            double Aminus1_cos = Aminus1 * cos_w0;
            double beta_sin    = sqrt(A) * M_SQRT2 * sin_w0;

            if (type == BiquadTypeLowshelf) {
                b0 =        A * ( Aplus1  - Aminus1_cos + beta_sin );
                b1 =  2.0 * A * ( Aminus1 - Aplus1_cos             );
                b2 =        A * ( Aplus1  - Aminus1_cos - beta_sin );
                a0 =            ( Aplus1  + Aminus1_cos + beta_sin );
                a1 = -2.0     * ( Aminus1 + Aplus1_cos             );
                a2 =            ( Aplus1  + Aminus1_cos - beta_sin );
            } else {
                b0 =        A * ( Aplus1  + Aminus1_cos + beta_sin );
                b1 = -2.0 * A * ( Aminus1 + Aplus1_cos             );
                b2 =        A * ( Aplus1  + Aminus1_cos - beta_sin );
                a0 =            ( Aplus1  - Aminus1_cos + beta_sin );
                a1 =  2.0     * ( Aminus1 - Aplus1_cos             );
                a2 =            ( Aplus1  - Aminus1_cos - beta_sin );
            }

        } else if (type == BiquadTypePeaking) {
            b0 =   1.0 + alpha * A;
            b1 =  -2.0 * cos_w0;
            b2 =   1.0 - alpha * A;
            a0 =   1.0 + alpha / A;
            a1 =  -2.0 * cos_w0;
            a2 =   1.0 - alpha / A;
        }

        for (vDSP_Length j = 0; j < mNumChannels; j++) {
            vDSP_Length offset = (i * mNumChannels * 5) + (j * 5);
        
            coefficients[offset + 0] = b0 / a0;
            coefficients[offset + 1] = b1 / a0;
            coefficients[offset + 2] = b2 / a0;
            coefficients[offset + 3] = a1 / a0;
            coefficients[offset + 4] = a2 / a0;
        }
    
        biquad++;
    }
    
    vDSP_biquadm_Setup result = vDSP_biquadm_CreateSetup(coefficients, biquadCount, mNumChannels);

    delete [] coefficients;
    
    return result;
}


void IccirAudioOptimizerPlugin::_remakeIfNeeded()
{
    if (!mNeedsRemake) return;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    IOAudioStream *stream = (IOAudioStream *)reserved->streamProvider;
    IOAudioEngine *engine = NULL;
    bool needsResume = false;
    
    if (stream) {
        engine = stream->audioEngine;
    }

    if (engine && (engine->getState() == kIOAudioEngineRunning)) {
		IOLog("IccirAudioOptimizerPlugin: Pausing audio engine.\n");
        engine->pauseAudioEngine();
        needsResume = true;
    }
#pragma clang diagnostic pop

    if (mBiquadSetup) {
        vDSP_biquadm_DestroySetup(mBiquadSetup);
        mBiquadSetup = NULL;
    }
    
    mBiquadSetup = _createSetup();
    mNeedsRemake = false;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (engine && needsResume) {
		IOLog("IccirAudioOptimizerPlugin: Resuming audio engine.\n");
        engine->resumeAudioEngine();
    }
#pragma clang diagnostic pop
}


void IccirAudioOptimizerPlugin::_reset()
{
    if (mBiquadSetup) {
        vDSP_biquadm_ResetState(mBiquadSetup);
    }
}


IOReturn IccirAudioOptimizerPlugin::pluginStart() 
{
    IOLog("IccirAudioOptimizerPlugin: Starting.\n");

	IOReturn result = super::pluginStart();

	if (result == kIOReturnSuccess) {
        _remakeIfNeeded();
        _reset();
	}

	return result;
}


IOReturn IccirAudioOptimizerPlugin::pluginReset() 
{
    IOLog("IccirAudioOptimizerPlugin: Reseting.\n");
    _reset();
	return kIOReturnSuccess;
}


IOReturn IccirAudioOptimizerPlugin::pluginSetFormat(const IOAudioStreamFormat * const inFormat, const IOAudioSampleRate * const inSampleRate) 
{
	IOLog(
        "IccirAudioOptimizerPlugin: Changing format, channels=%ld, sampleRate=%lf\n",
        (long)  (inFormat     ? inFormat->fNumChannels : 0),
        (double)(inSampleRate ? inSampleRate->whole    : 0)
    );
	
	if (inSampleRate) {
        double sampleRate = inSampleRate->whole;

        if (sampleRate != mSampleRate) {
            mSampleRate = sampleRate;
            mNeedsRemake = true;
        }
    }

	if (inFormat) {
        UInt32 numChannels = inFormat->fNumChannels;
        if (numChannels != mNumChannels) {
            if (mChannelsArray) {
                delete [] mChannelsArray;
            }

            if (numChannels > 0) {
                mChannelsArray = new float *[mNumChannels];
            } else {
                mChannelsArray = NULL;
            }

            mNumChannels = numChannels;
            mNeedsRemake = true;
        }
    }
   
	return kIOReturnSuccess;
} 


IOReturn IccirAudioOptimizerPlugin::pluginProcess(float *mixBuf, UInt32 numSampleFrames, UInt32 numChannels) 
{
    if (mBiquadsEnabled && mBiquadSetup) {
        for (UInt32 i = 0; i < mNumChannels; i++) {
            mChannelsArray[i] = &mixBuf[i];
        }
        
        vDSP_biquadm(
            mBiquadSetup,
            (const float **)mChannelsArray, mNumChannels,
            mChannelsArray, mNumChannels,
            numSampleFrames
        );
    }

	return kIOReturnSuccess;
}


IOReturn IccirAudioOptimizerPlugin::pluginStop() 
{
	IOLog("IccirAudioOptimizerPlugin: Stopping.\n");
	return super::pluginStop();
}
