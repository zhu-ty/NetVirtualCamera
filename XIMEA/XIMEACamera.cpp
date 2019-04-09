/**
@brief Generic Camera Driver Class
Implementation of XIMEA camera
@author Shane Yuan
@date Dec 29, 2017
*/

#include "XIMEACamera.h"

namespace cam {
	// function to check XIMEA function error
	void checkXIMEAInternal(XI_RETURN result, char const *const func,
		const char *const file, int const line) {
		if (result != XI_OK) {
			char info[256];
			sprintf(info, "XIMEA camera error at %s:%d function: %s\n",
				file, line, func);
			SysUtil::errorOutput(info);
			//exit(-1);
		}
	}

	GenCameraXIMEA::GenCameraXIMEA() : internalBufferSize(3) {
		this->camModel = CameraModel::XIMEA_xiC;
	}
	GenCameraXIMEA::~GenCameraXIMEA() {}

	/***********************************************************/
	/*                   basic camera functions                */
	/***********************************************************/
	/**
	@brief make setting effective
	by capturing some frames
	@return int
	*/
	int GenCameraXIMEA::makeSetEffective(int k) {
		if (!this->isCapture) {
			SysUtil::warningOutput("This function must be executed after " \
				"starting capturing !");
			return -1;
		}
		for (size_t k = 0; k < 10; k++) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
			}
		}
		return 0;
	}


	/**
	@brief init camera
	@return int
	*/
	int GenCameraXIMEA::init() {
		// check 
		if (this->isInit == true) {
			SysUtil::warningOutput("GenCameraXIMEA is already initialized. Please do not init twice !");
			return 0;
		}
		// get camera numbers
		PDWORD num = new DWORD;
		checkXIMEAErrors(xiGetNumberDevices(num));
		this->cameraNum = static_cast<size_t>(*num);

 		SysUtil::infoOutput(cv::format("GenCameraXIMEA::init %d XIMEA cameras detected", *num));

		if (cameraNum < 1) {
			SysUtil::errorOutput("No XIMEA cameras detected !");
			return -1;	
		}
		delete num;
		// set parameter
		// open cameras
		hcams.resize(this->cameraNum);
		for (size_t i = 0; i < this->cameraNum; i++) {
			checkXIMEAErrors(xiOpenDevice(i, &hcams[i]));
		}
		// set output to raw/bayer 8
		for (size_t i = 0; i < this->cameraNum; i++) {
			checkXIMEAErrors(xiSetParamInt(hcams[i], 
				XI_PRM_IMAGE_DATA_FORMAT, XI_RAW8));
		}
		// set camera inside buffers to 1 (can get the newest image)
		for (size_t i = 0; i < this->cameraNum; i++) {
			int width, height;
			// get camera 
			checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_WIDTH, &width));
			checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_HEIGHT, &height));
			checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_ACQ_BUFFER_SIZE_UNIT, 1));
			checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_ACQ_BUFFER_SIZE, width * height * internalBufferSize));
			//checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_BUFFERS_QUEUE_SIZE, internalBufferSize));
		}
		// enable lut
		for (size_t i = 0; i < this->cameraNum; i++) {
			checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LUT_EN, 1));
		}

		// for (size_t i = 0; i < this->cameraNum; i++) {
		// 	checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF));
		// 	checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LIMIT_BANDWIDTH_MODE, XI_OFF));
		// 	checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LIMIT_BANDWIDTH, 120*8));
		// 	checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LIMIT_BANDWIDTH_MODE, XI_ON));
		// }

		// init images
		xiImages.resize(this->cameraNum);
		for (size_t i = 0; i < this->cameraNum; i++) {
			memset(&xiImages[i], 0, sizeof(xiImages[i]));
			xiImages[i].size = sizeof(XI_IMG);
		}
		ths.resize(this->cameraNum);
		thStatus.resize(this->cameraNum);
		this->isInit = true;
		// init image ratio vector
		imgRatios.resize(this->cameraNum);
		for (size_t i = 0; i < this->cameraNum; i++) {
			imgRatios[i] = GenCamImgRatio::Full;
		}
		return 0;
	}

	int GenCameraXIMEA::setSyncType(GenCamSyncType type)
	{
		if (type == GenCamSyncType::Software)
		{
			for (size_t i = 0; i < this->cameraNum; i++)
			{
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPO_SELECTOR, 2));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPO_MODE, XI_GPO_OFF));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPI_SELECTOR, 2));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPI_MODE, XI_GPI_OFF));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_TRG_SOURCE, XI_TRG_OFF));
			}
			SysUtil::infoOutput("XIMEA cameras sync type set to SOFTWARE sync");
		}
		else
		{
			for (size_t i = 0; i < this->cameraNum; i++)
			{
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPO_SELECTOR, 2));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPO_MODE, XI_GPO_HIGH_IMPEDANCE));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPI_SELECTOR, 2));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_GPI_MODE, XI_GPI_TRIGGER));
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_TRG_SOURCE, XI_TRG_EDGE_FALLING));
			}
			SysUtil::infoOutput("XIMEA camera sync type set to HARDWARE sync\nUse red(SIG)/blue(GND) & falling edge to trigger the camera.");
		}
		return 0;
	}

	/**
	@brief start capture images
	@return int
	*/
	int GenCameraXIMEA::startCapture() {
		for (size_t i = 0; i < this->cameraNum; i++) {
			checkXIMEAErrors(xiStartAcquisition(hcams[i]));
		}
		this->isCapture = true;
		return 0;
	}

	/**
	@brief stop capture images
	@return int
	*/
	int GenCameraXIMEA::stopCapture() {
		for (size_t i = 0; i < this->cameraNum; i++) {
			checkXIMEAErrors(xiStopAcquisition(hcams[i]));
		}
		this->isCapture = false;
		return 0;
	}

	/**
	@brief release camera
	@return int
	*/
	int GenCameraXIMEA::release() {
		if (this->isCapture == true)
			this->stopCapture();
		// close cameras
		for (size_t i = 0; i < this->cameraNum; i++) {
			checkXIMEAErrors(xiCloseDevice(hcams[i]));
		}
		return 0;
	}

	/**
	@brief get camera information
	@param std::vector<GenCamInfo> & camInfos: output camera infos
	@return int
	*/
	int GenCameraXIMEA::getCamInfos(std::vector<GenCamInfo> & camInfos) {
		camInfos.resize(this->cameraNum);
		for (size_t i = 0; i < this->cameraNum; i++) {
			char sn[20] = "";
			xiGetParamString(hcams[i], XI_PRM_DEVICE_SN, sn, sizeof(sn));
			camInfos[i].sn = std::string(sn);
			int width, height;
			checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_WIDTH, &width));
			checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_HEIGHT, &height));
			camInfos[i].width = width;
			camInfos[i].height = height;
			float fps;
			int autoExposure, autoWhiteBalance;
			checkXIMEAErrors(xiGetParamFloat(hcams[i], XI_PRM_FRAMERATE, &fps));
			checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_AEAG, &autoExposure));
			checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_AUTO_WB, &autoWhiteBalance));
			camInfos[i].fps = fps;
			camInfos[i].autoExposure = static_cast<cam::Status>(autoExposure);
			checkXIMEAErrors(xiGetParamFloat(hcams[i], XI_PRM_WB_KR, &camInfos[i].redGain));
			checkXIMEAErrors(xiGetParamFloat(hcams[i], XI_PRM_WB_KG, &camInfos[i].greenGain));
			checkXIMEAErrors(xiGetParamFloat(hcams[i], XI_PRM_WB_KB, &camInfos[i].blueGain));
			this->getBayerPattern(i, camInfos[i].bayerPattern);
			camInfos[i].isWBRaw = false;
		}
		return 0;
	}

	/***********************************************************/
	/*                  camera setting functions               */
	/***********************************************************/
	/**
	@brief set frame rate
	@param float fps: input fps
	@param float exposureUpperLimitRatio: exposure upper limit time, make
		exposure upper limit time = 1000000us / fps * 0.8
	@return int
	*/
	int GenCameraXIMEA::setFPS(int camInd, float fps, float exposureUpperLimitRatio) {

		if(this->isCapture)
		{
			SysUtil::warningOutput("GenCameraXIMEA::setFPS Calling setFPS after StartCapture, framerate limit may not work");
		}

		// calculate max exposure time
		float maxExposureTime = 1000000.0f / fps * exposureUpperLimitRatio;

		if (camInd == -1) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FRAME_RATE_LIMIT));
				checkXIMEAErrors(xiSetParamFloat(hcams[i], XI_PRM_FRAMERATE, 
				static_cast<int>(fps * XIMEA_CAMERA_BANDWITH_REDUN)));
				checkXIMEAErrors(xiSetParamFloat(hcams[i], XI_PRM_AE_MAX_LIMIT, maxExposureTime));

				// int width, height;
				// checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_WIDTH, &width));
				// checkXIMEAErrors(xiGetParamInt(hcams[i], XI_PRM_HEIGHT, &height));
				// checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF));
				// checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LIMIT_BANDWIDTH_MODE, XI_OFF));
				// checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LIMIT_BANDWIDTH,
				//  static_cast<int>(width * height * fps * 8 / 1024.0 / 1024.0 * XIMEA_CAMERA_BANDWITH_REDUN)));
				// checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_LIMIT_BANDWIDTH_MODE, XI_ON));
			}
		}
		else {
			checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FRAME_RATE_LIMIT));
			checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_FRAMERATE, 
				static_cast<int>(fps * XIMEA_CAMERA_BANDWITH_REDUN)));
			checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_AE_MAX_LIMIT, maxExposureTime));

			// int width, height;
			// checkXIMEAErrors(xiGetParamInt(hcams[camInd], XI_PRM_WIDTH, &width));
			// checkXIMEAErrors(xiGetParamInt(hcams[camInd], XI_PRM_HEIGHT, &height));
			// checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF));
			// checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_LIMIT_BANDWIDTH_MODE, XI_OFF));
			// checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_LIMIT_BANDWIDTH,
			// 	 static_cast<int>(width * height * fps * 8 / 1024.0 / 1024.0 * XIMEA_CAMERA_BANDWITH_REDUN)));
			// checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_LIMIT_BANDWIDTH_MODE, XI_ON));
		}
		return 0;
	}

	/**
	@brief set white balance
	@param int ind: index of camera (-1 means all the cameras)
	@param Status autoWB: if use auto white balance
	@return int
	*/
	int GenCameraXIMEA::setAutoWhiteBalance(int camInd) {
		if (camInd == -1) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_MANUAL_WB,
					1));
				checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
				checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
			}
		}
		else {
			checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_AUTO_WB,
				1));
			checkXIMEAErrors(xiGetImage(hcams[camInd], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[camInd]));
			checkXIMEAErrors(xiGetImage(hcams[camInd], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[camInd]));
		}
		return 0;
	}

	/**
	@brief set auto white balance
	@param int ind: index of camera (-1 means all the cameras)
	@param float redGain: red gain of the white balance
	@param float greenGain: green gain of the white balance
	@param float blueGain: blue gain of the white balance
	@return int
	*/
	int GenCameraXIMEA::setWhiteBalance(int camInd, float redGain,
		float greenGain, float blueGain) {
		if (camInd == -1) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiSetParamFloat(hcams[i], XI_PRM_WB_KR, redGain));
				checkXIMEAErrors(xiSetParamFloat(hcams[i], XI_PRM_WB_KG, greenGain));
				checkXIMEAErrors(xiSetParamFloat(hcams[i], XI_PRM_WB_KB, blueGain));
				checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
				checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
			}
		}
		else {
			checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_WB_KR, redGain));
			checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_WB_KG, greenGain));
			checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_WB_KB, blueGain));
			checkXIMEAErrors(xiGetImage(hcams[camInd], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[camInd]));
			checkXIMEAErrors(xiGetImage(hcams[camInd], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[camInd]));
		}
		return 0;
	}

	/**
	@brief set exposure time
	@param int ind: index of camera (-1 means all the cameras)
	@param Status autoExposure: if use auto exposure
	@return int
	*/
	int GenCameraXIMEA::setAutoExposure(int camInd, Status autoExposure) {
		if (camInd == -1) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_AEAG, 
					static_cast<int>(autoExposure)));
			}
		}
		else
			checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_AEAG, 
				static_cast<int>(autoExposure)));
		
		return 0;
	}

	/**
	@brief set auto exposure level
	@param int ind: index of camera (-1 means all the cameras)
	@param float level: auto exposure level, average intensity of output
	signal AEAG should achieve
	@return int
	*/
	int GenCameraXIMEA::setAutoExposureLevel(int camInd, float level) {
		if (camInd == -1) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiSetParamFloat(hcams[i], XI_PRM_AEAG_LEVEL,
					level));
				char info[256];
				sprintf(info, "XIMEA camera %d, exposure level set to %f%% ", i, level);
				SysUtil::infoOutput(info);
			}
			for (size_t frame = 0; frame < 2; frame++) {
				for (size_t i = 0; i < this->cameraNum; i++) {
					checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
				}
			}
		}
		else {
			checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_AEAG_LEVEL,
				level));
			char info[256];
			sprintf(info, "XIMEA camera %d, exposure level set to %f%%", camInd, level);
			SysUtil::infoOutput(info);
			for (size_t frame = 0; frame < 2; frame++) {
				checkXIMEAErrors(xiGetImage(hcams[camInd], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[camInd]));
			}
		}

		// for (size_t frame = 0; frame < 2; frame++) {
		// 	for (size_t i = 0; i < this->cameraNum; i++) {
		// 		checkXIMEAErrors(xiGetImage(hcams[i], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[i]));
		// 	}
		// }
		return 0;
	}

	/**
	@brief set auto exposure compensation (only support PointGrey cameras)
	@param int ind: index of camera (-1 means all the cameras)
	@param Status status: if use auto EV value
	@param float relativeEV: only valid when the second argument is off.
	The reason why use relative EV value here is to directly set a absolute
	value is difficult
	@return int
	*/
	int GenCameraXIMEA::setAutoExposureCompensation(int camInd,
		Status status, float relativeEV) {
		SysUtil::warningOutput("setAutoExposureCompensation function is not "\
			" support for XIMEA camera. \n"\
			"Please use setAutoExposureLevel instead !");
		return 0;
	}

	/**
	@brief set brightness time
	@param int camInd: index of camera
	@param int brightness: input brightness
	+1: brighten, -1: darken, 0: do nothing
	@return int
	*/
	int GenCameraXIMEA::adjustBrightness(int camInd, int brightness) {
		float level, relativeLevel;
		checkXIMEAErrors(xiGetParamFloat(hcams[camInd], XI_PRM_AEAG_LEVEL, &level));
		if (brightness == 1)
			relativeLevel = 5;
		else if (brightness == -1)
			relativeLevel = -5;
		checkXIMEAErrors(xiSetParamFloat(hcams[camInd], XI_PRM_AEAG_LEVEL,
			level + relativeLevel));
		char info[256];
		sprintf(info, "XIMEA camera %d, exposure level set to %f%%", camInd, level);
		SysUtil::infoOutput(info);
		return 0;
	}

	/**
	@brief set exposure time
	@param int camInd: index of camera (-1 means all the cameras)
	@param int time: exposure time (in microseconds)
	@return int
	*/
	int GenCameraXIMEA::setExposure(int camInd, int time) {
		if (camInd == -1) {
			for (size_t i = 0; i < this->cameraNum; i++) {
				checkXIMEAErrors(xiSetParamInt(hcams[i], XI_PRM_EXPOSURE, time));
			}
		}
		else
			checkXIMEAErrors(xiSetParamInt(hcams[camInd], XI_PRM_EXPOSURE, time));
		return 0;
	}

	/**
	@brief set/get bayer pattern
	@param int camInd: input camera index
	@param GenCamBayerPattern & bayerPattern: output bayer pattern
	@return int
	*/
	int GenCameraXIMEA::getBayerPattern(int camInd, GenCamBayerPattern & bayerPattern) {
		int cfa;
		xiGetParamInt(hcams[camInd], XI_PRM_COLOR_FILTER_ARRAY, &cfa);
		switch(cfa) {
			case XI_CFA_BAYER_RGGB: bayerPattern = GenCamBayerPattern::BayerRGGB; break;
			case XI_CFA_BAYER_BGGR: bayerPattern = GenCamBayerPattern::BayerBGGR; break;
			case XI_CFA_BAYER_GRBG: bayerPattern = GenCamBayerPattern::BayerGRBG; break;
			case XI_CFA_BAYER_GBRG: bayerPattern = GenCamBayerPattern::BayerGBRG; break;
			default: 
				char info[256];
				sprintf(info, "Camera index %d unknown cfa bayer pattern ! ", camInd);
				SysUtil::errorOutput(std::string(info));
				exit(-1);
			break;
		}
		return 0;
	}

	/*************************************************************/
	/*                     capturing function                    */
	/*************************************************************/
	/**
	@brief capture single image of single camera in camera array
	@param int camInd: input index of camera
	@param Imagedata & img: output captured images
	@return int
	*/
	int GenCameraXIMEA::captureFrame(int camInd, Imagedata & img) {
		// capture images
		checkXIMEAErrors(xiGetImage(hcams[camInd], XIMEA_CAMERA_GET_IMAGE_WAIT, &xiImages[camInd]));
		// copy to opencv mat
		std::memcpy(img.data, xiImages[camInd].bp, sizeof(unsigned char) *
			xiImages[camInd].width * xiImages[camInd].height);
		return 0;
	}

}
