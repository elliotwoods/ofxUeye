#pragma once

#include "ofxMachineVision/Device/Blocking.h"

namespace ofxMachineVision {
	namespace Device {
		class UEye : public ofxMachineVision::Device::Blocking {
		public:
			struct InitialisationSettings : Base::InitialisationSettings {
			public:
				InitialisationSettings() {
					add(useCameraIDAsDeviceID.set("Use Device ID as Camera ID", true));

					add(useImageFormat.set("Use Image Format", false));
					add(imageFormat.set("Image Format", 1));

					this->deviceID = 1;
				}
				
				ofParameter<bool> useCameraIDAsDeviceID;

				ofParameter<bool> useImageFormat;
				ofParameter<int> imageFormat;
			};

			UEye();
			string getTypeName() const override;
			shared_ptr<Base::InitialisationSettings> getDefaultSettings() override {
				return make_shared<InitialisationSettings>();
			}
			Specification open(shared_ptr<Base::InitialisationSettings> = nullptr) override;
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