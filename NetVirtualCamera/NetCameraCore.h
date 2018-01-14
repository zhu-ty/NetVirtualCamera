
#pragma once
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
//#include "NPPJpegCoder.h"
#include <time.h>

#include <QWidget>
#include <QTimer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface> 
#include <QThread>


#include "Netbase.h"
//#include "imageDisplayWidget.h"
//#include "matAndQImage.h"
//#include "omp.h"
#include "NetCameraHeader.h"

//#include "cuda_runtime.h"
//#include "device_launch_parameters.h"

class CameraCommunicationThread : public QObject
{
Q_OBJECT
public:
	CameraCommunicationThread(int _id,std::vector<CameraServerUnitTypeDef> &_serverVec);
	~CameraCommunicationThread();
	//npp::NPPJpegCoder decoder;   //cuda
	//cv::cuda::GpuMat *img;
	//cv::cuda::GpuMat *resizeimg;
	QTimer *updateTimer;			//该Timer用于更新设备状态
	static const int32_t updateTimerInterval = 1000;		//心跳
	QVector<int> formVector_;      //窗宽窗位表
signals:
	void OperationFinished(CameraControlMessage &_cameraControlMessage);

public slots:
	void UpdateStatus(void);												//向全局参数表更新状态参数
	void SocketStateChanged(void);
	
	void StartOperation(CameraControlMessage &_cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec, QVector<int> &_formVector);

private:
	int id_;																//该线程对应的相机服务器id
	CameraClientSendPackageTypdef sendPackage_;								//发送消息包			
	CameraClientReceivePackageTypeDef receivePackage_;						//接收消息包
	std::vector<CameraServerUnitTypeDef> serverVec_;						//存储全局相机控制参数
	CameraControlMessage cameraControlMessage_;								//相机控制消息包
	QTcpSocket *tcpSocket_;													//socket通信子
	static const quint32 socketReadWaitForMs_ = 6000;						//阻塞的最长时间
	void ResetSocket(void);
	Communication_Camera_Status SendData(void);				
	
};


class CameraCommunication :public QObject
{
	Q_OBJECT
public:
	CameraCommunication();
	~CameraCommunication();
	QVector<int> formVector;      //窗换窗位表				
	std::string configFileName_ = "./config.xml";			//默认的配置文件保存路径,注意只能用xml文件
								   
	QTimer *updateTimer;									//该Timer用于更新设备状态
	static const int32_t updateTimerInterval = 1000;

signals:
	void LoadConfigFileFinished(QString,bool _flag, std::vector<CameraServerUnitTypeDef> &_serverVec);
	void SaveConfigFileFinished(bool _flag);
	void SendOperationToServer(CameraControlMessage &_cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec, QVector<int> &_formVector);	//通知各服务器socket
	void OperationFinished(CameraControlMessage &_cameraControlMessage);														//用于发送给各请求者

public slots:
	BaseErrorType LoadConfigFile(QString _file);
	BaseErrorType SaveConfigFile(QString _file, std::vector<CameraServerUnitTypeDef> &_serverVec);
	void StartStopTimer(bool _flag);
	void TimerTimeout(void);						
	void StartOperation(CameraControlMessage &_cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec);		//接收各请求者的请求,然后转发给各服务器
	void ReceiveOperationFinishedFromServer(CameraControlMessage &_cameraControlMessage);											//接收各服务器的完成信号，然后转发给请求者
	void Receive8bitForm(QVector<int> _F);

private:
	std::vector<QThread*> threadVec_;
	std::vector<CameraCommunicationThread*> cameraCommunicationThreadVec_;

	int32_t serverAmount = 1;																///该变量用于读取配置文件时使用
	std::vector<CameraServerUnitTypeDef> serverVec_;

	void UpdateLocalParameters(const std::vector<CameraServerUnitTypeDef> &_serverVec);		///更新通信用设置参数
	
};
