/**
@brief Generic Camera Driver Class
Image compressor class, JPEG
@author Shane Yuan
@date Jan 07, 2018
*/

#ifndef __GENERIC_CAMERA_DRIVER_COMPRESSOR_HPP__
#define __GENERIC_CAMERA_DRIVER_COMPRESSOR_HPP__

// include std
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <thread>
#include <memory>

// opencv
#include <opencv2/opencv.hpp>

// cuda
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

// Generic camera driver
#include "GenCameraDriver.h"

// cuda npp JPEG coder
#include "NPPJpegCoder.h"

class ImageCompressor {
private:
	int cameraNum;
	std::vector<cam::GenCamInfo> camInfos;
	std::vector<char*> rawdatas;
	std::vector<cam::Imagedata> compressdatas;
	int* thStatus;
public:

private:
	/**
	@brief image compression thread
	*/
	void compress_thread_();

public:
	ImageCompressor();
	~ImageCompressor();

	/**
	@brief init function
	@param std::vector<cam::GenCamInfo> camInfos: input camera infos
	@param std::vector<char*> rawdatas: input pointer of raw images
			data pointer of images captured by cameras
	@param std::vector<cam::Imagedata> compressdatas: input pointer of 
			compressed jpeg images
	@param int* thStatus: input vector of thread status 
	@return int
	*/
	int init(std::vector<cam::GenCamInfo> camInfos, std::vector<char*> rawdatas,
		std::vector<cam::Imagedata> compressdatas, int* thStatus);



};


#endif