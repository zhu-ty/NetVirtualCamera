/********************************************************************
//Describe: camera.cpp: used for controlling camera
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/10/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
*********************************************************************/
#include "NetCameraCore.h"

//#include <opencv2/cudawarping.hpp>
//#include <opencv2/cudaimgproc.hpp>

CameraCommunicationThread::CameraCommunicationThread(int _id,std::vector<CameraServerUnitTypeDef> &_serverVec)
{
	formVector_.resize(4096);

	for (int i = 0;i < formVector_.size();i++)
	{
		formVector_[i] = i / 16;
	} //默认取高八位
	//decoder.init(_serverVec[0].boxVec_[0].cameraVec_[0].height_, _serverVec[0].boxVec_[0].cameraVec_[0].width_, 75);
	//img=new cv::cuda::GpuMat(_serverVec[0].boxVec_[0].cameraVec_[0].height_, _serverVec[0].boxVec_[0].cameraVec_[0].width_, CV_8UC3);
	//resizeimg = new cv::cuda::GpuMat(512, 512, CV_8UC3);
	id_ = _id;
	serverVec_ = _serverVec;
	updateTimer = new QTimer(this);
	QObject::connect(updateTimer, SIGNAL(timeout()), this, SLOT(UpdateStatus()));
	updateTimer->setInterval(updateTimerInterval);
	updateTimer->start();

	//tcpSocket_ = new QTcpSocket();
	//tcpSocket_->setParent(this);
	//QObject::connect(tcpSocket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(SocketStateChanged()));
#ifdef CSharp
	tcpSocket_ = new SKSocket();
#else
	tcpSocket_ = new AsioSocket();
#endif
	receivePackage_.data_ = new char[CAMERA_IMAGE_DATA_MAX_SIZE];
}


CameraCommunicationThread::~CameraCommunicationThread()
{
	delete receivePackage_.data_;
}


void CameraCommunicationThread::UpdateStatus(void)
{
	///如果当前未连接则复位连接
	if (id_ < serverVec_.size()) {
		if (!serverVec_[id_].connectedFlag_) {
			ResetSocket();
		}
		//已连接则发送心跳包来帮助服务器确认状态
		else {
			sendPackage_.command_ = Communication_Camera_Get_Status;
			Communication_Camera_Status tmp=SendData();
			if (tmp == Communication_Camera_Get_Status_Ok) {
				serverVec_[id_].connectedFlag_ = true;
			}
			else {
				serverVec_[id_].connectedFlag_ = false;
			}
		}
		cameraControlMessage_.requestorId_ = 0;
		cameraControlMessage_.command_ = Communication_Camera_Get_Status;
		cameraControlMessage_.status_ = serverVec_[id_].connectedFlag_ ? Communication_Camera_Get_Status_Ok : Communication_Camera_Get_Status_Invalid;
		cameraControlMessage_.serverIndex_=id_;
		cameraControlMessage_.boxIndex_ = 0;
		cameraControlMessage_.cameraIndex_ = 0;
		emit OperationFinished(cameraControlMessage_);
	}
}


void CameraCommunicationThread::SocketStateChanged(void)
{
	//if (tcpSocket_!= NULL) {
	//	if (tcpSocket_->state() == QAbstractSocket::ConnectedState) {
	//		tcpSocket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	//		tcpSocket_->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
	//		if (id_ < serverVec_.size()) {
	//			serverVec_[id_].connectedFlag_ = true;
	//		}
	//	}
	//	else if (tcpSocket_->state() == QAbstractSocket::UnconnectedState) {
	//		if (id_ < serverVec_.size()) {
	//			serverVec_[id_].connectedFlag_ = false;
	//		}
	//	}
	//}
}


void CameraCommunicationThread::ResetSocket(void)
{
	if (id_ < serverVec_.size()) {
		try
		{
			tcpSocket_->abort();
		}
		catch(...)
		{

		}
		//tcpSocket_->abort();
		char tmp[100];
		memcpy(tmp, serverVec_[id_].ip_.c_str(), serverVec_[id_].ip_.size());
		serverVec_[id_].connectedFlag_ = tcpSocket_->connectToHost(
			(unsigned char*)tmp,
			serverVec_[id_].ip_.size(),
			std::stoi(serverVec_[id_].port_),
			1000);;
		//tcpSocket_->connectToHost(QString::fromStdString(serverVec_[id_].ip_), std::stoi(serverVec_[id_].port_), QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
	}
}


