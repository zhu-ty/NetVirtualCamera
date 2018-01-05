/**
@brief cuda source file of camera utility class
@author: Shane Yuan
@date: Sep 1, 2017
*/

#include "CameraUtilKernel.h"

/**
@brief cuda demosaicing kernel function
@param cv::cuda::PtrStep<uchar> bayerImg: input bayerImg
@param cv::cuda::PtrStep<uchar3> bgrImg: output color image
@param int width: image width
@param int height: image height
*/
__global__ void CameraUtilKernel::demosaic(cv::cuda::PtrStep<uchar> bayerImg,
	cv::cuda::PtrStep<uchar3> bgrImg,
	int width, int height) {
	// get thread position
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	//
	if (x < width / 2 && y < height / 2) {
		int index_row1 = 2 * y * width + 2 * x;
		int index_row2 = index_row1 + width;

		uchar r = bayerImg.ptr(2 * y)[2 * x];
		uchar g1 = bayerImg.ptr(2 * y)[2 * x + 1];
		uchar g2 = bayerImg.ptr(2 * y + 1)[2 * x];	
		uchar b = bayerImg.ptr(2 * y + 1)[2 * x + 1];

		uchar3 g1_uchar3 = make_uchar3(b, g1, r);
		uchar3 r_uchar3 = make_uchar3(b, g1, r);
		uchar3 b_uchar3 = make_uchar3(b, g2, r);
		uchar3 g2_uchar3 = make_uchar3(b, g2, r);

		bgrImg.ptr(2 * y)[2 * x] = g1_uchar3;
		bgrImg.ptr(2 * y)[2 * x + 1] = r_uchar3;
		bgrImg.ptr(2 * y + 1)[2 * x] = b_uchar3;
		bgrImg.ptr(2 * y + 1)[2 * x + 1] = g2_uchar3;
	}
}


/**
@brief demosaic function
@param cv::Mat bayerImg: input bayer image
@return cv::Mat bgrImg: demosaic result
*/
cv::Mat CameraUtilKernel::demosaic(cv::Mat bayerImg) {
	cv::cuda::GpuMat bayerImg_d;
	bayerImg_d.upload(bayerImg);
	cv::cuda::GpuMat gpuImg(bayerImg.rows, bayerImg.cols, CV_8UC3);
	dim3 dimBlock(32, 32);
	dim3 dimGrid((bayerImg.cols / 2 + dimBlock.x - 1) / dimBlock.x,
		(bayerImg.rows / 2 + dimBlock.y - 1) / dimBlock.y);
	CameraUtilKernel::demosaic << <dimGrid, dimBlock >> >(bayerImg_d, gpuImg, bayerImg.cols, bayerImg.rows);
	cv::Mat bgrImg;
	gpuImg.download(bgrImg);
	return bgrImg;
}


