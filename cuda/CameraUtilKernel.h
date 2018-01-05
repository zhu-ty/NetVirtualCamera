/**
@brief cuda header file of camera utility class
@author: Shane Yuan
@date: Sep 1, 2017
*/

#ifndef __CAMERA_UTIL_KERNEL_H__
#define __CAMERA_UTIL_KERNEL_H__

// basic 
#include <iostream>
#include <cstdlib>
#include <fstream>

// C++ 11 parallel 
#include <thread>

// opencv
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/cuda_stream_accessor.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>

// cuda
#ifdef _WIN32
#include <windows.h>
#endif
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <device_launch_parameters.h>
#include <surface_functions.h>


namespace CameraUtilKernel {
	/*************************************************************************/
	/*                            GPU Kernel function                        */
	/*************************************************************************/
	/**
	@brief cuda demosaicing kernel function
	@param cv::cuda::PtrStep<uchar> bayerImg: input bayerImg
	@param cv::cuda::PtrStep<uchar3> bgrImg: output color image
	@param int width: image width
	@param int height: image height
	*/
	__global__ void demosaic(cv::cuda::PtrStep<uchar> bayerImg, cv::cuda::PtrStep<uchar3> bgrImg,
		int width, int height);

	/*************************************************************************/
	/*                            CPU Host function                          */
	/*************************************************************************/
	/**
	@brief demosaic function
	@param cv::Mat bayerImg: input bayer image
	@return cv::Mat bgrImg: demosaic result
	*/
	cv::Mat demosaic(cv::Mat bayerImg);

}

#endif