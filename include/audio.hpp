#pragma once

#include <vector>
#include <jansson.h>

#ifndef ARCH_WEB
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <RtAudio.h>
#pragma GCC diagnostic pop
#endif

namespace rack {


struct AudioIO {
	// Stream properties
	int driver = 0;
	int device = -1;
	int offset = 0;
	int maxChannels = 8;
	int sampleRate = 48000;
#ifndef ARCH_WEB
	int blockSize = 512;
#else
	int blockSize = 1024;
#endif
	int numOutputs = 0;
	int numInputs = 0;
#ifndef ARCH_WEB
	RtAudio *rtAudio = NULL;
	/** Cached */
	RtAudio::DeviceInfo deviceInfo;
#endif

	AudioIO();
	virtual ~AudioIO();

	std::vector<int> getDrivers();
	std::string getDriverName(int driver);
	void setDriver(int driver);

	int getDeviceCount();
#ifndef ARCH_WEB
	bool getDeviceInfo(int device, RtAudio::DeviceInfo *deviceInfo);
#endif
	/** Returns the number of inputs or outputs, whichever is greater */
	int getDeviceChannels(int device);
	std::string getDeviceName(int device);
	std::string getDeviceDetail(int device, int offset);
	void setDevice(int device, int offset);

	std::vector<int> getSampleRates();
	void setSampleRate(int sampleRate);
	std::vector<int> getBlockSizes();
	void setBlockSize(int blockSize);

	void setChannels(int numOutputs, int numInputs);

	/** Must close the stream before opening */
	void openStream();
	void closeStream();

	virtual void processStream(const float *input, float *output, int frames) {}
	virtual void onCloseStream() {}
	virtual void onOpenStream() {}
	virtual void onChannelsChange() {}
	virtual void onDeviceChange() {}
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


} // namespace rack
