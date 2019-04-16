#include "ofxUeye.h"
#include "../libs/ueye/include/uEye.h"

using namespace ofxMachineVision;

namespace ofxMachineVision {
	namespace Device {
		//----------
		UEye::UEye() {
			this->cameraHandle = NULL;
			this->imageMemoryID = 0;
		}

		//----------
		string UEye::getTypeName() const {
			return "UEye";
		}

		//----------
		Specification UEye::open(shared_ptr<Base::InitialisationSettings> initialisationSettings) {
			auto settings = this->getTypedSettings<InitialisationSettings>(initialisationSettings);		

			int result;

			{
				//get camera ID or device ID
				HIDS cameraHandle;
				if (settings->useCameraIDAsDeviceID) {
					cameraHandle = settings->deviceID;
				}
				else {
					cameraHandle = settings->deviceID + 1001 | IS_USE_DEVICE_ID;
				}

				//open the camera
				result = is_InitCamera(&cameraHandle, NULL);
				this->cameraHandle = cameraHandle;
			}
			

			//upload firmware if we have to
			if (result == IS_STARTER_FW_UPLOAD_NEEDED) {
				int timeNeeded;
				is_GetDuration(cameraHandle, IS_SE_STARTER_FW_UPLOAD, &timeNeeded);
				OFXMV_WARNING << "Camera firmware upload required, please wait " << timeNeeded << "s";
				cameraHandle = (HIDS)(((INT)cameraHandle) | IS_ALLOW_STARTER_FW_UPLOAD);
				result = is_InitCamera(&cameraHandle, NULL);
			}

			//check it opened correctly
			if (result != IS_SUCCESS) {
				OFXMV_WARNING << "Couldn't open device with specified Device ID, attempting instead to open camera with Camera ID 1";
				HIDS cameraHandle = 1;
				result = is_InitCamera(&cameraHandle, NULL);
				this->cameraHandle = cameraHandle;
			}
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Couldn't initialise camera";
				return Specification();
			}

