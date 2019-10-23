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

//#define CAMERA_COMMAND_DATA_MAX_SIZE  (8192+2048)					///xxxxxxxxxxxxxxxxxxxxxx
#define CAMERA_COMMAND_DATA_MAX_SIZE  4096*8			///xxxxxxxxxxxxxxxxxxxxxx
#define CAMERA_IMAGE_DATA_MAX_SIZE    (4096*4096*4+2048)		///xxxxxxxxxxxxxxxx

#define MAX_CAMERA_NUM 16
#define MAX_PARAM_NUM 16
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
	GenCameraControlData & operator=(const GenCameraControlData& data)
	{
		//memcpy(this, &data, sizeof(GenCameraControlData));
		this->void_func.return_val = data.void_func.return_val;
		this->caminfo_func.return_val = data.caminfo_func.return_val;
		this->param_func.return_val = data.param_func.return_val;
		this->str_func.return_val = data.str_func.return_val;
		
		memcpy(this->caminfo_func.camInfos, data.caminfo_func.camInfos, sizeof(this->caminfo_func.camInfos));
		memcpy(this->param_func.param_bool, data.param_func.param_bool, sizeof(this->param_func.param_bool));
		memcpy(this->param_func.param_enum, data.param_func.param_enum, sizeof(this->param_func.param_enum));
		memcpy(this->param_func.param_int, data.param_func.param_int, sizeof(this->param_func.param_int));
		memcpy(this->param_func.param_float, data.param_func.param_float, sizeof(this->param_func.param_float));
		memcpy(this->str_func.str, data.str_func.str, sizeof(this->str_func.str));
		return *this;
	}
};




typedef struct
{
	int32_t id_ = 0;										//xxxxxxxxxxxxxxxxxxxx1~35
	int32_t width_ = 2048;									//xxxxxxxxxxxxxx2048
	int32_t height_ = 2048;									//xxxxxxxxxxxxxx2048
	int32_t exposure_ = 100;								//xxxxxxxxxx1~65535
	int32_t gain_ = 3;										//xxxxxxxxxx1~255
	int32_t brightness_ = 32;                               //xxxxxxxxxx1~96 (x0.3125)
	int32_t contrast_ = 32;                                 //xxxxxxxxxxxx1~96 (x0.3125)
	int32_t bitLut_ = 8;                                    //xxxxxxxxxxxxxxxx0~8 (bit[7+x:x])
	int32_t saveFormat_ = 1;								//xxxxxxxxxx0:xxxx 1:bmp 2:raw 3:bmp+raw
	int32_t skipNumber_ = 0;								//xxxxxxxx  0:noskip else: 1/(N+1)
	int32_t triggerMode_ = 1;								//xxxxxxxxxx0xxxxxxxxxxxxxxxxxxxxxxxx 1:xxxxxxxxxxxxxxxxxxxxxxxx2xxxxxxxxxxxxxx
	std::string savePath_ = "/home/tmp";					//linuxxxxxxxxxxxxx/home/tmp/	xxxxxxxxxx256
	std::string saveName_ = "";							    //xxxxxxxxxxxxxxxx test			xxxxxxxxxx128
	//std::string othersInfo_ = "Operator:zhangyang\nDate:2017-07-14 10:33\nExperimental purpose:observation rat brain.";                           //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx xxxxxxxxxx1024
}CameraParametersUnitTypeDef;


///xxxxxxxxxxxxxxxxxxxxxxxxGUIxxxxxxxxxxxxxxxxxx
typedef struct
{
	int32_t id_ = 0;										//xxxxxxxxxxxxxxxxxxxx1~18			
	std::string mac_ = "00:11:1c:02:30:98";					//xxxxxxmacxxxx  xxxxxxxxxx17+1
	int32_t cameraAmount_ = 2;								//xxxxxxxxxxxxxxxx
	std::vector<CameraParametersUnitTypeDef> cameraVec_ = std::vector<CameraParametersUnitTypeDef>(2, CameraParametersUnitTypeDef());//xxxxxxxxxxxxxxxx

}CameraBoxUnitTypeDef;


typedef struct
{
	int32_t id_ = 0;										//xxxxxxxxxxxxxxxxxxxxxx1~9
	bool connectedFlag_ = false;							//xxxxxxxxxxxx
	std::string ip_ = "10.8.5.199";					//xxxxxxIPxx 192.168.111.000 xxxxxxxxxx15+1
	std::string port_ = "54321";							//xxxxxxxxxxxx 123456
	int32_t boxAmount_ = 1;									//xxxxxxxxxxxxxxxxxxxx
	std::vector<CameraBoxUnitTypeDef> boxVec_ = std::vector<CameraBoxUnitTypeDef>(boxAmount_, CameraBoxUnitTypeDef());		//xxxxxxxxxxxxxxxx

}CameraServerUnitTypeDef;


