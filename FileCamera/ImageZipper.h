/**
@brief  Depth Zipper Writer/Reader Class
		Image will be compressed into a zip file without loss.
@author zhu-ty
@date Apr 25, 2019
*/


#ifndef _DEPTH_ZIPPER_DEPTH_ZIPPER_H_
#define _DEPTH_ZIPPER_DEPTH_ZIPPER_H_

#include <opencv2/opencv.hpp>
#define MAX_FRAME_BUFFER_SIZE 5 // 10 ^ size , 5 means 10000

class ImageZipperWriter
{
public:
	ImageZipperWriter() {};
	~ImageZipperWriter() {};
	enum class WriteType
	{
		Create = 0, 
		Append = 1
	};

	int init(std::string fileName, WriteType type = WriteType::Create);
	int append(std::vector<cv::Mat> &imgs);
	int append(cv::Mat &img);
	int release();
private:
	int _frame; //Next frame to write
	//zip_t *_zip = nullptr;
	void *_zip = nullptr;
};

class ImageZipperReader
{
public:
	ImageZipperReader() {};
	~ImageZipperReader() {};
	int init(std::string fileName);
	int init(std::string fileName, int &maxFrameNum);
	int getMaxFrameNum();
	cv::Mat read(int frameNum);
	cv::Mat read(int frameNum, std::string &fileName);
	int release();

private:
	void *_zip = nullptr;
	std::vector<std::string> fileList;

};


#endif //_DEPTH_ZIPPER_DEPTH_ZIPPER_H_