/**
@brief Generic Camera Driver Class
@author Shane Yuan
@date Dec 29, 2017
*/

#include "GenCameraDriver.h"
#include <time.h>
#include <algorithm>
#include <functional>   // std::minus
#include <numeric>      // std::accumulate
#include <fstream>

// include NPPJpegCoder
#include "NPPJpegCoder.h"
//cuda
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>

namespace cam {

	GenCamera::GenCamera() : isInit(false), isCapture(false),
		isVerbose(false), bufferType(GenCamBufferType::Raw),
		camPurpose(GenCamCapturePurpose::Streaming),
		JPEGQuality(75), sizeRatio(0.12) {}
	GenCamera::~GenCamera() {}

	/**
	@brief set verbose
	@param bool isVerbose: true, verbose mode, output many infomations
	for debugging
	@return int
	*/
	int GenCamera::setVerbose(bool isVerbose) {
		this->isVerbose = isVerbose;
		return 0;
	}

	/**
	@brief set buffer type
	@param GenCamBufferType type: buffer type
	@return int
	*/
	int GenCamera::setCamBufferType(GenCamBufferType type) {
		this->bufferType = type;
		return 0;
	}

	/**
	@brief set jpeg compression quality
	@param int quality: JPEG compression quality (1 - 100)
	@param float sizeRatio: expected compression ratio used for
	pre-malloc memory
	@return int
	*/
	int GenCamera::setJPEGQuality(int quality, float sizeRatio) {
		this->JPEGQuality = quality;
		this->sizeRatio = sizeRatio;
		return 0;
	}

	/**
	@brief set capture purpose
	@param GenCamCapturePurpose camPurpose: purpose, for streaming or recording
	@return int
	*/
	int GenCamera::setCapturePurpose(GenCamCapturePurpose camPurpose) {
		this->camPurpose = camPurpose;
		return 0;
	}

	/*************************************************************/
	/*        function to save capture images to files           */
	/*************************************************************/
	/**
	@brief save captured images to dir
	@param std::string dir: input dir to save images
	@return int
	*/
	int GenCamera::saveImages(std::string dir) {
		if (this->bufferType == GenCamBufferType::JPEG) {
			SysUtil::mkdir(dir);
			for (size_t i = 0; i < this->cameraNum; i++) {
				for (size_t j = 0; j < this->bufferSize; j++) {
					char outname[256];
					sprintf(outname, "%s/%02d_%05d.jpg", dir.c_str(), i, j);
					std::ofstream outputFile(outname, std::ios::out | std::ios::binary);
					outputFile.write(reinterpret_cast<const char*>(this->bufferImgs[j][i].data),
						this->bufferImgs[j][i].length);
				}
			}
		}
		else {
			SysUtil::errorOutput("Sorry, save function for other buffer types is not support yet. ");
			exit(-1);
		}
		return 0;
	}

	/**
	@brief save captured videos to dir
	@param std::string dir: input dir to save videos
	@return int
	*/
	int GenCamera::saveVideos(std::string dir) {
		if (this->bufferType == GenCamBufferType::JPEG) {
			SysUtil::mkdir(dir);
			for (size_t i = 0; i < this->cameraNum; i++) {
				// init npp jpeg coder
				npp::NPPJpegCoder coder;
				coder.init(camInfos[i].width, camInfos[i].height, JPEGQuality);
				// init video parameter
				std::string videoname = cv::format("%s/cam_%02d.avi", dir.c_str(), i);
				cv::VideoWriter writer(videoname, cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 
					camInfos[i].fps, cv::Size(camInfos[i].width, camInfos[i].height), true);
				cv::cuda::GpuMat img_d(camInfos[i].height, camInfos[i].width, CV_8UC3);
				cv::Mat img(camInfos[i].height, camInfos[i].width, CV_8UC3);
				for (size_t j = 0; j < this->bufferSize; j++) {
					coder.decode(reinterpret_cast<uchar*>(this->bufferImgs[j][i].data), 
						this->bufferImgs[j][i].length,
						img_d, 0);
					img_d.download(img);
					writer << img;
				}
				writer.release();
				// release npp jpeg coder
				coder.release();
			}
		}
		else {
			SysUtil::errorOutput("Sorry, save function for other buffer types is not support yet. ");
			exit(-1);
		}
		return 0;
	}

	/*************************************************************/
	/*   function to set jepg scale ratio for capture function   */
	/*************************************************************/
	/**
	@brief set scale ratio vector of capture function
	@param std::vector<GenCamImgRatio> imgRatios: input scale ratio vector
	@return int
	*/
	int GenCamera::setImageRatios(std::vector<GenCamImgRatio> imgRatios)
	{
		this->imgRatios = imgRatios;
		return 0;
	}

	/**
	@brief make image size even
	@param cv::Size size: input size
	@param cam::GenCamImgRatio ratio: input resize ratio
	@return cv::Size: even size (NPP only accept even size)
	*/
	cv::Size GenCamera::makeDoubleSize(cv::Size size, cam::GenCamImgRatio ratio) {
		cv::Size out;
		float r = 1.0f / powf(2.0f, static_cast<int>(ratio));
		out.width = static_cast<int>(size.width * r);
		out.height = static_cast<int>(size.height * r);
		out.width += (out.width % 2);
		out.height += (out.height % 2);
		return out;
	}