Communication_Camera_Status CameraCommunicationThread::SendData(void)
{
	if (id_ < serverVec_.size()) {
		//if (tcpSocket_ != NULL&&tcpSocket_->state() == QAbstractSocket::ConnectedState) {
		if (tcpSocket_ != nullptr && tcpSocket_->state() == true)
		{
			//tcpSocket_->write((char *)&sendPackage_.command_, sizeof(sendPackage_));
			if (tcpSocket_->write((unsigned char *)&sendPackage_.command_, sizeof(sendPackage_))) {
				if (tcpSocket_->waitForReadyRead(socketReadWaitForMs_))
					//if (tcpSocket_->waitForReadyRead(socketReadWaitForMs_)) {
						///先解析返回的状态
					if (sendPackage_.command_ == Communication_Camera_Get_Status) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						//tcpSocket_->read((char *)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Open_Box) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						//tcpSocket_->read((char *)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Open_Camera) {
						//int readbyteSize = tcpSocket_->read((char *)&receivePackage_.status_, sizeof(receivePackage_.status_) + sizeof(receivePackage_.dataSize_));
						int readbyteSize = tcpSocket_->read(
							(unsigned char*)&receivePackage_.status_,
							sizeof(receivePackage_.status_) + sizeof(receivePackage_.dataSize_));
						//tcpSocket_->read((char *)&receivePackage_.status_, sizeof(receivePackage_.status_));
						//	读取返回值
						if (receivePackage_.status_ == Communication_Camera_Open_Camera_Ok) {
							int dataAmount = receivePackage_.dataSize_;
							if (dataAmount > 0 && dataAmount < CAMERA_IMAGE_DATA_MAX_SIZE) {
								int readedSize = 0;
								while (readedSize < dataAmount) {
									if (tcpSocket_->waitForReadyRead(socketReadWaitForMs_)) {
										//readedSize += tcpSocket_->read(receivePackage_.data_ + readedSize, dataAmount - readedSize);
										readedSize += tcpSocket_->read(
											(unsigned char*)(receivePackage_.data_ + readedSize), dataAmount - readedSize);
									}
									else {
										receivePackage_.status_ = Communication_Camera_Action_Overtime;
										return receivePackage_.status_;
									}
								}
							}
						}
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Trigger_Continous) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Trigger_Single) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Get_Image) {
						//int readbyteSize = tcpSocket_->read((char *)&receivePackage_.status_, sizeof(receivePackage_.status_) + sizeof(receivePackage_.dataSize_));
						int readbyteSize = tcpSocket_->read(
							(unsigned char*)&receivePackage_.status_,
							sizeof(receivePackage_.status_) + sizeof(receivePackage_.dataSize_));
						//	读取图片
						if (receivePackage_.status_ == Communication_Camera_Get_Image_Ok) {
							int dataAmount = receivePackage_.dataSize_;
							if (dataAmount > 0 && dataAmount < CAMERA_IMAGE_DATA_MAX_SIZE) {
								int readedSize = 0;
								while (readedSize < dataAmount) {
									if (tcpSocket_->waitForReadyRead(socketReadWaitForMs_)) {
										//readedSize += tcpSocket_->read(receivePackage_.data_ + readedSize, dataAmount - readedSize);
										readedSize += tcpSocket_->read(
											(unsigned char*)(receivePackage_.data_ + readedSize), dataAmount - readedSize);
									}
									else {
										receivePackage_.status_ = Communication_Camera_Action_Overtime;
										return receivePackage_.status_;
									}
								}
							}
						}
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Close_Box) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Close_Camera) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
					else if (sendPackage_.command_ == Communication_Camera_Reset_Id) {
						tcpSocket_->read((unsigned char*)&receivePackage_.status_, sizeof(receivePackage_.status_));
						return receivePackage_.status_;
					}
			}
			return Communication_Camera_Action_Overtime;
		}
	}
	return Communication_Camera_Action_Invalid;
}


