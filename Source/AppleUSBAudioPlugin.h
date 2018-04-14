#ifndef __APPLE_USB_AUDIO_PLUGIN_H__
#define __APPLE_USB_AUDIO_PLUGIN_H__

#include <IOKit/IOService.h>
#include <IOKit/audio/IOAudioTypes.h>

class AppleUSBAudioEngine;
class AppleUSBAudioStream;

class AppleUSBAudioPlugin : public IOService {

	OSDeclareDefaultStructors (AppleUSBAudioPlugin)

private:
	AppleUSBAudioEngine *			mOurProvider;

protected:
	struct ExpansionData { 
		AppleUSBAudioStream *	streamProvider;
	};

	ExpansionData *reserved;

public:
	// OSMetaClassDeclareReservedUsed (AppleUSBAudioPlugin, 0);
	virtual IOReturn	pluginProcessInput (float * destBuf, UInt32 numSampleFrames, UInt32 numChannels);
	// OSMetaClassDeclareReservedUsed (AppleUSBAudioPlugin, 1);
	virtual IOReturn	pluginSetDirection (IOAudioStreamDirection direction);

private:
	OSMetaClassDeclareReservedUsed (AppleUSBAudioPlugin, 0);
	OSMetaClassDeclareReservedUsed (AppleUSBAudioPlugin, 1);

	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 2);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 3);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 4);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 5);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 6);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 7);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 8);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 9);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 10);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 11);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 12);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 13);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 14);
	OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 15);

public:
	virtual	bool		start(IOService *provider);
	virtual	void		stop(IOService *provider);

	IOReturn			pluginDeviceRequest(void *request, void *completion = NULL);
	void				pluginSetConfigurationApp (const char *bundleID);

	virtual	IOReturn	pluginInit(IOService *provider, UInt16 vendorID, UInt16 modelID);
	virtual	IOReturn	pluginStart();
	virtual	IOReturn	pluginReset();
	virtual	IOReturn	pluginSetFormat(const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate);
	virtual	IOReturn	pluginProcess(float *mixBuf, UInt32 numSampleFrames, UInt32 numChannels);
	virtual	IOReturn	pluginStop();
};

#endif // __APPLE_USB_AUDIO_PLUGIN_H__
