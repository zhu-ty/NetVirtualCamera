/**
@brief Generic Camera Driver Class
Implementation of PointGrey camera
@author Shane Yuan
@date Dec 31, 2017
*/

#ifndef __GENERIC_CAMERA_DRIVER_PTGREY_H__
#define __GENERIC_CAMERA_DRIVER_PTGREY_H__

// pointgrey spinnaker sdk
#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <memory.h>

#include "GenCameraDriver.h"
#include "RealCameraDriver.h"

namespace cam {

	// function to check XIMEA function error
	void checkPTGREYInternal(int result, char const *const func,
		const char *const file, int const line);

	// pointgrey function safe call
#define checkPTGREYErrors(val)  checkPTGREYInternal ( (val), #val, __FILE__, __LINE__ )

	class GenCameraPTGREY : public RealCamera {
	private:
		Spinnaker::SystemPtr sysPtr;
		Spinnaker::CameraList camList;
		std::vector<Spinnaker::ImagePtr> ptgreyImages;
	public:

	private:


	public:
		GenCameraPTGREY();
		~GenCameraPTGREY();

		/***********************************************************/
		/*                   basic camera functions                */
		/***********************************************************/

		int genSettingInterface(int value1, std::string value2) override { return -2; };

		/**
		@brief init camera
		@return int
		*/
		int init() override;

		/**
		@brief set camera sync type
		@return int
		*/
		int setSyncType(GenCamSyncType type) override;

		/**
		@brief start capture images
		@return int
		*/
		int startCapture() override;

		/**
		@brief stop capture images
		@return int
		*/
		int stopCapture() override;

		/**
		@brief release camera
		@return int
		*/
		int release() override;

		/**
		@brief get camera information
		@param std::vector<GenCamInfo> & camInfos: output camera infos
		@return int
		*/
		int getCamInfos(std::vector<GenCamInfo> & camInfos) override;

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
		int setFPS(int camInd, float fps, float exposureUpperLimitRatio = 0.8) override;

		/**
		@brief set auto white balance
		@param int ind: index of camera (-1 means all the cameras)
		@return int
		*/
		int setAutoWhiteBalance(int camInd) override;

		/**
		@brief set auto white balance
		@param int ind: index of camera (-1 means all the cameras)
		@param float redGain: red gain of the white balance
		@param float greenGain: green gain of the white balance
		@param float blueGain: blue gain of the white balance
		@return int
		*/
		int setWhiteBalance(int camInd, float redGain,
			float greenGain, float blueGain) override;

		/**
		@brief set auto exposure
		@param int ind: index of camera (-1 means all the cameras)
		@param Status autoExposure: if use auto exposure
		@return int
		*/
		int setAutoExposure(int camInd, Status autoExposure) override;

		/**
		@brief set auto exposure level
		@param int ind: index of camera (-1 means all the cameras)
		@param float level: auto exposure level, average intensity of output
		signal AEAG should achieve
		@return int
		*/
		int setAutoExposureLevel(int camInd, float level) override;

		/**
		@brief set auto exposure compensation (only support PointGrey cameras)
		@param int ind: index of camera (-1 means all the cameras)
		@param Status status: if use auto EV value
		@param float relativeEV: only valid when the second argument is off.
		The reason why use relative EV value here is to directly set a absolute
		value is difficult
		@return int
		*/
		int setAutoExposureCompensation(int camInd,
			Status status, float relativeEV) override;

		/**
		@brief set brightness time
		@param int camInd: index of camera
		@param int brightness: input brightness
		+1: brighten, -1: darken, 0: do nothing
		@return int
		*/
		int adjustBrightness(int camInd, int time) override;

		/**
		@brief set exposure time
		@param int ind: index of camera (-1 means all the cameras)
		@param int time: exposure time (in microseconds)
		@return int
		*/
		int setExposure(int camInd, int time) override;

		/**
		@brief set/get bayer pattern
		@param int camInd: input camera index
		@param GenCamBayerPattern & bayerPattern: output bayer pattern
		@return int
		*/
		int getBayerPattern(int camInd, GenCamBayerPattern & bayerPattern) override;

		/**
		@brief make setting effective
		by capturing some frames
		@param int k: capture image frames (default is 10)
		@return int
		*/
		int makeSetEffective(int k = 10) override;

		/*************************************************************/
		/*                     capturing function                    */
		/*************************************************************/
		/**
		@brief capture single image of single camera in camera array
		@param int camInd: input index of camera
		@param Imagedata & img: output captured images (pre-allocated memory)
		@return int
		*/
		int captureFrame(int camInd, Imagedata & img) override;
	};

};

#endif