void CameraCommunicationThread::StartOperation(CameraControlMessage &_cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec, QVector<int> &_formVector)
{
	serverVec_ = _serverVec;
	if (_cameraControlMessage.serverIndex_<serverVec_.size()&&_cameraControlMessage.serverIndex_ == id_) {
		cameraControlMessage_ = _cameraControlMessage;
		Communication_Camera_Command command = _cameraControlMessage.command_;
		int serverIndex = _cameraControlMessage.serverIndex_;
		bool operateAllFlag = _cameraControlMessage.operateAllFlag_;
		switch (command) {
		case Communication_Camera_Get_Status: {					//get status
			if (serverVec_[serverIndex].connectedFlag_ == true) {
				cameraControlMessage_.status_ = Communication_Camera_Get_Status_Ok;
			}
			else {
				cameraControlMessage_.status_ = Communication_Camera_Get_Status_Invalid;
			}
			emit OperationFinished(cameraControlMessage_);
			break;
		}
		case Communication_Camera_Reset_Connection: {			//reset
			ResetSocket();
			break;
		}
		case Communication_Camera_Open_Box: {					//open box
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				CameraOpenBoxPackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
				dataTmp.boxIndex_ = boxIndex;
				memcpy(dataTmp.macAddress_, serverVec_[serverIndex].boxVec_[boxIndex].mac_.c_str(), sizeof(dataTmp.macAddress_));

				sendPackage_.command_ = Communication_Camera_Open_Box;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				cameraControlMessage_.boxIndex_ = boxIndex;
				emit OperationFinished(cameraControlMessage_);
			}
			else {			//操作当前服务器下的所有盒子
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					int boxIndex = i;
					CameraOpenBoxPackage dataTmp;
					dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
					dataTmp.boxIndex_ = boxIndex; 
					memcpy(dataTmp.macAddress_, serverVec_[serverIndex].boxVec_[boxIndex].mac_.c_str(), sizeof(dataTmp.macAddress_));

					sendPackage_.command_ = Communication_Camera_Open_Box;
					memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
					cameraControlMessage_.status_ = SendData();
					cameraControlMessage_.boxIndex_ = boxIndex;					
					emit OperationFinished(cameraControlMessage_);
				}
			}
			break;
		}
		case Communication_Camera_Open_Camera: {				//open camera
			//if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				CameraOpenCameraPackage dataTmp;
				dataTmp.operationIndex_ = _cameraControlMessage.openCameraOperationIndex_;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
				dataTmp.boxIndex_ = boxIndex;
				dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
				dataTmp.cameraIndex_ = cameraIndex;
				dataTmp.cameraId_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].id_;
				dataTmp.width_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].width_;
				dataTmp.height_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].height_;
				dataTmp.exposure_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].exposure_;
				dataTmp.gain_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].gain_;
				dataTmp.brightness_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].brightness_;
				dataTmp.contrast_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].contrast_;
				dataTmp.bitLut_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].bitLut_;
				dataTmp.saveFormat_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].saveFormat_;
				dataTmp.skipNumber_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].skipNumber_;
				dataTmp.triggerMode_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].triggerMode_;
				memcpy(dataTmp.savePath_, serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].savePath_.c_str(), sizeof(dataTmp.savePath_));
				memcpy(dataTmp.saveName_, serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].saveName_.c_str(), sizeof(dataTmp.saveName_));

				memcpy(dataTmp.genfunc_c, _cameraControlMessage.genfunc_.c_str(), _cameraControlMessage.genfunc_.size());
				dataTmp.genfunc_c[_cameraControlMessage.genfunc_.size()] = '\0';
				memcpy(&dataTmp.gendata_c, &_cameraControlMessage.gendata_, sizeof(GenCameraControlData));
				//memcpy(dataTmp.othersInfo_, serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].othersInfo_.c_str(), sizeof(dataTmp.othersInfo_));
				sendPackage_.command_ = Communication_Camera_Open_Camera;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				if (receivePackage_.status_ == Communication_Camera_Open_Camera_Ok) {
					CameraOpenCameraPackage receiveDataTmp;
					memcpy(&receiveDataTmp, receivePackage_.data_, sizeof(CameraOpenCameraPackage));
					cameraControlMessage_.status_ = receivePackage_.status_;
					cameraControlMessage_.boxIndex_ = receiveDataTmp.boxIndex_;
					cameraControlMessage_.cameraIndex_ = receiveDataTmp.cameraIndex_;
					cameraControlMessage_.imageType_ = receivePackage_.dataSize_;  
					cameraControlMessage_.cameraAmount_ = receiveDataTmp.cameraAmount_;

					std::string tmp_str(receiveDataTmp.genfunc_c);
					cameraControlMessage_.genfunc_ = tmp_str;
					cameraControlMessage_.gendata_ = receiveDataTmp.gendata_c;

				}
				cameraControlMessage_.boxIndex_ = boxIndex;
				cameraControlMessage_.cameraIndex_ = cameraIndex;
				emit OperationFinished(cameraControlMessage_);
			//}
			//else {			//操作当前服务器下的所有相机
			//	for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size();++i) {
			//		for (int32_t j = 0; j < serverVec_[serverIndex].boxVec_[i].cameraVec_.size(); ++j) {
			//			int boxIndex = i;
			//			int cameraIndex = j;
			//			CameraOpenCameraPackage dataTmp;
			//			dataTmp.operationIndex_ = _cameraControlMessage.openCameraOperationIndex_;
			//			dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
			//			dataTmp.boxIndex_ = boxIndex;
			//			dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
			//			dataTmp.cameraIndex_ = cameraIndex;
			//			dataTmp.cameraId_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].id_;
			//			dataTmp.width_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].width_;
			//			dataTmp.height_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].height_;
			//			dataTmp.exposure_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].exposure_;
			//			dataTmp.gain_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].gain_;
			//			dataTmp.brightness_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].brightness_;
			//			dataTmp.contrast_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].contrast_;
			//			dataTmp.bitLut_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].bitLut_;
			//			dataTmp.saveFormat_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].saveFormat_;
			//			dataTmp.skipNumber_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].skipNumber_;
			//			dataTmp.triggerMode_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].triggerMode_;
			//			memcpy(dataTmp.savePath_, serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].savePath_.c_str(), sizeof(dataTmp.savePath_));
			//			memcpy(dataTmp.saveName_, serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].saveName_.c_str(), sizeof(dataTmp.saveName_));
			//			//memcpy(dataTmp.othersInfo_, serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].othersInfo_.c_str(), sizeof(dataTmp.othersInfo_));
			//			sendPackage_.command_ = Communication_Camera_Open_Camera;
			//			memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
			//			cameraControlMessage_.status_ = SendData();
			//			cameraControlMessage_.boxIndex_ = boxIndex;
			//			cameraControlMessage_.cameraIndex_ = cameraIndex;
			//			emit OperationFinished(cameraControlMessage_);
			//		}
			//	}
			//}
			break;
		}
		case Communication_Camera_Trigger_Continous: {			//trigger continous
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				std::string triggerSaveName = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].saveName_;
				CameraTriggerContinousPackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
				dataTmp.boxIndex_ = boxIndex;
				dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
				dataTmp.cameraIndex_ = cameraIndex;
				dataTmp.triggerNumber_ = -1;
				memcpy(dataTmp.saveName_, triggerSaveName.c_str(), sizeof(dataTmp.saveName_));

				sendPackage_.command_ = Communication_Camera_Trigger_Continous;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				cameraControlMessage_.boxIndex_ = boxIndex;
				cameraControlMessage_.cameraIndex_ = cameraIndex;
				emit OperationFinished(cameraControlMessage_);
			}
			else {				//操作当前服务器下的所有相机
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					for (int32_t j = 0; j < serverVec_[serverIndex].boxVec_[i].cameraVec_.size(); ++j) {
						int boxIndex = i;
						int cameraIndex = j;
						std::string triggerSaveName = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].saveName_;
						CameraTriggerContinousPackage dataTmp;
						dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
						dataTmp.boxIndex_ = boxIndex;
						dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
						dataTmp.cameraIndex_ = cameraIndex;
						dataTmp.triggerNumber_ = -1;
						memcpy(dataTmp.saveName_, triggerSaveName.c_str(), sizeof(dataTmp.saveName_));

						sendPackage_.command_ = Communication_Camera_Trigger_Continous;
						memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
						cameraControlMessage_.status_ = SendData();
						cameraControlMessage_.boxIndex_ = boxIndex;
						cameraControlMessage_.cameraIndex_ = cameraIndex;
						emit OperationFinished(cameraControlMessage_);
					}
				}
			}
			break;
		}
		case Communication_Camera_Trigger_Single: {			//trigger single
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				std::string triggerSaveName = _cameraControlMessage.triggerSingleSaveName_;
				CameraTriggerSinglePackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
				dataTmp.boxIndex_ = boxIndex;
				dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
				dataTmp.cameraIndex_ = cameraIndex;
				dataTmp.triggerNumber_ = 1;
				memcpy(dataTmp.saveName_, triggerSaveName.c_str(), sizeof(dataTmp.saveName_));

				sendPackage_.command_ = Communication_Camera_Trigger_Single;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				cameraControlMessage_.boxIndex_ = boxIndex;
				cameraControlMessage_.cameraIndex_ = cameraIndex;
				emit OperationFinished(cameraControlMessage_);
			}
			else {				//操作当前服务器下的所有相机
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					for (int32_t j = 0; j < serverVec_[serverIndex].boxVec_[i].cameraVec_.size(); ++j) {
						int boxIndex = i;
						int cameraIndex = j;
						std::string triggerSaveName = _cameraControlMessage.triggerSingleSaveName_;
						CameraTriggerSinglePackage dataTmp;
						dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
						dataTmp.boxIndex_ = boxIndex;
						dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
						dataTmp.cameraIndex_ = cameraIndex;
						dataTmp.triggerNumber_ = 1;
						memcpy(dataTmp.saveName_, triggerSaveName.c_str(), sizeof(dataTmp.saveName_));

						sendPackage_.command_ = Communication_Camera_Trigger_Single;
						memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
						cameraControlMessage_.status_ = SendData();
						cameraControlMessage_.boxIndex_ = boxIndex;
						cameraControlMessage_.cameraIndex_ = cameraIndex;
						emit OperationFinished(cameraControlMessage_);
					}
				}
			}
			break;
		}
		case Communication_Camera_Get_Image: {			//get image
			formVector_ = _formVector;
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;

				CameraGetImagePackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxAmount_;
				dataTmp.boxIndex_ = boxIndex;
				dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraAmount_;
				dataTmp.cameraIndex_ = cameraIndex;
				dataTmp.imageType_ = _cameraControlMessage.imageType_;
				dataTmp.resizeFactor_ = _cameraControlMessage.imageResizedFactor_ > 0 ? _cameraControlMessage.imageResizedFactor_ : 1;
				dataTmp.resizedWidth_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].width_ / dataTmp.resizeFactor_;
				dataTmp.resizedHeight_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].height_ / dataTmp.resizeFactor_;

				sendPackage_.command_ = Communication_Camera_Get_Image;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				///解析接收到的数据
				if (receivePackage_.status_ == Communication_Camera_Get_Image_Ok) {
					CameraGetImagePackage receiveDataTmp;
					memcpy(&receiveDataTmp, receivePackage_.data_, sizeof(CameraGetImagePackage));
					if (dataTmp == receiveDataTmp) {
						cameraControlMessage_.status_ = receivePackage_.status_;
						cameraControlMessage_.boxIndex_ = receiveDataTmp.boxIndex_;
						cameraControlMessage_.cameraIndex_ = receiveDataTmp.cameraIndex_;
						cameraControlMessage_.imageType_ = receivePackage_.dataSize_;        ////////////////////////////////////////////
						cameraControlMessage_.imageResizedFactor_ = receiveDataTmp.resizeFactor_;
						cameraControlMessage_.imageResizedWidth_ = receiveDataTmp.resizedWidth_;
						cameraControlMessage_.imageResizedHeight_ = receiveDataTmp.resizedHeight_;
						cameraControlMessage_.cameraAmount_ = receiveDataTmp.cameraAmount_;
						int32_t jpegdatatotallength = receivePackage_.dataSize_ - sizeof(CameraGetImagePackage);
						char *jpegdatas = receivePackage_.data_ + sizeof(CameraGetImagePackage);
						if (cameraControlMessage_.images_jpeg_raw.size() == receiveDataTmp.cameraAmount_)
						{

							for (int cameraIndex = 0; cameraIndex < receiveDataTmp.cameraAmount_; cameraIndex++)
							{
								int32_t jpegdatalength = 0;
								memcpy(&jpegdatalength, jpegdatas, sizeof(int32_t));
								jpegdatas = jpegdatas + sizeof(int32_t);
								memcpy(cameraControlMessage_.images_jpeg_raw[cameraIndex], jpegdatas, jpegdatalength);
								//cameraControlMessage_.images_[cameraIndex]->length = jpegdatalength;
								*(cameraControlMessage_.images_jpeg_len[cameraIndex]) = jpegdatalength;
								jpegdatas = jpegdatas + jpegdatalength;
							}

						}
						cameraControlMessage_.imageSize_ = jpegdatatotallength;
					}
				}
				emit OperationFinished(cameraControlMessage_);
			}
			else {
				//TODO: delete this part
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					for (int32_t j = 0; j < serverVec_[serverIndex].boxVec_[i].cameraVec_.size(); ++j) {
						int boxIndex = i;
						int cameraIndex = j;

						CameraGetImagePackage dataTmp;
						dataTmp.boxAmount_ = serverVec_[serverIndex].boxAmount_;
						dataTmp.boxIndex_ = boxIndex;
						dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraAmount_;
						dataTmp.cameraIndex_ = cameraIndex;
						dataTmp.imageType_ = _cameraControlMessage.imageType_;
						dataTmp.resizeFactor_ = _cameraControlMessage.imageResizedFactor_ > 0 ? _cameraControlMessage.imageResizedFactor_ : 1;
						dataTmp.resizedWidth_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].width_ / dataTmp.resizeFactor_;
						dataTmp.resizedHeight_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].height_ / dataTmp.resizeFactor_;

						sendPackage_.command_ = Communication_Camera_Get_Image;
						memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
						cameraControlMessage_.status_ = SendData();
						///解析接收到的数据
						if (receivePackage_.status_ == Communication_Camera_Get_Image_Ok) {
							CameraGetImagePackage receiveDataTmp;
							memcpy(&receiveDataTmp, receivePackage_.data_, sizeof(CameraGetImagePackage));
							if (dataTmp == receiveDataTmp) {
								cameraControlMessage_.status_ = receivePackage_.status_;
								cameraControlMessage_.boxIndex_ = receiveDataTmp.boxIndex_;
								cameraControlMessage_.cameraIndex_ = receiveDataTmp.cameraIndex_;
								cameraControlMessage_.imageType_ = receiveDataTmp.imageType_;
								cameraControlMessage_.imageResizedFactor_ = receiveDataTmp.resizeFactor_;
								cameraControlMessage_.imageResizedWidth_ = receiveDataTmp.resizedWidth_;
								cameraControlMessage_.imageResizedHeight_ = receiveDataTmp.resizedHeight_;
								cameraControlMessage_.cameraAmount_ = receiveDataTmp.cameraAmount_;
								if (cameraControlMessage_.images_jpeg_raw.size() != 0)
								{
									cameraControlMessage_.images_jpeg_raw.clear();
								}
								int32_t jpegdatatotallength = receivePackage_.dataSize_ - sizeof(CameraGetImagePackage);
								char *jpegdatas = receivePackage_.data_ + sizeof(CameraGetImagePackage);
								if (cameraControlMessage_.images_jpeg_raw.size() == receiveDataTmp.cameraAmount_)
								{
									for (int cameraIndex = 0; cameraIndex < receiveDataTmp.cameraAmount_; cameraIndex++)
									{
										int32_t jpegdatalength = 0;
										memcpy(&jpegdatalength, jpegdatas, sizeof(int32_t));
										jpegdatas = jpegdatas + sizeof(int32_t);
										//memcpy(cameraControlMessage_.images_[cameraIndex]->data, jpegdatas, jpegdatalength);
										//cameraControlMessage_.images_[cameraIndex]->length = jpegdatalength;
										memcpy(cameraControlMessage_.images_jpeg_raw[cameraIndex], jpegdatas, jpegdatalength);
										*(cameraControlMessage_.images_jpeg_len[cameraIndex]) = jpegdatalength;
										jpegdatas = jpegdatas + jpegdatalength;
									}
								}
								cameraControlMessage_.imageSize_ = jpegdatatotallength;
							}
						}
						emit OperationFinished(cameraControlMessage_);
					}
				}
			}
			break;
		}
		case Communication_Camera_Close_Box: {			//close box
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				CameraCloseBoxPackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
				dataTmp.boxIndex_ = boxIndex;
				memcpy(dataTmp.macAddress_, serverVec_[serverIndex].boxVec_[boxIndex].mac_.c_str(), sizeof(dataTmp.macAddress_));

				sendPackage_.command_ = Communication_Camera_Close_Box;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				cameraControlMessage_.boxIndex_ = boxIndex;
				emit OperationFinished(cameraControlMessage_);
			}
			else {			//操作当前服务器下的所有盒子
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					int boxIndex = i;
					CameraCloseBoxPackage dataTmp;
					dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
					dataTmp.boxIndex_ = boxIndex;
					memcpy(dataTmp.macAddress_, serverVec_[serverIndex].boxVec_[boxIndex].mac_.c_str(), sizeof(dataTmp.macAddress_));

					sendPackage_.command_ = Communication_Camera_Close_Box;
					memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
					cameraControlMessage_.status_ = SendData();
					cameraControlMessage_.boxIndex_ = boxIndex;
					emit OperationFinished(cameraControlMessage_);
				}
			}
			break;
		}
		case Communication_Camera_Close_Camera: {			//close camera
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				CameraCloseCameraPackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
				dataTmp.boxIndex_ = boxIndex;
				dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
				dataTmp.cameraIndex_ = cameraIndex;

				sendPackage_.command_ = Communication_Camera_Close_Camera;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				cameraControlMessage_.boxIndex_ = boxIndex;
				cameraControlMessage_.cameraIndex_ = cameraIndex;
				emit OperationFinished(cameraControlMessage_);
			}
			else {			//操作当前服务器下的所有相机
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					for (int32_t j = 0; j < serverVec_[serverIndex].boxVec_[i].cameraVec_.size(); ++j) {
						int boxIndex = i;
						int cameraIndex = j;
						CameraCloseCameraPackage dataTmp;
						dataTmp.boxAmount_ = serverVec_[serverIndex].boxVec_.size();
						dataTmp.boxIndex_ = boxIndex;
						dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size();
						dataTmp.cameraIndex_ = cameraIndex;

						sendPackage_.command_ = Communication_Camera_Close_Camera;
						memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
						cameraControlMessage_.status_ = SendData();
						cameraControlMessage_.boxIndex_ = boxIndex;
						cameraControlMessage_.cameraIndex_ = cameraIndex;
						emit OperationFinished(cameraControlMessage_);
					}
				}
			}
			break;
		}
		case Communication_Camera_Reset_Id: {			//reset id
			if (!operateAllFlag) {
				int boxIndex = _cameraControlMessage.boxIndex_;
				int cameraIndex = _cameraControlMessage.cameraIndex_;
				CameraResetIdPackage dataTmp;
				dataTmp.boxAmount_ = serverVec_[serverIndex].boxAmount_;
				dataTmp.boxIndex_ = boxIndex;
				dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraAmount_;
				dataTmp.cameraIndex_ = cameraIndex;

				sendPackage_.command_ = Communication_Camera_Reset_Id;
				memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
				cameraControlMessage_.status_ = SendData();
				cameraControlMessage_.boxIndex_ = boxIndex;
				cameraControlMessage_.cameraIndex_ = cameraIndex;
				emit OperationFinished(cameraControlMessage_);
				break;
			}
			else {					//操作当前服务器下的所有相机
				for (int32_t i = 0; i < serverVec_[serverIndex].boxVec_.size(); ++i) {
					for (int32_t j = 0; j < serverVec_[serverIndex].boxVec_[i].cameraVec_.size(); ++j) {
						int boxIndex = i;
						int cameraIndex = j;
						CameraResetIdPackage dataTmp;
						dataTmp.boxAmount_ = serverVec_[serverIndex].boxAmount_;
						dataTmp.boxIndex_ = boxIndex;
						dataTmp.cameraAmount_ = serverVec_[serverIndex].boxVec_[boxIndex].cameraAmount_;
						dataTmp.cameraIndex_ = cameraIndex;

						sendPackage_.command_ = Communication_Camera_Reset_Id;
						memcpy(sendPackage_.data, &dataTmp, sizeof(dataTmp));
						cameraControlMessage_.status_ = SendData();
						cameraControlMessage_.boxIndex_ = boxIndex;
						cameraControlMessage_.cameraIndex_ = cameraIndex;
						emit OperationFinished(cameraControlMessage_);
					}
				}
			}
		}
		default:
			break;
		}
	}
}


