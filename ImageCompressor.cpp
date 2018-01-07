/**
@brief Generic Camera Driver Class
Image compressor class, JPEG
@author Shane Yuan
@date Jan 07, 2018
*/

#include "ImageCompressor.h"

ImageCompressor::ImageCompressor() {}
ImageCompressor::~ImageCompressor() {}

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
int ImageCompressor::init(std::vector<cam::GenCamInfo> camInfos,
	std::vector<char*> rawdatas,
	std::vector<cam::Imagedata> compressdatas,
	int* thStatus) {
	this->cameraNum = camInfos.size();
	this->rawdatas = rawdatas;
	this->compressdatas = compressdatas;
	this->thStatus = thStatus;
	return 0;
}