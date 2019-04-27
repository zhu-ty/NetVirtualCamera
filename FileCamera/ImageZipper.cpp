#include "ImageZipper.h"
#include "zip.h"
#include "SKCommon.hpp"

int ImageZipperWriter::init(std::string fileName, WriteType type)
{
	release();
	if (type == WriteType::Append)
	{
		zip_t *tmpZip = zip_open(fileName.c_str(), 0, 'r');
		int i, n = zip_total_entries(tmpZip);
		for (i = 0; i < n; ++i) 
		{
			zip_entry_openbyindex(tmpZip, i);
			const char *name_ = zip_entry_name(tmpZip);
			int isdir = zip_entry_isdir(tmpZip);
			if (!isdir)
			{
				std::string name(name_);
				int num = atoi(name.substr(0, name.find("..")).c_str());
				if (_frame < (num + 1))
					_frame = num + 1;
			}
			zip_entry_close(tmpZip);
		}
		zip_close(tmpZip);
	}
	else
		_frame = 0;
	_zip = zip_open(fileName.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, (type == WriteType::Create) ? 'w' : 'a');
	return 0;
}

int ImageZipperWriter::append(std::vector<cv::Mat> &imgs)
{
	if (_zip == nullptr)
	{
		SKCommon::errorOutput("Init DepthZipper First!");
		return -1;
	}
	for (int i = 0; i < imgs.size(); i++)
	{
		if (_frame > pow(10, MAX_FRAME_BUFFER_SIZE))
			SKCommon::warningOutput(SKCommon::format("max frame size reached, maxnimum = %d, frame count = %d", pow(10, MAX_FRAME_BUFFER_SIZE), _frame));
		//if (depths[i].type() != CV_16UC1)
		//	SKCommon::warningOutput(DEBUG_STRING + "depth.type != CV_16UC1");
		int type = imgs[i].type();
		uchar depth = type & CV_MAT_DEPTH_MASK;
		uchar chans = 1 + (type >> CV_CN_SHIFT);
		std::string formats, file;
		std::vector<uchar> pngdata;
		cv::Mat pngImage;
		if (depth == CV_32F && chans == 1)
		{
			pngImage = cv::Mat(imgs[i].rows, imgs[i].cols, CV_8UC4, (cv::Vec4b*)imgs[i].data);
			formats = SKCommon::format("%%0%dd..32fc1.png", MAX_FRAME_BUFFER_SIZE);
		}
		else
		{
			pngImage = imgs[i];
			formats = SKCommon::format("%%0%dd..png", MAX_FRAME_BUFFER_SIZE);

		}
		cv::imencode(".png", pngImage, pngdata);
		file = SKCommon::format(formats.c_str(), _frame);
		zip_entry_open((zip_t*)_zip, file.c_str());
		zip_entry_write((zip_t*)_zip, pngdata.data(), pngdata.size());
		zip_entry_close((zip_t*)_zip);
		SKCommon::infoOutput(file + " ziped.");
		_frame++;
	}
	return 0;
}

int ImageZipperWriter::append(cv::Mat &img)
{
	return append(std::vector<cv::Mat>{img});
}

int ImageZipperWriter::release()
{
	if (_zip != nullptr)
		zip_close((zip_t*)_zip);
	_zip = nullptr;
	return 0;
}





int ImageZipperReader::init(std::string fileName)
{
	int t;
	init(fileName, t);
	return 0;
}

int ImageZipperReader::init(std::string fileName, int &maxFrameNum)
{
	release();
	int _maxFrame = 0;
	_zip = zip_open(fileName.c_str(), 0, 'r');
	int i, n = zip_total_entries((zip_t*)_zip);
	for (i = 0; i < n; ++i) 
	{
		zip_entry_openbyindex((zip_t*)_zip, i);
		const char *name_ = zip_entry_name((zip_t*)_zip);
		std::string name(name_);
		int isdir = zip_entry_isdir((zip_t*)_zip);
		if (!isdir && name.substr(name.find_last_of(".")) == ".png")
		{
			int num = atoi(name.substr(0, name.find("..")).c_str());
			if (_maxFrame < (num + 1))
				_maxFrame = num + 1;
		}
		zip_entry_close((zip_t*)_zip);
	}
	maxFrameNum = _maxFrame;
	fileList.resize(_maxFrame);
	for (i = 0; i < n; ++i)
	{
		zip_entry_openbyindex((zip_t*)_zip, i);
		const char *name_ = zip_entry_name((zip_t*)_zip);
		std::string name(name_);
		int isdir = zip_entry_isdir((zip_t*)_zip);
		if (!isdir && name.substr(name.find_last_of(".")) == ".png")
		{
			int num = atoi(name.substr(0, name.find("..")).c_str());
			fileList[num] = name;
		}
		zip_entry_close((zip_t*)_zip);
	}
	return 0;
}

int ImageZipperReader::getMaxFrameNum()
{
	return fileList.size();
}

cv::Mat ImageZipperReader::read(int frameNum)
{
	std::string t;
	return read(frameNum, t);
}

cv::Mat ImageZipperReader::read(int frameNum, std::string & fileName)
{
	if (_zip == nullptr && frameNum >= fileList.size())
	{
		SKCommon::errorOutput("Invalid zip file or frameNum");
		return cv::Mat();
	}
	fileName = fileList[frameNum];
	void *buf;
	size_t bufsize;
	std::string file = fileList[frameNum];
	zip_entry_open((zip_t*)_zip, file.c_str());
	zip_entry_read((zip_t*)_zip, &buf, &bufsize);
	zip_entry_close((zip_t*)_zip);
	SKCommon::infoOutput(file + " read.");
	std::vector<uchar> bufs(bufsize);
	for (int i = 0; i < bufsize; i++)
		bufs[i] = ((uchar*)buf)[i];

	int st = fileList[frameNum].find("..") + 2;
	int ed = fileList[frameNum].find_last_of(".");
	if (ed - st <= 0)
		return cv::imdecode(bufs, cv::IMREAD_UNCHANGED);
	else if (SKCommon::toLower(fileList[frameNum].substr(st, ed - st)) == "32fc1")
	{
		cv::Mat pngImage = cv::imdecode(bufs, cv::IMREAD_UNCHANGED);
		cv::Mat floatImage(pngImage.rows, pngImage.cols, CV_32FC1, (float*)pngImage.data);
		return floatImage.clone();
	}
	else
		return cv::imdecode(bufs, cv::IMREAD_UNCHANGED);
}

int ImageZipperReader::release()
{
	if (_zip != nullptr)
		zip_close((zip_t*)_zip);
	_zip = nullptr;
	fileList.clear();
	return 0;
}
