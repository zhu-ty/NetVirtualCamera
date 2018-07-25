/********************************************************************
//Describe: camera.h: header file for camera.cpp
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/10/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
*********************************************************************/
#pragma once
#include "stdint.h"

//#include "GenCameraDriver.h"

#include <iostream>
#include <QMutex>

//#define CAMERA_COMMAND_DATA_MAX_SIZE  (8192+2048)					///通信命令数据包最大长度
#define CAMERA_COMMAND_DATA_MAX_SIZE  4096*8			///通信命令数据包最大长度
#define CAMERA_IMAGE_DATA_MAX_SIZE    (4096*4096*4+2048)		///图像数据最大长度

#define MAX_CAMERA_NUM 10
#define MAX_PARAM_NUM 10
#define MAX_PATH_LEN 256
#define MAX_SN_LEN MAX_PATH_LEN

struct GenCamInfoStruct
{
	char sn[MAX_SN_LEN];
	int width;
	int height;
	float fps;
	int autoExposure;
	int bayerPattern;
	float redGain;
	float greenGain;
	float blueGain;
	bool isWBRaw;
};


struct GenCameraControlData
{
	struct
	{
		int return_val;
	} void_func;
	struct
	{
		int return_val;
		GenCamInfoStruct camInfos[MAX_CAMERA_NUM];
	} caminfo_func;
	struct
	{
		int return_val;
		bool param_bool[MAX_PARAM_NUM];
		int param_enum[MAX_PARAM_NUM];
		int param_int[MAX_PARAM_NUM];
		float param_float[MAX_PARAM_NUM];
	} param_func;
	struct
	{
		int return_val;
		char str[MAX_PATH_LEN];
	} str_func;

};




typedef struct
{
	int32_t id_ = 0;										//对应真实的相机编号，1~35
	int32_t width_ = 2048;									//图像宽度：最大2048
	int32_t height_ = 2048;									//图像高度：最大2048
	int32_t exposure_ = 100;								//相机曝光：1~65535
	int32_t gain_ = 3;										//相机增益：1~255
	int32_t brightness_ = 32;                               //相机亮度：1~96 (x0.3125)
	int32_t contrast_ = 32;                                 //相机对比度：1~96 (x0.3125)
	int32_t bitLut_ = 8;                                    //输出灰度查找位：0~8 (bit[7+x:x])
	int32_t saveFormat_ = 1;								//保存格式：0:禁用 1:bmp 2:raw 3:bmp+raw
	int32_t skipNumber_ = 0;								//跳帧数：  0:noskip else: 1/(N+1)
	int32_t triggerMode_ = 1;								//触发模式：0：两路独立软触发或硬触发 1:两路同时软触发或硬触发，2：内部自由触发
	std::string savePath_ = "/home/tmp";					//linux下保存路径：/home/tmp/	最大长度为256
	std::string saveName_ = "";							    //保存的文件前缀： test			最大长度为128
	//std::string othersInfo_ = "Operator:zhangyang\nDate:2017-07-14 10:33\nExperimental purpose:observation rat brain.";                           //其他文件信息：操作人员，实验目的，日期 最大长度为1024
}CameraParametersUnitTypeDef;


///以下为保存参数文件、及与GUI通信的数据类型定义
typedef struct
{
	int32_t id_ = 0;										//对应真实的盒子编号，1~18			
	std::string mac_ = "00:11:1c:02:30:98";					//盒子的mac地址  固定长度为17+1
	int32_t cameraAmount_ = 2;								//盒子连接的相机数
	std::vector<CameraParametersUnitTypeDef> cameraVec_ = std::vector<CameraParametersUnitTypeDef>(2, CameraParametersUnitTypeDef());//各相机的配置参数

}CameraBoxUnitTypeDef;


typedef struct
{
	int32_t id_ = 0;										//对应真实的服务器编号，1~9
	bool connectedFlag_ = false;							//连接状态标志
	std::string ip_ = "10.8.5.199";					//服务器IP： 192.168.111.000 固定长度为15+1
	std::string port_ = "54321";							//服务器端口： 123456
	int32_t boxAmount_ = 1;									//服务器下连接的盒子数
	std::vector<CameraBoxUnitTypeDef> boxVec_ = std::vector<CameraBoxUnitTypeDef>(boxAmount_, CameraBoxUnitTypeDef());		//各盒子的配置参数

}CameraServerUnitTypeDef;