CameraCommunication::CameraCommunication()
{
	////初始化默认的相机配置参数,用于生成配置表
	//CameraServerUnitTypeDef serverUnitTmp;
	//serverVec_.clear();
	//for (int32_t i = 0; i < serverAmount; ++i) {
	//	serverVec_.push_back(serverUnitTmp);
	//}
	//SaveConfigFile(QString::fromStdString(configFileName_), serverVec_);

	serverVec_.clear();
	formVector.resize(4096);
	for (int i=0;i <formVector.size();i++)
	{
		formVector[i] = i/16;
	}        //默认取高八位
	//加载默认的配置文件

	LoadConfigFile(QString::fromStdString(configFileName_));

	updateTimer = new QTimer(this);
	QObject::connect(updateTimer, SIGNAL(timeout()), this, SLOT(TimerTimeout()));
	updateTimer->setInterval(updateTimerInterval);
	updateTimer->start();
}


CameraCommunication::~CameraCommunication()
{

	
}

BaseErrorType CameraCommunication::SaveConfigFile(QString _file, std::vector<CameraServerUnitTypeDef> &_serverVec)
{
	///采用opencv的yml文件保存参数,保存路径为当前工作目录下
	///注意保存为多级节点时的写法，各节点名不要带":"
	cv::FileStorage configFileTmp(_file.toStdString(), cv::FileStorage::WRITE);
	if (!configFileTmp.isOpened() || _serverVec.empty()) {
		MY_ERROR(Error_WriteFile_Failed, _file.toStdString().c_str());
		emit SaveConfigFileFinished(false);
	}
	std::ostringstream strTmp;
	//第一级的服务器节点
	configFileTmp << "serverAmount" << int32_t(_serverVec.size());
	for (int32_t i = 0; i < _serverVec.size(); ++i) {
		_serverVec[i].id_ = i;
		strTmp.str("");
		strTmp << "server_" << i;
		configFileTmp << strTmp.str() << "[" << "{:";
		configFileTmp << "id" << _serverVec[i].id_;
		configFileTmp << "ip" << _serverVec[i].ip_;
		configFileTmp << "port" << _serverVec[i].port_;

		//第二级的盒子节点
		_serverVec[i].boxAmount_ = _serverVec[i].boxVec_.size();
		configFileTmp << "boxAmount" << _serverVec[i].boxAmount_;
		for (int32_t j = 0; j < _serverVec[i].boxVec_.size(); ++j) {
			strTmp.str("");
			strTmp << "box_" << j;
			configFileTmp << strTmp.str() << "[" << "{:";
			configFileTmp << "id" << _serverVec[i].boxVec_[j].id_;
			configFileTmp << "mac" << _serverVec[i].boxVec_[j].mac_;

			//第三级的相机节点
			_serverVec[i].boxVec_[j].cameraAmount_ = _serverVec[i].boxVec_[j].cameraVec_.size();
			configFileTmp << "cameraAmount" << _serverVec[i].boxVec_[j].cameraAmount_;
			for (int32_t k = 0; k < _serverVec[i].boxVec_[j].cameraVec_.size(); ++k) {
				strTmp.str("");
				strTmp << "camera_" << k;
				configFileTmp << strTmp.str() << "[" << "{:";
				configFileTmp << "id" << _serverVec[i].boxVec_[j].cameraVec_[k].id_;
				configFileTmp << "width" << _serverVec[i].boxVec_[j].cameraVec_[k].width_;
				configFileTmp << "height" << _serverVec[i].boxVec_[j].cameraVec_[k].height_;
				configFileTmp << "exposure" << _serverVec[i].boxVec_[j].cameraVec_[k].exposure_;
				configFileTmp << "gain" << _serverVec[i].boxVec_[j].cameraVec_[k].gain_;
				configFileTmp << "brightness" << _serverVec[i].boxVec_[j].cameraVec_[k].brightness_;
				configFileTmp << "contrast" << _serverVec[i].boxVec_[j].cameraVec_[k].contrast_;
				configFileTmp << "bitLut" << _serverVec[i].boxVec_[j].cameraVec_[k].bitLut_;
				configFileTmp << "saveFormat" << _serverVec[i].boxVec_[j].cameraVec_[k].saveFormat_;
				configFileTmp << "skipNumber" << _serverVec[i].boxVec_[j].cameraVec_[k].skipNumber_;
				configFileTmp << "triggerMode" << _serverVec[i].boxVec_[j].cameraVec_[k].triggerMode_;
				configFileTmp << "savePath" << _serverVec[i].boxVec_[j].cameraVec_[k].savePath_;
				configFileTmp << "saveName" << _serverVec[i].boxVec_[j].cameraVec_[k].saveName_;
				configFileTmp << "}" << "]";
			}
			configFileTmp << "}" << "]";
		}
		configFileTmp << "}" << "]";
	}
	configFileTmp.release();

	/*UpdateLocalParameters(_serverVec);*/
	emit SaveConfigFileFinished(true);
	return No_Error;
}

