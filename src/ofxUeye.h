#pragma once

#include "ofxMachineVision/Device/Blocking.h"

namespace ofxMachineVision {
	namespace Device {
		class UEye : public ofxMachineVision::Device::Blocking {
		public:
			UEye();
			string getTypeName() const override;
			ofxMachineVision::Specification open(int deviceID = 0) override;
			void close() override;
			bool startCapture() override;
			void stopCapture() override;
			void setExposure(ofxMachineVision::Microseconds) override;
			void setGain(float) override;
			void setBinning(int binningX = 1, int binningY = 1) override;
			void setROI(const ofRectangle &) override;
			/*void setTriggerMode(const ofxMachineVision::TriggerMode &, const ofxMachineVision::TriggerSignalType &) override;
			void setGPOMode(const ofxMachineVision::GPOMode &) override;*/
			void getFrame(shared_ptr<ofxMachineVision::Frame>) override;

		protected:
			DWORD cameraHandle;
			ofPixels pixels;
			int imageMemoryID;
			int maxClock;
			double fps;
			int frameIndex;
		};
	}
}