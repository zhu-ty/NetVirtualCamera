/**
@brief Generic Camera Driver Class
Implementation of XIMEA camera
@author Shane Yuan
@date Dec 31, 2017
*/

#include "NetVirtualCamera.h"

namespace cam {

	// constructor
	GenCameraNETVIR::GenCameraNETVIR() {
		this->camModel = CameraModel::Network;
	}
	GenCameraNETVIR::~GenCameraNETVIR() {}

	/***********************************************************/
	/*                   basic camera functions                */
	/***********************************************************/
	/**
	@brief make setting effective
	by capturing some frames
	@param int k: capture image frames (default is 10)
	@return int
	*/
	int GenCameraNETVIR::makeSetEffective(int k) {
		if (!this->isCapture) {
			SysUtil::warningOutput("This function must be executed after " \
				"starting capturing !");
			return -1;
		}
		SysUtil::errorOutput("NetVirtualCamera::makeSetEffective Function unused");
		return -1;
	}

	/**
	@brief init camera
	@return int
	*/
	int GenCameraNETVIR::init() {
		//TODO
		ths.resize(this->cameraNum);
		thStatus.resize(this->cameraNum);
		this->isInit = true;
		return 0;
	}

	/**
	@brief get camera information
	@param std::vector<GenCamInfo> & camInfos: output camera infos
	@return int
	*/
	int GenCameraNETVIR::getCamInfos(std::vector<GenCamInfo> & camInfos) {
		camInfos.resize(this->cameraNum);
		//TODO
		return 0;
	}

	/**
	@brief start capture images
	@return int
	*/
	int GenCameraNETVIR::startCapture() {
		SysUtil::errorOutput("NetVirtualCamera::startCapture Function unused");
		return -1;
		this->isCapture = true;
		return 0;
	}

	/**
	@brief stop capture images
	@return int
	*/
	int GenCameraNETVIR::stopCapture() {
		SysUtil::errorOutput("NetVirtualCamera::stopCapture Function unused");
		return -1;
		this->isCapture = false;
		return 0;
	}

	/**
	@brief release camera
	@return int
	*/
	int GenCameraNETVIR::release() {
		SysUtil::errorOutput("NetVirtualCamera::release Function unused");
		return -1;
		if (this->isCapture == true)
			this->stopCapture();
		return 0;
	}

	/***********************************************************/
	/*                  camera setting functions               */
	/***********************************************************/
	/**
	@brief set frame rate
	@param float fps: input fps
	@return int
	*/
	int GenCameraNETVIR::setFPS(int camInd, float fps) {
		SysUtil::errorOutput("NetVirtualCamera::setFPS Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto white balance
	@param int ind: index of camera (-1 means all the cameras)
	@return int
	*/
	int GenCameraNETVIR::setAutoWhiteBalance(int camInd) {
		SysUtil::errorOutput("NetVirtualCamera::setAutoWhiteBalance Function unused");
		return -1;
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
	int GenCameraNETVIR::setWhiteBalance(int camInd, float redGain,
		float greenGain, float blueGain) {
		SysUtil::errorOutput("NetVirtualCamera::setWhiteBalance Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto exposure
	@param int ind: index of camera (-1 means all the cameras)
	@param Status autoExposure: if use auto exposure
	@return int
	*/
	int GenCameraNETVIR::setAutoExposure(int camInd, Status autoExposure) {
		SysUtil::errorOutput("NetVirtualCamera::setAutoExposure Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto exposure level
	for pointgrey camera auto exposure level is adjusted by EV
	@param int ind: index of camera (-1 means all the cameras)
	@param float level: auto exposure level, average intensity of output
	signal AEAG should achieve
	@return int
	*/
	int GenCameraNETVIR::setAutoExposureLevel(int camInd, float level) {
		SysUtil::errorOutput("NetVirtualCamera::setAutoExposureLevel Function unused");
		return -1;
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
	int GenCameraNETVIR::setAutoExposureCompensation(int camInd,
		Status status, float relativeEV) {
		SysUtil::errorOutput("NetVirtualCamera::setAutoExposureCompensation Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set exposure time
	@param int ind: index of camera (-1 means all the cameras)
	@param int time: exposure time (in microseconds)
	@return int
	*/
	int GenCameraNETVIR::setExposure(int camInd, int time) {
		SysUtil::errorOutput("NetVirtualCamera::setExposure Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set/get bayer pattern
	@param int camInd: input camera index
	@param GenCamBayerPattern & bayerPattern: output bayer pattern
	@return int
	*/
	int GenCameraNETVIR::getBayerPattern(int camInd, GenCamBayerPattern & bayerPattern) {
		//TODO
		SysUtil::warningOutput("NetVirtualCamera::getBayerPattern TODO!");
		bayerPattern = GenCamBayerPattern::BayerRGGB;
		return 0;
	}

	/*************************************************************/
	/*                     capturing function                    */
	/*************************************************************/
	/**
	@brief capture single image of single camera in camera array
	@param int camInd: input index of camera
	@param cv::Mat & img: output captured images (pre-allocated memory)
	@return int
	*/
	int GenCameraNETVIR::captureFrame(int camInd, Imagedata & img) {
		//TODO
		return 0;
	}
	/**
	@brief capture single image of single camera in camera array (raw data)
	@param int camInd: input index of camera
	@param Imagedata & img: output captured images
	@return int
	*/
	int GenCameraNETVIR::captureFrame(std::vector<Imagedata> & imgs) {
		//TODO
		return 0;
	}
}