BaseErrorType CameraCommunication::LoadConfigFile(QString _file)
{
	///采用opencv的yml文件保存参数,保存路径为当前工作目录下
	///注意读取多级节点时要采用iterator，各节点名不要带":"
	std::vector<CameraServerUnitTypeDef> serverVecTmp;
	cv::FileStorage configFileTmp(_file.toStdString(), cv::FileStorage::READ);
	if (!configFileTmp.isOpened()) {
		MY_ERROR(Error_ReadFile_Failed, _file.toStdString().c_str());
		emit LoadConfigFileFinished(_file,false, serverVecTmp);
	}

	std::ostringstream strTmp;
	configFileTmp["serverAmount"] >> serverAmount;
	//第一级的服务器节点
	serverVecTmp.clear();
	for (int32_t i = 0; i < serverAmount; ++i) {
		CameraServerUnitTypeDef serverUnitTmp;
		strTmp.str("");
		strTmp << "server_" << i;
		cv::FileNodeIterator serverFileNodeItr = configFileTmp[strTmp.str()].begin();
		(*serverFileNodeItr)["id"] >> serverUnitTmp.id_;
		(*serverFileNodeItr)["ip"] >> serverUnitTmp.ip_;
		(*serverFileNodeItr)["port"] >> serverUnitTmp.port_;
		(*serverFileNodeItr)["boxAmount"] >> serverUnitTmp.boxAmount_;

		//第二级的盒子节点
		serverUnitTmp.boxVec_.clear();
		for (int32_t j = 0; j < serverUnitTmp.boxAmount_; ++j) {
			CameraBoxUnitTypeDef boxUnitTmp;
			strTmp.str("");
			strTmp << "box_" << j;
			cv::FileNodeIterator boxFileNodeItr = (*serverFileNodeItr)[strTmp.str()].begin();
			(*boxFileNodeItr)["id"] >> boxUnitTmp.id_;
			(*boxFileNodeItr)["mac"] >> boxUnitTmp.mac_;
			(*boxFileNodeItr)["cameraAmount"] >> boxUnitTmp.cameraAmount_;

			//第三级的相机节点
			boxUnitTmp.cameraVec_.clear();
			for (int32_t k = 0; k < boxUnitTmp.cameraAmount_; ++k) {
				CameraParametersUnitTypeDef cameraUnitTmp;
				strTmp.str("");
				strTmp << "camera_" << k;
				cv::FileNodeIterator cameraFileNodeItr = (*boxFileNodeItr)[strTmp.str()].begin();
				(*cameraFileNodeItr)["id"] >> cameraUnitTmp.id_;
				(*cameraFileNodeItr)["width"] >> cameraUnitTmp.width_;
				(*cameraFileNodeItr)["height"] >> cameraUnitTmp.height_;
				(*cameraFileNodeItr)["exposure"] >> cameraUnitTmp.exposure_;
				(*cameraFileNodeItr)["gain"] >> cameraUnitTmp.gain_;
				(*cameraFileNodeItr)["brightness"] >> cameraUnitTmp.brightness_;
				(*cameraFileNodeItr)["contrast"] >> cameraUnitTmp.contrast_;
				(*cameraFileNodeItr)["bitLut"] >> cameraUnitTmp.bitLut_;
				(*cameraFileNodeItr)["saveFormat"] >> cameraUnitTmp.saveFormat_;
				(*cameraFileNodeItr)["skipNumber"] >> cameraUnitTmp.skipNumber_;
				(*cameraFileNodeItr)["triggerMode"] >> cameraUnitTmp.triggerMode_;
				(*cameraFileNodeItr)["savePath"] >> cameraUnitTmp.savePath_;
				(*cameraFileNodeItr)["saveName"] >> cameraUnitTmp.saveName_;
				boxUnitTmp.cameraVec_.push_back(cameraUnitTmp);
			}
			serverUnitTmp.boxVec_.push_back(boxUnitTmp);
		}
		serverVecTmp.push_back(serverUnitTmp);
	}
	UpdateLocalParameters(serverVecTmp);
	//_servervec = serverVecTmp;
	configFileTmp.release();
	emit LoadConfigFileFinished(_file, true, serverVecTmp);
	return No_Error;
}