///以下为相机控制的命令与状态定义
enum Communication_Camera_Command
{
	Communication_Camera_Reset_Connection = 0,				//复位socket连接,用于本地，反馈为Get_Status
	Communication_Camera_Get_Status = 1,					//用于监测连接及设备状态,用于心跳包
	Communication_Camera_Open_Box = 2,						//同时设置图像宽度，高度，网络延迟以及路径
	Communication_Camera_Close_Box = 3,						//关闭盒子,再次启动需要重新初始化
	Communication_Camera_Open_Camera = 4,					//同时设置曝光、增益、存图格式、跳帧、触发模式
	Communication_Camera_Close_Camera = 5,					//以盒子为单位关闭相机
	Communication_Camera_Trigger_Continous = 6,				//连续触发,内部自由运行或外部触发信号
	Communication_Camera_Trigger_Single = 7,				//触发单张,从连续流中捕捉单张
	Communication_Camera_Reset_Id = 8,						//复位存图Id
	Communication_Camera_Get_Image = 9						//读取拍摄图
};

enum  Communication_Camera_Status
{
	Communication_Camera_Get_Status_Ok = 0,						//获取状态命令有效
	Communication_Camera_Get_Status_Invalid = 1,				//获取状态命令无效
	Communication_Camera_Open_Box_Ok = 2,						//打开盒子命令有效
	Communication_Camera_Open_Box_Invalid = 3,					//打开盒子命令无效
	Communication_Camera_Close_Box_Ok = 4,						//关闭盒子命令有效
	Communication_Camera_Close_Box_Invalid=5,					//关闭盒子命令无效
	Communication_Camera_Open_Camera_Ok = 6,					//打开相机命令有效
	Communication_Camera_Open_Camera_Invalid=7,					//打开相机命令无效
	Communication_Camera_Close_Camera_Ok = 8,					//关闭相机命令有效
	Communication_Camera_Close_Camera_Invalid = 9,				//关闭相机命令无效
	Communication_Camera_Trigger_Continous_Ok = 10,				//连续触发命令有效
	Communication_Camera_Trigger_Continous_Invalid = 11,		//连续触发命令无效
	Communication_Camera_Trigger_Single_Ok = 12,				//触发单张命令有效
	Communication_Camera_Trigger_Single_Invalid = 13,			//触发单张命令有效
	Communication_Camera_Reset_Id_Ok = 14,						//复位Id命令有效
	Communication_Camera_Reset_Id_Invalid = 15,					//复位Id命令有效
	Communication_Camera_Get_Image_Ok = 16,						//读取图片命令有效
	Communication_Camera_Get_Image_Invalid = 17,				//读取图片命令有效
	Communication_Camera_Action_Overtime = 18,					//相机控制动作命令超时
	Communication_Camera_Action_Invalid = 19					//相机控制动作命令超时
};


///**
//@brief struct to save image data
//both raw image data and jpeg compressed image data
//*/
//struct Imagedata {
//	char* data; // data pointer
//	size_t maxLength; // max malloced memory size
//	size_t length; // jpeg data length
//};