///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
enum Communication_Camera_Command
{
	Communication_Camera_Reset_Connection = 0,				//xxxxsocketxxxx,xxxxxxxxxxxxxxxxGet_Status
	Communication_Camera_Get_Status = 1,					//xxxxxxxxxxxxxxxxxxxxxx,xxxxxxxxxx
	Communication_Camera_Open_Box = 2,						//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	Communication_Camera_Close_Box = 3,						//xxxxxxxx,xxxxxxxxxxxxxxxxxxxxxx
	Communication_Camera_Open_Camera = 4,					//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	Communication_Camera_Close_Camera = 5,					//xxxxxxxxxxxxxxxxxxxx
	Communication_Camera_Trigger_Continous = 6,				//xxxxxxxx,xxxxxxxxxxxxxxxxxxxxxxxxxx
	Communication_Camera_Trigger_Single = 7,				//xxxxxxxx,xxxxxxxxxxxxxxxxxx
	Communication_Camera_Reset_Id = 8,						//xxxxxxxxId
	Communication_Camera_Get_Image = 9						//xxxxxxxxxx
};

enum  Communication_Camera_Status
{
	Communication_Camera_Get_Status_Ok = 0,						//xxxxxxxxxxxxxxxx
	Communication_Camera_Get_Status_Invalid = 1,				//xxxxxxxxxxxxxxxx
	Communication_Camera_Open_Box_Ok = 2,						//xxxxxxxxxxxxxxxx
	Communication_Camera_Open_Box_Invalid = 3,					//xxxxxxxxxxxxxxxx
	Communication_Camera_Close_Box_Ok = 4,						//xxxxxxxxxxxxxxxx
	Communication_Camera_Close_Box_Invalid=5,					//xxxxxxxxxxxxxxxx
	Communication_Camera_Open_Camera_Ok = 6,					//xxxxxxxxxxxxxxxx
	Communication_Camera_Open_Camera_Invalid=7,					//xxxxxxxxxxxxxxxx
	Communication_Camera_Close_Camera_Ok = 8,					//xxxxxxxxxxxxxxxx
	Communication_Camera_Close_Camera_Invalid = 9,				//xxxxxxxxxxxxxxxx
	Communication_Camera_Trigger_Continous_Ok = 10,				//xxxxxxxxxxxxxxxx
	Communication_Camera_Trigger_Continous_Invalid = 11,		//xxxxxxxxxxxxxxxx
	Communication_Camera_Trigger_Single_Ok = 12,				//xxxxxxxxxxxxxxxx
	Communication_Camera_Trigger_Single_Invalid = 13,			//xxxxxxxxxxxxxxxx
	Communication_Camera_Reset_Id_Ok = 14,						//xxxxIdxxxxxxxx
	Communication_Camera_Reset_Id_Invalid = 15,					//xxxxIdxxxxxxxx
	Communication_Camera_Get_Image_Ok = 16,						//xxxxxxxxxxxxxxxx
	Communication_Camera_Get_Image_Invalid = 17,				//xxxxxxxxxxxxxxxx
	Communication_Camera_Action_Overtime = 18,					//xxxxxxxxxxxxxxxxxxxx
	Communication_Camera_Action_Invalid = 19					//xxxxxxxxxxxxxxxxxxxx
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

///xxxxxxxxxxxxGUIxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
class CameraControlMessage
{
public:
	int32_t requestorId_ = -1;
	Communication_Camera_Command command_ = Communication_Camera_Open_Box;
	Communication_Camera_Status status_ = Communication_Camera_Open_Box_Invalid;
	int32_t serverIndex_ = -1;									//xxxxxxxxxxxx
	int32_t boxIndex_ = -1;										//xxxxxxxxxx
	int32_t cameraIndex_ = -1;									//xxxxxxxxxx
	int32_t cameraAmount_ = -1;									//xxxxxxxxxx
	bool operateAllFlag_ = false;								//xxxxxxxxxxxxxxxxxxxxxxxxxxxx
	int32_t openCameraOperationIndex_ = -1;						//xxxxxxxxxxxxxxxxxxxxxxxxxx
	std::string triggerSingleSaveName_ = "tmp";					//xxxxxxxxxxxxxxxxxxxxxxxx
	std::vector<std::vector<int>> validFlagVec_;				//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	int32_t imageType_ = 2;										//xxxxxxxxxxxxxxxxxx
	int64_t imageResizedFactor_=1;								//xxxxxxxxxxxxxxxxxxxxxx
	int32_t imageResizedWidth_ = 0;								//xxxxxxxxxxxxxxxxxxxxxx
	int32_t imageResizedHeight_ = 0;							//xxxxxxxxxxxxxxxxxxxx
	float waitTime_ = 0;
	//cv::Mat *imageMat_ =NULL;									//xxxxxxxxxxxxxxxxxxxxxxxxxx
	//std::vector<cv::Mat *>imagesMat_;
	//std::vector<cam::Imagedata *>images_;								//GetImagexxxxxxxx
	std::vector<char *> images_jpeg_raw;
	std::vector<int *> images_jpeg_ratio;
	std::vector<size_t *> images_jpeg_len;
	//QImage *qimage_ = NULL;									//xxxxxxxxxxxxxxxxxxxxxxxxxx
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
	const Communication_Camera_Command command_ = Communication_Camera_Open_Box;		//xxxx
	int32_t boxAmount_ = 2;																//xxxxxxxxxxxxxxxxxxxxxx
	int32_t boxIndex_ = 0;																//xxxxxxxxxxxxxx
	char macAddress_[18] = "00:00:00:00:00:00";											//xxxxxxmacxxxx xxxxxxxxxx18

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
	const Communication_Camera_Command command_ = Communication_Camera_Close_Box;		//xxxx
	int32_t boxAmount_ = 2;																//xxxxxxxxxxxxxxxxxxxxxx
	int32_t boxIndex_ = 0;																//xxxxxxxxxxxxxx
	char macAddress_[18] = "00:00:00:00:00:00";											//xxxxxxmacxxxx xxxxxxxxxx18

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
	const Communication_Camera_Command command_ = Communication_Camera_Open_Camera;		///xxxx
	int32_t operationIndex_ = -1;                                   ///xxxxxxxxxxxxxxxx-1xxxxxxxxxxxxxxxxxxxxxxxxexposurexxxxxxxxxxxx
	int32_t boxAmount_ = 2;											///xxxxxxxxxxxxxxxxxxxxxx
	int32_t boxIndex_ = 0;											///xxxxxxxxxxxxxx
	int32_t cameraAmount_ = 2;										///xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraIndex_ = 0;										///xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraId_ = 0;											///xxxxxxxxxxxxxx1~35
	int32_t width_ = 2048;											///xxxxxxxxxxxxxx2048
	int32_t height_ = 2048;											///xxxxxxxxxxxxxx2048
	int32_t exposure_ = 255;										///0:xxxxxxxxxx1~65535
	int32_t gain_ = 3;												///1:xxxxxxxxxx1~255
	int32_t brightness_ = 32;                                       ///2:xxxxxxxxxx1~96 (x0.3125)
	int32_t contrast_ = 32;                                         ///3:xxxxxxxxxxxx1~96 (x0.3125)
	int32_t bitLut_ = 8;                                            ///4:xxxxxxxxxxxxxxxx0~8 (bit[7+x:x])
	int32_t saveFormat_ = 1;										///5:xxxxxxxxxx0:xxxx 1:bmp 2:raw 3:bmp+raw
	int32_t skipNumber_ = 0;										///6:xxxxxxxx0:noskip else: 1/(N+1)
	int32_t triggerMode_ = 1;										///7:xxxxxxxxxx0xxxxxxxxxxxxxxxxxxxxxxxx 1:xxxxxxxxxxxxxxxxxxxxxxxx2xxxxxxxxxxxxxxxxxxxxxxxxxxxx
	char savePath_[256] = "/home/tmp";								///8:linuxxxxxxxxxxxxx/home/tmp/		xxxxxxxxxx256
	char saveName_[128] = "";										///8:xxxxxxxxxxxxxx test				xxxxxxxxxx128
	//char othersInfo_[8192] = "";                                    ///Tiffxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  xxxxxxxxxx8192
	//char othersInfo_[1024] = "";                                    ///Tiffxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  xxxxxxxxxx1024