	/*************************************************************/
	/*    function to set mapping vector of capture function     */
	/*************************************************************/
	/**
	@brief set mapping vector of capture function
	@param std::vector<size_t> mappingVector: input mapping vector
	@return int
	*/
	int GenCamera::setMappingVector(std::vector<size_t> mappingVector) {
		this->mappingVector = mappingVector;
		return 0;
	}

	/**
	@brief capture one frame
	@param std::vector<Imagedata> & imgs: output captured images
	if in single mode, memory of image mats should be malloced
	before using this function
	@return int
	*/
	int GenCamera::captureFrame(std::vector<Imagedata> & imgs) {
		if (captureMode == GenCamCaptureMode::Continous ||
			captureMode == GenCamCaptureMode::ContinousTrigger) {
			// get images from buffer
			for (size_t camInd = 0; camInd < this->cameraNum; camInd++) {
				int index = (thBufferInds[camInd] - 1 + bufferSize) % bufferSize;
				imgs[camInd] = bufferImgs[index][camInd];
			}

		}
		else if (captureMode == GenCamCaptureMode::Single ||
			captureMode == GenCamCaptureMode::SingleTrigger) {
			SysUtil::errorOutput("Single mode is not implemented yet !");
			exit(-1);
		}
		return 0;
	}

	/**
	@brief capture one frame with Mapping
	@param std::vector<Imagedata> & imgs: output captured images
	if in single mode, memory of image mats should be malloced
	before using this function
	@return int
	*/
	int GenCamera::captureFrameWithMapping(std::vector<Imagedata> & imgs) {
		size_t camInd;
		if (captureMode == GenCamCaptureMode::Continous ||
			captureMode == GenCamCaptureMode::ContinousTrigger) {
			// get images from buffer
			for (size_t i = 0; i < this->cameraNum; i++) {
				camInd = mappingVector[i];
				int index = (thBufferInds[camInd] - 1 + bufferSize) % bufferSize;
				imgs[i] = bufferImgs[index][camInd];
			}

		}
		else if (captureMode == GenCamCaptureMode::Single ||
			captureMode == GenCamCaptureMode::SingleTrigger) {
			SysUtil::errorOutput("Single mode is not implemented yet !");
			exit(-1);
		}
		return 0;
	}

	/**
	@brief get camera infos list with mapping
	@param std::vector<cam::GenCamInfo> & camInfos: output camera info list
	@return int
	*/
	int GenCamera::getCameraInfoListsWithMapping(std::vector<cam::GenCamInfo> & camInfos) {
		size_t camInd;
		// get camera infos without mapping
		std::vector<cam::GenCamInfo> camInfosWoMapping;
		this->getCamInfos(camInfosWoMapping);
		camInfos.clear();
		camInfos.resize(this->cameraNum);
		for (size_t i = 0; i < this->cameraNum; i++) {
			camInd = mappingVector[i];
			camInfos[i] = camInfosWoMapping[camInd];
		}
		return 0;
	}

	/*************************************************************/
	/*                function to capture images                 */
	/*************************************************************/
	/**
	@brief capture one frame
	@param std::vector<Imagedata> & refImgs: output reference images
	@param std::vector<Imagedata> & localImgs: output localview images
	@param std::vector<int> refInds: input reference indices
	@param std::vector<int> localInds: input local indices
	@return int
	*/
	int GenCamera::captureFrame(std::vector<Imagedata> & refImgs,
		std::vector<Imagedata> & localImgs,
		std::vector<int> refInds,
		std::vector<int> localInds) {
		size_t camInd;
		if (captureMode == GenCamCaptureMode::Continous ||
			captureMode == GenCamCaptureMode::ContinousTrigger) {
			// get refernce images from buffer
			for (size_t i = 0; i < refInds.size(); i++) {
				camInd = refInds[i];
				int index = (thBufferInds[camInd] - 1 + bufferSize) % bufferSize;
				refImgs[i] = bufferImgs[index][camInd];
			}
			// get local images from buffer
			for (size_t i = 0; i < localInds.size(); i++) {
				camInd = localInds[i];
				int index = (thBufferInds[camInd] - 1 + bufferSize) % bufferSize;
				localImgs[i] = bufferImgs[index][camInd];
			}
			// increase buffer indices for file camera
			if (this->camModel == cam::CameraModel::File) {
				for (size_t camInd = 0; camInd < this->cameraNum; camInd++) {
					thBufferInds[camInd] = (thBufferInds[camInd] + 1) % bufferSize;
				}
			}
		}
		else if (captureMode == GenCamCaptureMode::Single ||
			captureMode == GenCamCaptureMode::SingleTrigger) {
			SysUtil::errorOutput("Single mode is not implemented yet !");
			exit(-1);
		}
		return 0;
	}
}