///该结构体用于GUI和其它线程向相机控制线程发送消息并接收反馈
class CameraControlMessage
{
public:
	int32_t requestorId_ = -1;
	Communication_Camera_Command command_ = Communication_Camera_Open_Box;
	Communication_Camera_Status status_ = Communication_Camera_Open_Box_Invalid;
	int32_t serverIndex_ = -1;									//服务器索引号
	int32_t boxIndex_ = -1;										//盒子索引号
	int32_t cameraIndex_ = -1;									//相机索引号
	int32_t cameraAmount_ = -1;									//相机总个数
	bool operateAllFlag_ = false;								//操作当前服务器下所有设备标志
	int32_t openCameraOperationIndex_ = -1;						//打开相机时欲操作的参数序号
	std::string triggerSingleSaveName_ = "tmp";					//手动保存单张图片的文件名
	std::vector<std::vector<int>> validFlagVec_;				//对当前服务器下的盒子下的相机操作有效性的校验
	int32_t imageType_ = 2;										//获取单张图片的类型
	int64_t imageResizedFactor_=1;								//获取单张图片的缩放因子
	int32_t imageResizedWidth_ = 0;								//获取单张图片缩放后宽度
	int32_t imageResizedHeight_ = 0;							//获取单图片缩放后高度
	float waitTime_ = 0;
	//cv::Mat *imageMat_ =NULL;									//获取单张图片的存放起始地址
	//std::vector<cv::Mat *>imagesMat_;
	//std::vector<cam::Imagedata *>images_;								//GetImage图片地址
	std::vector<char *> images_jpeg_raw;
	std::vector<int *> images_jpeg_ratio;
	std::vector<size_t *> images_jpeg_len;
	//QImage *qimage_ = NULL;									//获取单张图片的存放起始地址
	int32_t imageSize_ = 0;


	std::string genfunc_ = "";
	GenCameraControlData gendata_;

	inline void operator=(const CameraControlMessage &_value)
	{
		requestorId_ = _value.requestorId_;
		command_ = _value.command_;
		status_ = _value.status_;
		serverIndex_ = _value.serverIndex_;
		boxIndex_ = _value.boxIndex_;
		cameraIndex_ = _value.cameraIndex_;
		cameraAmount_ = _value.cameraAmount_;
		openCameraOperationIndex_ = _value.openCameraOperationIndex_;
		operateAllFlag_ = _value.operateAllFlag_;
		triggerSingleSaveName_ = _value.triggerSingleSaveName_;
		imageResizedFactor_ = _value.imageResizedFactor_;
		imageResizedWidth_ = _value.imageResizedWidth_;
		imageResizedHeight_ = _value.imageResizedHeight_;
		waitTime_ = _value.waitTime_;
		/*imageMat_ = _value.imageMat_;*/
		//imagesMat_ = _value.imagesMat_;
		images_jpeg_raw = _value.images_jpeg_raw;
		images_jpeg_len = _value.images_jpeg_len;
		images_jpeg_ratio = _value.images_jpeg_ratio;
		/*qimage_ = _value.qimage_;*/
		imageSize_ = _value.imageSize_;
		genfunc_ = _value.genfunc_;
		gendata_ = _value.gendata_;
	}

};

class CameraOpenBoxPackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Open_Box;		//命令
	int32_t boxAmount_ = 2;																//当前服务器下的盒子总数
	int32_t boxIndex_ = 0;																//预操作的盒子号
	char macAddress_[18] = "00:00:00:00:00:00";											//盒子的mac地址 固定长度为18

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Open_Box" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "macAddress: " << macAddress_ << endl;
	}
};


class CameraCloseBoxPackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Close_Box;		//命令
	int32_t boxAmount_ = 2;																//当前服务器下的盒子总数
	int32_t boxIndex_ = 0;																//预操作的盒子号
	char macAddress_[18] = "00:00:00:00:00:00";											//盒子的mac地址 固定长度为18

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Close_Box" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "macAddress: " << macAddress_ << endl;
	}
};


class CameraOpenCameraPackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Open_Camera;		///命令
	int32_t operationIndex_ = -1;                                   ///欲操作参数序号，-1：整个相机参数，其它：从exposure的第几个参数
	int32_t boxAmount_ = 2;											///当前服务器下的盒子总数
	int32_t boxIndex_ = 0;											///预操作的盒子号
	int32_t cameraAmount_ = 2;										///预操作的盒子下的相机总数
	int32_t cameraIndex_ = 0;										///预操作的盒子下的相机编号
	int32_t cameraId_ = 0;											///相机的整体编号1~35
	int32_t width_ = 2048;											///图像宽度：最大2048
	int32_t height_ = 2048;											///图像高度：最大2048
	int32_t exposure_ = 255;										///0:相机曝光：1~65535
	int32_t gain_ = 3;												///1:相机增益：1~255
	int32_t brightness_ = 32;                                       ///2:相机亮度：1~96 (x0.3125)
	int32_t contrast_ = 32;                                         ///3:相机对比度：1~96 (x0.3125)
	int32_t bitLut_ = 8;                                            ///4:输出灰度查找位：0~8 (bit[7+x:x])
	int32_t saveFormat_ = 1;										///5:保存格式：0:禁用 1:bmp 2:raw 3:bmp+raw
	int32_t skipNumber_ = 0;										///6:跳帧数：0:noskip else: 1/(N+1)
	int32_t triggerMode_ = 1;										///7:触发模式：0：两路独立软触发或硬触发 1:两路同时软触发或硬触发，2：内部自由触发，一直采集图像
	char savePath_[256] = "/home/tmp";								///8:linux下保存路径：/home/tmp/		固定长度为256
	char saveName_[128] = "";										///8:保存的文件名： test				固定长度为128
	//char othersInfo_[8192] = "";                                    ///Tiff保存的操作者、日期、实验目的信息  固定长度为8192
	//char othersInfo_[1024] = "";                                    ///Tiff保存的操作者、日期、实验目的信息  固定长度为1024

	char genfunc_c[256];                                            ///待调用函数
	GenCameraControlData gendata_c;                                 ///数据
	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Open_Camera" << endl;
#ifndef OPEN_CAMERA_LESS_OUTPUT
		std::cout << "         " << "operationIndex: " << operationIndex_ << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
		std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
		std::cout << "         " << "cameraId: " << cameraId_ << endl;
		std::cout << "         " << "width: " << width_ << endl;
		std::cout << "         " << "height: " << height_ << endl;
		std::cout << "         " << "exposure: " << exposure_ << endl;
		std::cout << "         " << "gain: " << gain_ << endl;
		std::cout << "         " << "brightness:  " << brightness_ << endl;
		std::cout << "         " << "contrast: " << contrast_ << endl;
		std::cout << "         " << "bitLut:  " << bitLut_ << endl;
		std::cout << "         " << "saveFormat: " << saveFormat_ << endl;
		std::cout << "         " << "skipNumber: " << skipNumber_ << endl;
		std::cout << "         " << "triggerMode: " << triggerMode_ << endl;
		std::cout << "         " << "savePath: " << savePath_ << endl;
		std::cout << "         " << "saveName: " << saveName_ << endl;
#endif
		std::cout << "         " << "function: " << genfunc_c << endl;
	}
};



class CameraCloseCameraPackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Close_Camera;	//命令
	int32_t boxAmount_ = 2;																//当前服务器下的盒子总数						
	int32_t boxIndex_ = 0;																//预操作的盒子号
	int32_t cameraAmount_ = 2;															//预操作的盒子下的相机总数
	int32_t cameraIndex_ = 0;															//预操作的盒子下的相机编号

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Close_Camera" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
		std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
	}
};


class CameraTriggerContinousPackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Trigger_Continous;	//命令
	int32_t boxAmount_ = 2;																	//当前服务器下的盒子总数						
	int32_t boxIndex_ = 0;																	//预操作的盒子号
	int32_t cameraAmount_ = 2;																//预操作的盒子下的相机总数
	int32_t cameraIndex_ = 0;																//预操作的盒子下的相机编号	
	int32_t triggerNumber_ = -1;															//采集数量：在triggerMode_为2时无效，将会一直采图，在触发模式为0或1时，设为-1将会一直采图，大于0采集指定的数量
	char saveName_[128] = "";																//保存的文件名：固定长度为128	

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Trigger_Soft" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
		std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
		std::cout << "         " << "triggerNumber: " << triggerNumber_ << endl;
		std::cout << "         " << "saveName: " << saveName_ << endl;
	}
};


class CameraTriggerSinglePackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Trigger_Single;	//命令
	int32_t boxAmount_ = 2;																//当前服务器下的盒子总数						
	int32_t boxIndex_ = 0;																//预操作的盒子号
	int32_t cameraAmount_ = 2;															//预操作的盒子下的相机总数
	int32_t cameraIndex_ = 0;															//预操作的盒子下的相机编号	
	int32_t triggerNumber_ = 1;															//采集数量：在triggerMode_为2时无效，将会一直采图，在触发模式为0或1时，设为-1将会一直采图，大于0采集指定的数量
	char saveName_[128] = "";															//保存的文件名：固定长度为128	

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Trigger_Soft" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
		std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
		std::cout << "         " << "triggerNumber: " << triggerNumber_ << endl;
		std::cout << "         " << "saveName: " << saveName_ << endl;
	}
};


class CameraResetIdPackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Reset_Id;		//命令
	int32_t boxAmount_ = 2;																//当前服务器下的盒子总数						
	int32_t boxIndex_ = 0;																//预操作的盒子号
	int32_t cameraAmount_ = 2;															//预操作的盒子下的相机总数
	int32_t cameraIndex_ = 0;															//预操作的盒子下的相机编号	

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Reset_Id" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
		std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
	}
};

 
class CameraGetImagePackage
{
public:
	const Communication_Camera_Command command_ = Communication_Camera_Get_Image;		//命令
	int32_t boxAmount_ = 2;																///当前服务器下的盒子总数
	int32_t boxIndex_ = 0;																///预操作的盒子号
	int32_t cameraAmount_ = 2;															///预操作的盒子下的相机总数
	int32_t cameraIndex_ = 0;															///预操作的盒子下的相机编号
	int32_t imageType_ = 2;																///图像类型，1:Mono_8Bit,2:Mono_16Bit,3:Color_8Bit,
	int32_t resizeFactor_ = 0;															///每一位为一个插值等级
	int32_t resizedWidth_ = 2048;														///插值图像宽度，该值由请求者指定
	int32_t resizedHeight_ = 2048;														///插值图像高度，该值由请求者指定

	void PrintInfo(std::string _str)
	{
		std::cout << _str << endl;
		std::cout << "Command: " << "Communication_Camera_Get_Image" << endl;
		std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
		std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
		std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
		std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
		std::cout << "         " << "imageType: " << imageType_ << endl;
		std::cout << "         " << "resizeFactor: " << resizeFactor_ << endl;
		std::cout << "         " << "resizedWidth: " << resizedWidth_ << endl;
		std::cout << "         " << "resizedHeight: " << resizedHeight_ << endl;
	}

	inline bool operator==(CameraGetImagePackage &_value)
	{
		if (boxAmount_ != _value.boxAmount_ || boxIndex_ != _value.boxIndex_||cameraAmount_!=_value.cameraAmount_||cameraIndex_!=_value.cameraIndex_) {
			return false;
		}
		if (imageType_ != _value.imageType_ || resizeFactor_ != _value.resizeFactor_ || resizedWidth_ != _value.resizedWidth_ || resizedHeight_ != _value.resizedHeight_) {
			return false;
		}
		return true;
	}
};


///定长,分配在heap上
typedef struct
{
	Communication_Camera_Command command_;
	//只有在命令不为Communication_Camera_Command_Get_Status时才调用
	//第一个字节为command,后面的对应不同的解析方式
	char data[CAMERA_COMMAND_DATA_MAX_SIZE];

}CameraClientSendPackageTypdef;


///释放时要先释放data指向的内存区域
typedef struct
{
	Communication_Camera_Status status_;
	int dataSize_;          //说明后面接的缓冲区的内存大小
	char *data_=NULL;       //图像数据缓冲区，需要动态分配
}CameraClientReceivePackageTypeDef;



typedef struct
{
	Communication_Camera_Command command_;
	//只有在命令不为Communication_Camera_Command_Get_Status时才调用
	//第一个字节为command,后面的对应不同的解析方式
	char data[CAMERA_COMMAND_DATA_MAX_SIZE];

}CameraServerReceivePackageTypdef;


///释放时要先释放data指向的内存区域
typedef struct
{
	Communication_Camera_Status status_;
	int dataSize_;          //说明后面接的缓冲区的内存大小
	char *data_=NULL;       //图像数据缓冲区，需要动态分配
}CameraServerSendPackageTypeDef;