void CameraCommunication::StartStopTimer(bool _flag)
{
	if (_flag) {
		updateTimer->start(updateTimerInterval);
	}
	else {
		updateTimer->stop();
	}
}

void CameraCommunication::TimerTimeout(void)
{

}

void CameraCommunication::UpdateLocalParameters(const std::vector<CameraServerUnitTypeDef> &_serverVec)
{
	///更新服务器参数
	serverAmount = serverVec_.size();
	serverVec_ = _serverVec;

	///更新线程管理数组
	if (threadVec_.size() < serverVec_.size()) {
		while (threadVec_.size() < serverVec_.size()) {
			threadVec_.push_back(new QThread(this));
		}
	}
	else if (threadVec_.size() > serverVec_.size()) {
		while (threadVec_.size() > serverVec_.size()) {
			QThread *tmp = threadVec_.back();
			tmp->quit();
			tmp->wait();
			threadVec_.pop_back();
		}
	}

	///更新socket管理数组
	if (cameraCommunicationThreadVec_.size() < serverVec_.size()) {
		while (cameraCommunicationThreadVec_.size() < serverVec_.size()) {
			int threadIndex = cameraCommunicationThreadVec_.size();
			cameraCommunicationThreadVec_.push_back(new CameraCommunicationThread(threadIndex,serverVec_));
			cameraCommunicationThreadVec_.back()->moveToThread(threadVec_[threadIndex]);
			CameraCommunicationThread *tmp = cameraCommunicationThreadVec_.back();
			//自定义信号与槽的数据类型
			qRegisterMetaType<CameraControlMessage>("CameraControlMessage &");
			qRegisterMetaType<std::vector<CameraServerUnitTypeDef> >("std::vector<CameraServerUnitTypeDef> &");
			qRegisterMetaType<QVector<int>>("QVector<int> &");
			QObject::connect(this, SIGNAL(SendOperationToServer(CameraControlMessage &, std::vector<CameraServerUnitTypeDef> &, QVector<int> &)), tmp, SLOT(StartOperation(CameraControlMessage &, std::vector<CameraServerUnitTypeDef> &, QVector<int> &)), Qt::QueuedConnection);
			QObject::connect(tmp, SIGNAL(OperationFinished(CameraControlMessage &)), this, SLOT(ReceiveOperationFinishedFromServer(CameraControlMessage &)), Qt::QueuedConnection);
			threadVec_[threadIndex]->start(QThread::NormalPriority);
		}
	}
	else if (cameraCommunicationThreadVec_.size() > serverVec_.size()) {
		while (cameraCommunicationThreadVec_.size() > serverVec_.size()) {
			///注意要删除对象以调用对象的析构函数
			delete cameraCommunicationThreadVec_.back();
			cameraCommunicationThreadVec_.pop_back();
		}
	}
}

void CameraCommunication::StartOperation(CameraControlMessage &_cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec)
{
	UpdateLocalParameters(_serverVec);
	emit SendOperationToServer(_cameraControlMessage,_serverVec, formVector);
}

void CameraCommunication::ReceiveOperationFinishedFromServer(CameraControlMessage &_cameraControlMessage)
{
	emit OperationFinished(_cameraControlMessage);
}
void CameraCommunication::Receive8bitForm(QVector<int> _F)
{
	int x = 0;
	formVector = _F;
}
