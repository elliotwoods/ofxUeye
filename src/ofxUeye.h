#pragma once

#include "ofxMachineVision.h"

namespace ofxUeye {
	class Device : ofxMachineVision::Device::Blocking {
	public:
		Device();
		virtual ~Device();
		ofxMachineVision::Specification open(int deviceID = 0) override;
		void close() override;
		bool startCapture() override;
		void stopCapture() override;
		void setExposure(ofxMachineVision::Microseconds) override;
		void setGain(float) override;
		void setBinning(int binningX = 1, int binningY = 1) override;
		void setROI(const ofRectangle &) override;
		/*void setTriggerMode(const ofxMachineVision::TriggerMode &, const ofxMachineVision::TriggerSignalType &) override;
		void setGPOMode(const ofxMachineVision::GPOMode &) override;
		void getFrame(shared_ptr<ofxMachineVision::Frame>) override;*/

	protected:
		DWORD cameraHandle;
		ofPixels pixels;
		int imageMemoryID;
		int maxClock;
		double fps;
	};
}