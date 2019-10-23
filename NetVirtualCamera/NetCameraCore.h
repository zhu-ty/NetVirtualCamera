
#pragma once
#define CSharp

#ifdef CSharp
#include "CSharpSocketCppExport.h"
#else
#include "AsioSocket.h"
#endif

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

//#using "../NetVirtualCamera/CSharpSocket.dll"


#define WAIT_MS_INIT 20000


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
	static quint32 socketReadWaitForMs_;						//xxxxxxxxxxxxxx
	QTimer *updateTimer;			//xxTimerxxxxxxxxxxxxxxxx
	static const int32_t updateTimerInterval = 1000;		//xxxx
	QVector<int> formVector_;      //xxxxxxxxxx
signals:
	void OperationFinished(CameraControlMessage _cameraControlMessage);

public slots:
	void UpdateStatus(void);												//xxxxxxxxxxxxxxxxxxxxxxxx
	void SocketStateChanged(void);
	
	void StartOperation(CameraControlMessage _cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec, QVector<int> &_formVector);

private:
	int id_;																//xxxxxxxxxxxxxxxxxxxxxxid
	CameraClientSendPackageTypdef sendPackage_;								//xxxxxxxxxx			
	CameraClientReceivePackageTypeDef receivePackage_;						//xxxxxxxxxx
	std::vector<CameraServerUnitTypeDef> serverVec_;						//xxxxxxxxxxxxxxxxxxxx
	// CameraControlMessage cameraControlMessage_;								//xxxxxxxxxxxxxx
	//QTcpSocket *tcpSocket_;													//socketxxxxxx
	/*CSharpSocket::SKTcpSocket ^tcpSocket_;*/
#ifdef CSharp
	SKSocket *tcpSocket_;
#else
	AsioSocket *tcpSocket_;
#endif
	
	void ResetSocket(void);
	Communication_Camera_Status SendData(void);				
	
};


class CameraCommunication :public QObject
{
	Q_OBJECT
public:
	CameraCommunication();
	~CameraCommunication();
	QVector<int> formVector;      //xxxxxxxxxx				
	std::string configFileName_ = "./config.xml";			//xxxxxxxxxxxxxxxxxxxxxx,xxxxxxxxxxxmlxxxx
								   
	QTimer *updateTimer;									//xxTimerxxxxxxxxxxxxxxxx
	static const int32_t updateTimerInterval = 1000;

signals:
	void LoadConfigFileFinished(QString,bool _flag, std::vector<CameraServerUnitTypeDef> &_serverVec);
	void SaveConfigFileFinished(bool _flag);
	void SendOperationToServer(CameraControlMessage _cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec, QVector<int> &_formVector);	//xxxxxxxxxxxxsocket
	void OperationFinished(CameraControlMessage _cameraControlMessage);														//xxxxxxxxxxxxxxxxxx

public slots:
	BaseErrorType LoadConfigFile(QString _file);
	BaseErrorType SaveConfigFile(QString _file, std::vector<CameraServerUnitTypeDef> &_serverVec);
	void StartStopTimer(bool _flag);
	void TimerTimeout(void);						
	void StartOperation(CameraControlMessage _cameraControlMessage, std::vector<CameraServerUnitTypeDef> &_serverVec);		//xxxxxxxxxxxxxxxxxx,xxxxxxxxxxxxxxxxxx
	void ReceiveOperationFinishedFromServer(CameraControlMessage _cameraControlMessage);											//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	void Receive8bitForm(QVector<int> _F);

private:
	std::vector<QThread*> threadVec_;
	std::vector<CameraCommunicationThread*> cameraCommunicationThreadVec_;

	int32_t serverAmount = 1;																///xxxxxxxxxxxxxxxxxxxxxxxxxxxx
	std::vector<CameraServerUnitTypeDef> serverVec_;

	void UpdateLocalParameters(const std::vector<CameraServerUnitTypeDef> &_serverVec);		///xxxxxxxxxxxxxxxxxx
	
};