	char genfunc_c[256];                                            ///xxxxxxxxxx
	GenCameraControlData gendata_c;                                 ///xxxx
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
	const Communication_Camera_Command command_ = Communication_Camera_Close_Camera;	//xxxx
	int32_t boxAmount_ = 2;																//xxxxxxxxxxxxxxxxxxxxxx						
	int32_t boxIndex_ = 0;																//xxxxxxxxxxxxxx
	int32_t cameraAmount_ = 2;															//xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraIndex_ = 0;															//xxxxxxxxxxxxxxxxxxxxxxxx

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
	const Communication_Camera_Command command_ = Communication_Camera_Trigger_Continous;	//xxxx
	int32_t boxAmount_ = 2;																	//xxxxxxxxxxxxxxxxxxxxxx						
	int32_t boxIndex_ = 0;																	//xxxxxxxxxxxxxx
	int32_t cameraAmount_ = 2;																//xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraIndex_ = 0;																//xxxxxxxxxxxxxxxxxxxxxxxx	
	int32_t triggerNumber_ = -1;															//xxxxxxxxxxxxtriggerMode_xx2xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx0xx1xxxxxxxx-1xxxxxxxxxxxxxxxxxx0xxxxxxxxxxxxxx
	char saveName_[128] = "";																//xxxxxxxxxxxxxxxxxxxxxxxx128	

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
	const Communication_Camera_Command command_ = Communication_Camera_Trigger_Single;	//xxxx
	int32_t boxAmount_ = 2;																//xxxxxxxxxxxxxxxxxxxxxx						
	int32_t boxIndex_ = 0;																//xxxxxxxxxxxxxx
	int32_t cameraAmount_ = 2;															//xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraIndex_ = 0;															//xxxxxxxxxxxxxxxxxxxxxxxx	
	int32_t triggerNumber_ = 1;															//xxxxxxxxxxxxtriggerMode_xx2xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx0xx1xxxxxxxx-1xxxxxxxxxxxxxxxxxx0xxxxxxxxxxxxxx
	char saveName_[128] = "";															//xxxxxxxxxxxxxxxxxxxxxxxx128	

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
	const Communication_Camera_Command command_ = Communication_Camera_Reset_Id;		//xxxx
	int32_t boxAmount_ = 2;																//xxxxxxxxxxxxxxxxxxxxxx						
	int32_t boxIndex_ = 0;																//xxxxxxxxxxxxxx
	int32_t cameraAmount_ = 2;															//xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraIndex_ = 0;															//xxxxxxxxxxxxxxxxxxxxxxxx	

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
	const Communication_Camera_Command command_ = Communication_Camera_Get_Image;		//xxxx
	int32_t boxAmount_ = 2;																///xxxxxxxxxxxxxxxxxxxxxx
	int32_t boxIndex_ = 0;																///xxxxxxxxxxxxxx
	int32_t cameraAmount_ = 2;															///xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t cameraIndex_ = 0;															///xxxxxxxxxxxxxxxxxxxxxxxx
	int32_t imageType_ = 2;																///xxxxxxxxxx1:Mono_8Bit,2:Mono_16Bit,3:Color_8Bit,
	int64_t resizeFactor_ = 0;															///xxxxxxxxxxxxxxxxxxxx
	int32_t resizedWidth_ = 2048;														///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	int32_t resizedHeight_ = 2048;														///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

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


///xxxx,xxxxxxheapxx
typedef struct
{
	Communication_Camera_Command command_;
	//xxxxxxxxxxxxxxCommunication_Camera_Command_Get_Statusxxxxxxxx
	//xxxxxxxxxxxxcommand,xxxxxxxxxxxxxxxxxxxxxxxx
	char data[CAMERA_COMMAND_DATA_MAX_SIZE];

}CameraClientSendPackageTypdef;


///xxxxxxxxxxxxxxdataxxxxxxxxxxxxxx
typedef struct
{
	Communication_Camera_Status status_;
	int dataSize_;          //xxxxxxxxxxxxxxxxxxxxxxxxxxxx
	char *data_=NULL;       //xxxxxxxxxxxxxxxxxxxxxxxxxxxx
}CameraClientReceivePackageTypeDef;



typedef struct
{
	Communication_Camera_Command command_;
	//xxxxxxxxxxxxxxCommunication_Camera_Command_Get_Statusxxxxxxxx
	//xxxxxxxxxxxxcommand,xxxxxxxxxxxxxxxxxxxxxxxx
	char data[CAMERA_COMMAND_DATA_MAX_SIZE];

}CameraServerReceivePackageTypdef;


///xxxxxxxxxxxxxxdataxxxxxxxxxxxxxx
typedef struct
{
	Communication_Camera_Status status_;
	int dataSize_;          //xxxxxxxxxxxxxxxxxxxxxxxxxxxx
	char *data_=NULL;       //xxxxxxxxxxxxxxxxxxxxxxxxxxxx
}CameraServerSendPackageTypeDef;