			//get camera properties
			BOARDINFO cameraInfo;
			result = is_GetCameraInfo(this->cameraHandle, &cameraInfo);
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Can't get camera info";
				return Specification();
			}
			SENSORINFO sensorInfo;
			result = is_GetSensorInfo(this->cameraHandle, &sensorInfo);
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Can't get sensor info";
				return Specification();
			}

			//default width and height is sensor maximum
			int width = sensorInfo.nMaxWidth;
			int height = sensorInfo.nMaxHeight;

			//set the image format
			if (settings->useImageFormat) {
				int imageFormat = settings->imageFormat;
				//from is_ImageFormat reference:

				// Get number of available formats and size of list
				UINT count;
				UINT bytesNeeded = sizeof(IMAGE_FORMAT_LIST);
				result = is_ImageFormat(this->cameraHandle, IMGFRMT_CMD_GET_NUM_ENTRIES, &count, sizeof(count));
				bytesNeeded += (count - 1) * sizeof(IMAGE_FORMAT_INFO);
				void* ptr = malloc(bytesNeeded);

				// Create and fill list
				IMAGE_FORMAT_LIST* pformatList = (IMAGE_FORMAT_LIST*)ptr;
				pformatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
				pformatList->nNumListElements = count;
				result = is_ImageFormat(this->cameraHandle, IMGFRMT_CMD_GET_LIST, pformatList, bytesNeeded);

				//go through list finding our format
				bool foundFormat = false;
				for (UINT i = 0; i < count; i++) {
					auto & format = pformatList->FormatInfo[i];
					if (format.nFormatID == imageFormat) {
						//found the format
						width = format.nWidth;
						height = format.nHeight;

						result = is_ImageFormat(this->cameraHandle, IMGFRMT_CMD_SET_FORMAT, &imageFormat, sizeof(imageFormat));
						if (result != IS_SUCCESS) {
							OFXMV_ERROR << "Couldn't set image format";
						}

						foundFormat = true;
						break;
					}
				}
				if (!foundFormat) {
					OFXMV_ERROR << "This camera doesn't support the image format you specified.";
					OFXMV_NOTICE << "Supported formats:";

					for (UINT i = 0; i < count; i++) {
						auto & format = pformatList->FormatInfo[i];
						OFXMV_NOTICE << " [" << format.nFormatID << "] \"" << format.strFormatName << "\" " << format.nWidth << "x" << format.nHeight;
					}
				}
			}

			auto specification = Specification(CaptureSequenceType::Continuous, width, height, cameraInfo.ID, sensorInfo.strSensorName, cameraInfo.SerNo);

			this->pixels.allocate(width, height, OF_PIXELS_GRAY);
			is_SetAllocatedImageMem(this->cameraHandle, width, height, 8, (char *) this->pixels.getData(), &this->imageMemoryID);
			is_SetImageMem(this->cameraHandle, (char *) this->pixels.getData(), this->imageMemoryID);

			//setup some camera parameters
			is_SetColorMode(this->cameraHandle, IS_CM_SENSOR_RAW8);
			result = is_SetOptimalCameraTiming(this->cameraHandle, IS_BEST_PCLK_RUN_ONCE, 4000, &this->maxClock, &this->fps);;

			return specification;
		}

		//----------
		void UEye::close() {
			is_FreeImageMem(this->cameraHandle, (char *) this->pixels.getData(), this->imageMemoryID);
			is_ExitCamera(this->cameraHandle);
			this->cameraHandle = NULL;
		}

		//----------
		bool UEye::startCapture() {
			//if (is_CaptureVideo(this->cameraHandle, IS_DONT_WAIT) != IS_SUCCESS) {
			//	OFXMV_ERROR << "Couldn't start capture";
			//	return false;
			//} else {
			//	return true;
			//}
			return true;
		}

		//----------
		void UEye::stopCapture() {
			//currently we don't use free run mode
		}

		/*
		//----------
		void UEye::setExposure(chrono::microseconds exposureMicros) {
			double autoExposure = 0.0;
			int result;
			result = is_SetAutoParameter(this->cameraHandle, IS_SET_ENABLE_AUTO_SHUTTER, &autoExposure, 0);
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Couldn't stop auto exposure";
			}

			double exposureMillis = (double)exposureMicros.count() / 1000.0;
			result = is_Exposure(this->cameraHandle, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposureMillis, sizeof(exposureMillis));
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Couldn't set exposure";
			}
		}

		//----------
		void UEye::setGain(float gain) {
			double autoGain = 0.0;
			int result;
			result = is_SetAutoParameter(this->cameraHandle, IS_SET_ENABLE_AUTO_GAIN, &autoGain, 0);
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Couldn't stop auto gain";
			}

			int gainInt = (int)(gain * 100.0f);
			result = is_SetHardwareGain(this->cameraHandle, gainInt, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
			if (result != IS_SUCCESS) {
				OFXMV_ERROR << "Couldn't set gain";
			}
		}

		//----------
		void UEye::setBinning(int binningX, int binningY) {
			int result = IS_SUCCESS;
			if (binningX < 1 || binningY < 1 || binningX > 16 || binningY > 16) {
				OFXMV_ERROR << "Can't set a binning value of less than 1 or more than 16";
			}
			else if (binningX == 1 && binningY == 1) {
				result = is_SetBinning(this->cameraHandle, IS_BINNING_DISABLE);
			}
			else {
				int horizontalFlag = IS_BINNING_DISABLE;
				switch (binningX) {
				case 2:
					horizontalFlag = IS_BINNING_2X_HORIZONTAL;
					break;
				case 3:
					horizontalFlag = IS_BINNING_3X_HORIZONTAL;
					break;
				case 4:
					horizontalFlag = IS_BINNING_4X_HORIZONTAL;
					break;
				case 5:
					horizontalFlag = IS_BINNING_5X_HORIZONTAL;
					break;
				case 6:
					horizontalFlag = IS_BINNING_6X_HORIZONTAL;
					break;
				case 8:
					horizontalFlag = IS_BINNING_8X_HORIZONTAL;
					break;
				case 16:
					horizontalFlag = IS_BINNING_16X_HORIZONTAL;
					break;
				default:
					OFXMV_ERROR << "Cannot set binning, please check manual for valid values";
					break;
				}
				if (horizontalFlag != IS_BINNING_DISABLE) {
					if (is_SetBinning(this->cameraHandle, horizontalFlag) != IS_SUCCESS) {
						OFXMV_ERROR << "Failed to set horizontal binning";
					}
				}

				int verticalFlag = IS_BINNING_DISABLE;
				switch (binningX) {
				case 2:
					verticalFlag = IS_BINNING_2X_VERTICAL;
					break;
				case 3:
					verticalFlag = IS_BINNING_3X_VERTICAL;
					break;
				case 4:
					verticalFlag = IS_BINNING_4X_VERTICAL;
					break;
				case 5:
					verticalFlag = IS_BINNING_5X_VERTICAL;
					break;
				case 6:
					verticalFlag = IS_BINNING_6X_VERTICAL;
					break;
				case 8:
					verticalFlag = IS_BINNING_8X_VERTICAL;
					break;
				case 16:
					verticalFlag = IS_BINNING_16X_VERTICAL;
					break;
				default:
					OFXMV_ERROR << "Cannot set binning, please check manual for valid values";
					break;
				}
				if (verticalFlag != IS_BINNING_DISABLE) {
					if (is_SetBinning(this->cameraHandle, verticalFlag) != IS_SUCCESS) {
						OFXMV_ERROR << "Failed to set vertical binning";
					}
				}
			}
		}

		//----------
		void UEye::setROI(const ofRectangle & roi) {
			IS_RECT aoiRect;
			aoiRect.s32X = (int)roi.x | IS_AOI_IMAGE_POS_ABSOLUTE;
			aoiRect.s32Y = (int)roi.y | IS_AOI_IMAGE_POS_ABSOLUTE;
			aoiRect.s32Width = (int)roi.width;
			aoiRect.s32Height = (int)roi.height;

			is_AOI(this->cameraHandle, IS_AOI_IMAGE_SET_AOI, &aoiRect, sizeof(aoiRect));
		}
		*/

		//----------
		shared_ptr<Frame> UEye::getFrame() {
			auto result = is_FreezeVideo(this->cameraHandle, IS_WAIT);
			if (result != IS_SUCCESS) {
				if (result != IS_TIMED_OUT) {
					throw(ofxMachineVision::Exception("Failed to capture frame"));
				}
			}

			UEYEIMAGEINFO imageInfo;
			if (is_GetImageInfo(this->cameraHandle, this->imageMemoryID, &imageInfo, sizeof(imageInfo)) == IS_SUCCESS) {
				auto frame = FramePool::X().getAvailableFrameFilledWith(this->pixels);
				frame->setTimestamp(chrono::microseconds(imageInfo.u64TimestampDevice / 10));
				frame->setFrameIndex(imageInfo.u64FrameNumber);
				return frame;
			}
			else {
				throw(ofxMachineVision::Exception("Failed to get image info"));
			}
		}
	}
}
