/**
@brief Generic Camera Driver Class
Implementation of XIMEA camera
@author Shane Yuan
@date Dec 31, 2017
*/

#include "NetVirtualCamera.h"

namespace cam {
	/**
	@brief thread capturing function (jpeg buffer)
	used for continous mode
	thread function to get images from camera
	*/
	void GenCameraNETVIR::capture_thread_JPEG_()
	{
		clock_t begin_time, end_time;
		//TODO: Can't set fps for each camera right now
		double time = 1000.0 / static_cast<double>(camInfos[0].fps);
		for (;;) {
			// begin time
			begin_time = clock();
			// check status
			if (thexit == 1)
				break;
			// capture image
			// this->captureFrame(camInd, bufferImgs_singleframe[camInd]);
			int camInd = 0;
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				cameraControlMessage_.serverIndex_ = serverIndex;
				int boxIndex = 0;
				cameraControlMessage_.command_ = Communication_Camera_Get_Image;//Get Image
				cameraControlMessage_.status_ = Communication_Camera_Get_Image_Invalid;
				cameraControlMessage_.imageType_ = 2;	
				cameraControlMessage_.imageResizedFactor_ = 0;
				cameraControlMessage_.cameraIndex_ = 0;
				cameraControlMessage_.cameraAmount_ = serverVec_[serverIndex].boxVec_[0].cameraAmount_;
				cameraControlMessage_.images_jpeg_raw.clear();
				cameraControlMessage_.images_jpeg_len.clear();
				for (int cameraIndex = 0; cameraIndex < cameraControlMessage_.cameraAmount_; cameraIndex++)
				{
					cameraControlMessage_.images_jpeg_raw.push_back(
						bufferImgs[thBufferInds[camInd]][camInd].data);
					cameraControlMessage_.images_jpeg_len.push_back(
						&bufferImgs[thBufferInds[camInd]][camInd].length);
					camInd++;
				}
				emit StartOperation(cameraControlMessage_, serverVec_);
				image_receiving_flag[serverIndex] = 1;
			}
			wait_for_image_receive();
			// end time
			end_time = clock();
			float waitTime = time - static_cast<double>(end_time - begin_time) / CLOCKS_PER_SEC * 1000;
			// increase index
			for (int camIndTmp = 0; camIndTmp < this->cameraNum; camIndTmp++)
			{
				if (camPurpose == GenCamCapturePurpose::Streaming)
					thBufferInds[camIndTmp] = (thBufferInds[camIndTmp] + 1) % bufferSize;
				else {
					thBufferInds[camIndTmp] = thBufferInds[camIndTmp] + 1;
					if (thBufferInds[camIndTmp] == bufferSize) {
						thexit = 1;//TODO: with only one thread, just exit
					}
				}
			}
			// wait for some time
			if (isVerbose) {
				printf("GenCameraNETVIR::capture_thread_JPEG_ Camera all captures one frame, wait %lld milliseconds for next frame ...\n",
					static_cast<long long>(waitTime));
			}
			if (waitTime > 0) {
				SysUtil::sleep(waitTime);
			}
		}
		SysUtil::infoOutput("GenCameraNETVIR::capture_thread_JPEG_:: Capturing thread for camera all finish, exit successfully !");
	}

	int GenCameraNETVIR::wait_for_image_receive()
	{
		for (int j = 0; j < serverVec_.size(); j++)
		{
			if (image_receiving_flag[j] == 0)
			{
				return 0;
			}
		}
		for (int i = 0;;)
		{
			bool breakflag = true;
			for (int j = 0; j < serverVec_.size(); j++)
			{
				if (image_receiving_flag[j] != -1)
				{
					breakflag = false;
				}
			}
			if (breakflag == true)
				break;
			else if (i > wait_time_local)
			{
				SysUtil::warningOutput("GenCameraNETVIR::wait_for_image_receive wait too long, check your network");
				break;
			}
			else
			{
				i += 5;
				SysUtil::sleep(5);
			}
		}
		for (int j = 0; j < serverVec_.size(); j++)
		{
			image_receiving_flag[j] = 0;
		}
		return 0;
	}

	// constructor
	GenCameraNETVIR::GenCameraNETVIR() {
		this->camModel = CameraModel::Network;
		communication_camera = new CameraCommunication();
		communication_camera->moveToThread(&communication_thread);
		//communication_camera->StartStopTimer(true);
		//Qt Connect
		qRegisterMetaType<CameraControlMessage>("CameraControlMessage &");   //自定义信号与槽函数类型
		qRegisterMetaType<std::vector<CameraServerUnitTypeDef> >("std::vector<CameraServerUnitTypeDef> &");
		QObject::connect(this,
			SIGNAL(StartOperation(CameraControlMessage &, std::vector<CameraServerUnitTypeDef> &)),
			communication_camera,
			SLOT(StartOperation(CameraControlMessage &, std::vector<CameraServerUnitTypeDef> &)),
			Qt::QueuedConnection);
		QObject::connect(communication_camera, SIGNAL(OperationFinished(CameraControlMessage &)),
			this, SLOT(OperationFinished(CameraControlMessage &)), Qt::DirectConnection);

		//QObject::connect(this, SIGNAL(LoadConfigFile(QString)),
		//	communication_camera, SLOT(LoadConfigFile(QString)), Qt::QueuedConnection);
		//QObject::connect(communication_camera, SIGNAL(LoadConfigFileFinished(QString, bool, std::vector<CameraServerUnitTypeDef> &)),
		//	this, SLOT(LoadConfigFileFinished(QString, bool, std::vector<CameraServerUnitTypeDef> &)), Qt::QueuedConnection);

		QObject::connect(this, SIGNAL(LoadConfigFile(QString)),
			communication_camera, SLOT(LoadConfigFile(QString)), Qt::QueuedConnection);
		QObject::connect(communication_camera, 
			SIGNAL(LoadConfigFileFinished(QString, bool, std::vector<CameraServerUnitTypeDef> &)),
			this, 
			SLOT(LoadConfigFileFinished(QString, bool, std::vector<CameraServerUnitTypeDef> &)), Qt::DirectConnection);

	}
	GenCameraNETVIR::~GenCameraNETVIR() {}

	/***********************************************************/
	/*                 Qt and network functions                */
	/***********************************************************/

	///**
	//@brief Network CameraControl Send (In main thread)
	//@param CameraControlMessage & _cameraControlMessage: Message used to control the camera
	//@param std::vector<CameraServerUnitTypeDef> & _serverVec: ServerVectorList
	//*/
	//void GenCameraNETVIR::StartOperation(CameraControlMessage &_cameraControlMessage,
	//	std::vector<CameraServerUnitTypeDef> &_serverVec)
	//{
	//	emit StartOperation(cameraControlMessage_, serverVec_);
	//}

	/**
	@brief Network CameraControl Receive (In main thread)
	@param CameraControlMessage & _cameraControlMessage: Server's respond pack
	*/
	void GenCameraNETVIR::OperationFinished(CameraControlMessage &_cameraControlMessage)
	{
		if (_cameraControlMessage.requestorId_ == id_ || _cameraControlMessage.requestorId_ == 0) {
			cameraControlMessage_ = _cameraControlMessage;
			Communication_Camera_Command command = cameraControlMessage_.command_;
			Communication_Camera_Status status = cameraControlMessage_.status_;
			int serverIndex = cameraControlMessage_.serverIndex_;
			int boxIndex = cameraControlMessage_.boxIndex_;
			int cameraIndex = cameraControlMessage_.cameraIndex_;
			int cameraAmount = cameraControlMessage_.cameraAmount_;
			bool operateAllFlag = true;
			int currentBoxIndex = 0;
			int currentCameraIndex = 0;
			if (serverIndex < serverVec_.size() && boxIndex<serverVec_[serverIndex].boxVec_.size() && cameraIndex<serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_.size()) {
				switch (command) {
				case Communication_Camera_Get_Status: {			//update status
					switch (status) {
					case Communication_Camera_Get_Status_Ok: {
						serverVec_[serverIndex].connectedFlag_ = true;
						bool cameraIsConnect = true;
						for (int32_t i = 0; i < serverVec_.size(); i++)
						{
							if (serverVec_[i].connectedFlag_ == false)
								cameraIsConnect = false;
						}
						if (cameraIsConnect == true) {
							//SysUtil::infoOutput("Connected!");
							////if (!isInitial)StartOperation();
						}
						break;
					}
					case Communication_Camera_Get_Status_Invalid: {

						serverVec_[serverIndex].connectedFlag_ = false;
						SysUtil::warningOutput("Communication_Camera_Get_Status_Invalid!");
						break;
					}
					}
					break;
				}
				case Communication_Camera_Open_Box: {			//open box
					SysUtil::warningOutput("Communication_Camera_Open_Box not supported reply pack");
					break;
				}
				case Communication_Camera_Open_Camera: {			//open camera
					std::ostringstream strTmp("");
					int cameraId = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].id_;
					switch (status) {
					case Communication_Camera_Open_Camera_Ok: {

						SysUtil::infoOutput(("[Server" + QString::number(serverIndex, 10) + "]Open cameras OK.").toStdString());
						this->isCapture = true;
						break;
					}
					case Communication_Camera_Open_Camera_Invalid: {
						strTmp << "Failed to open Server_" << serverIndex << "-Box_" << boxIndex << "-Cam_" << cameraIndex << "-Id_" << cameraId << "!";
						SysUtil::warningOutput(("[Warning]" + QString::fromStdString(strTmp.str())).toStdString());
						this->isCapture = false;
						break;
					}
					case Communication_Camera_Action_Overtime: {
						strTmp << "Overtime to open Server_" << serverIndex << "-Box_" << boxIndex << "-Cam_" << cameraIndex << "-Id_" << cameraId << "!";      
						SysUtil::warningOutput(("[Warning]" + QString::fromStdString(strTmp.str())).toStdString());
						this->isCapture = false;
						break;
					}
					}

					break;
				}
				case Communication_Camera_Trigger_Continous: {			//trigger continous
					SysUtil::warningOutput("Communication_Camera_Trigger_Continous not supported reply pack");
					break;
				}
				case Communication_Camera_Trigger_Single: {			//trigger single
					SysUtil::warningOutput("Communication_Camera_Trigger_Single not supported reply pack");
					break;
				}
				case Communication_Camera_Get_Image: {			//get image
					std::ostringstream strTmp("");
					int cameraId = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].id_;
					image_receiving_flag[serverIndex] = -1;
					switch (status) {
					case Communication_Camera_Get_Image_Ok: {
						int32_t imagesize = _cameraControlMessage.imageSize_;
						SysUtil::infoOutput(("server" + QString::number(serverIndex, 10) +
							"getImage OK,imageSize=" + QString::number(imagesize, 10)).toStdString());
						break;
					}
					case Communication_Camera_Get_Image_Invalid: {
						SysUtil::warningOutput(("server" + QString::number(serverIndex, 10) +
							"_camera" + QString::number(cameraIndex, 10) + "getImage Invalid").toStdString());
						break;
					}
					case Communication_Camera_Action_Overtime: {
						SysUtil::warningOutput(("server" + QString::number(serverIndex, 10) +
							"_camera" + QString::number(cameraIndex, 10) + "getImage Overtime").toStdString());
						break;
					}
					}
					break;
				}
				case Communication_Camera_Close_Box: {			//close box
					SysUtil::warningOutput("Communication_Camera_Close_Box not supported reply pack");
					break;
				}

				case Communication_Camera_Close_Camera: {			//close camera
					std::ostringstream strTmp("");
					int cameraId = serverVec_[serverIndex].boxVec_[boxIndex].cameraVec_[cameraIndex].id_;
					switch (status) {
					case Communication_Camera_Close_Camera_Ok: {
						SysUtil::infoOutput(("[Server" + QString::number(serverIndex, 10) + "]Colse Camera").toStdString());
						this->isCapture = false;
						break;
					}
					case Communication_Camera_Close_Camera_Invalid: {
						strTmp << "Failed to close Server_" << serverIndex << "-Box_" << boxIndex << "-Cam_" << cameraIndex << "-Id_" << cameraId << "!";
						SysUtil::warningOutput(strTmp.str());
						this->isCapture = false;
						break;
					}
					case Communication_Camera_Action_Overtime: {
						strTmp << "Overtime to close Server_" << serverIndex << "-Box_" << boxIndex << "-Cam_" << cameraIndex << "-Id_" << cameraId << "!";
						SysUtil::warningOutput(strTmp.str());
						this->isCapture = false;
						break;
					}
					}
					break;
				}
				case Communication_Camera_Reset_Id: {			//reset id
					SysUtil::warningOutput("Communication_Camera_Reset_Id not supported reply pack");
					break;
				}
				default:
					break;
				}
			}
		}
	}



	/***********************************************************/
	/*                   basic camera functions                */
	/***********************************************************/
	/**
	@brief make setting effective
	by capturing some frames
	@param int k: capture image frames (default is 10)
	@return int
	*/
	int GenCameraNETVIR::makeSetEffective(int k) {
		if (!this->isCapture) {
			SysUtil::warningOutput("This function must be executed after " \
				"starting capturing !");
			return -1;
		}
		SysUtil::errorOutput("GenCameraNETVIR::makeSetEffective Function unused");
		return -1;
	}

	/**
	@brief init camera
	@return int
	*/
	int GenCameraNETVIR::init() {
		//ths.resize(this->cameraNum);
		//thStatus.resize(this->cameraNum);
		//communication_camera->LoadConfigFile(QString::fromStdString(communication_camera->configFileName_), serverVec_);
		communication_thread.start(QThread::NormalPriority);
		config_file_status = 0;
		emit LoadConfigFile(QString::fromStdString(communication_camera->configFileName_));
		for (int i = 0;;)
		{
			if (config_file_status == 1)
				break;
			else if (config_file_status == -1)
			{
				SysUtil::errorOutput("GenCameraNETVIR::init config file load error! check config file config.xml");
				return -1;
			}
			else if (i > wait_time_local)
			{
				SysUtil::warningOutput("GenCameraNETVIR::init wait too long, check config file");
				break;
			}
			else
			{
				SysUtil::sleep(5);
				i += 5;
			}
		}
		this->cameraNum = 0;
		for (int i = 0; i < serverVec_.size(); i++)
		{
			this->cameraNum += serverVec_[i].boxVec_[0].cameraAmount_;
			image_receiving_flag.push_back(0);
		}
		this->isInit = true;
		return 0;
	}

	/**
	@brief get camera information
	@param std::vector<GenCamInfo> & camInfos: output camera infos
	@return int
	*/
	int GenCameraNETVIR::getCamInfos(std::vector<GenCamInfo> & camInfos) {
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::getCamInfos init first please");
			return -1;
		}
		camInfos.resize(this->cameraNum);
		this->camInfos.resize(this->cameraNum);
		//TODO: move static version to dynamic version
		for (int i = 0; i < this->cameraNum; i++)
		{
			camInfos[i].sn = "000000";
			camInfos[i].width = 4096;
			camInfos[i].height = 3000;
			camInfos[i].fps = 10.0f;
			camInfos[i].autoExposure = Status::on;
			this->getBayerPattern(i, camInfos[i].bayerPattern);
			camInfos[i].redGain = 1.0f;
			camInfos[i].greenGain = 1.0f;
			camInfos[i].blueGain = 1.0f;
			camInfos[i].isWBRaw = true;
			this->camInfos[i] = camInfos[i];
		}
		return 0;
	}

	/**
	@brief start capture images
	@return int
	*/
	int GenCameraNETVIR::startCapture() {
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::startCapture init first please");
			return -1;
		}
		if (serverVec_.size() == NULL ) {
			std::ostringstream strTmp("");
			strTmp << "CameraServer_" << 0 << " is not connected!\nPlease check connection!";
			SysUtil::warningOutput(strTmp.str());
		}
		else
		{
			for (int i = 0;;)
			{
				if (serverVec_[0].connectedFlag_ == true)//TODO: should check every server!
				{
					SysUtil::infoOutput("Connected!");
					break;
				}
				else if (i > wait_time_local * 10)
				{
					SysUtil::warningOutput("GenCameraNETVIR::startCapture connection wait too long, check your network");
					std::ostringstream strTmp("");
					strTmp << "CameraServer_" << 0 << " is not connected!\nPlease check connection!";
					SysUtil::warningOutput(strTmp.str());
					return -1;
				}
				else
				{
					i += 5;
					SysUtil::sleep(5);
				}

			}
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				cameraControlMessage_.requestorId_ = id_;
				cameraControlMessage_.boxIndex_ = 0;
				cameraControlMessage_.cameraIndex_ = 0;
				cameraControlMessage_.operateAllFlag_ = false;
				cameraControlMessage_.cameraAmount_ = serverVec_[serverIndex].boxVec_[0].cameraAmount_;
				SysUtil::infoOutput(("[Camera]Opening server" + QString::number(serverIndex, 10) + "'s camera...").toStdString());
				cameraControlMessage_.serverIndex_ = serverIndex;
				cameraControlMessage_.command_ = Communication_Camera_Open_Camera;//Open Camera
				cameraControlMessage_.status_ = Communication_Camera_Open_Camera_Invalid;
				cameraControlMessage_.openCameraOperationIndex_ = -1; //-1代表修改所有参数
				emit StartOperation(cameraControlMessage_, serverVec_);
			}
			//isCapture moved to OperationFinished
			//this->isCapture = true;
			for (int i = 0;;)
			{
				if (this->isCapture == true)
					break;
				else if (i > wait_time_local * 10)
				{
					SysUtil::warningOutput("GenCameraNETVIR::startCapture wait too long, check your network");
					return -1;
				}
				else
				{
					i += 5;
					SysUtil::sleep(5);
				}

			}

			//TODO: removed from here
			this->getCamInfos(camInfos);
		}
		return 0;
	}

	/**
	@brief stop capture images
	@return int
	*/
	int GenCameraNETVIR::stopCapture() {
		if (serverVec_.size() == NULL || !serverVec_[0].connectedFlag_) {//TODO: should check every server!
			std::ostringstream strTmp("");
			strTmp << "CameraServer_" << 0 << " is not connected!\nPlease check connection!";
			SysUtil::warningOutput(strTmp.str());
		}
		else if(this->isCapture)
		{
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				cameraControlMessage_.requestorId_ = id_;
				cameraControlMessage_.boxIndex_ = 0;
				cameraControlMessage_.cameraIndex_ = 0;
				cameraControlMessage_.operateAllFlag_ = false;
				cameraControlMessage_.cameraAmount_ = serverVec_[serverIndex].boxVec_[0].cameraAmount_;
				SysUtil::infoOutput(("[Camera]Closing server" + QString::number(serverIndex, 10) + "'s camera...").toStdString());
				cameraControlMessage_.serverIndex_ = serverIndex;
				cameraControlMessage_.command_ = Communication_Camera_Close_Camera;//Open Camera
				cameraControlMessage_.status_ = Communication_Camera_Close_Camera_Invalid;
				cameraControlMessage_.openCameraOperationIndex_ = -1; //-1代表修改所有参数
				emit StartOperation(cameraControlMessage_, serverVec_);
			}
			for (int i = 0;;)
			{
				if (this->isCapture == false)
					break;
				else if (i > wait_time_local)
				{
					SysUtil::warningOutput("GenCameraNETVIR::stopCapture wait too long, check your network");
					break;
				}
				else
				{
					i += 5;
					SysUtil::sleep(5);
				}

			}
		}
		return 0;
	}

	/**
	@brief release camera
	@return int
	*/
	int GenCameraNETVIR::release() {
		if (this->isCapture == true)
			this->stopCapture();
		return 0;
	}

	/***********************************************************/
	/*                  camera setting functions               */
	/***********************************************************/
	/**
	@brief set frame rate
	@param float fps: input fps
	@return int
	*/
	int GenCameraNETVIR::setFPS(int camInd, float fps, float exposureUpperLimitRatio) {
		SysUtil::warningOutput("GenCameraNETVIR::setFPS Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto white balance
	@param int ind: index of camera (-1 means all the cameras)
	@return int
	*/
	int GenCameraNETVIR::setAutoWhiteBalance(int camInd) {
		SysUtil::warningOutput("GenCameraNETVIR::setAutoWhiteBalance Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto white balance
	@param int ind: index of camera (-1 means all the cameras)
	@param float redGain: red gain of the white balance
	@param float greenGain: green gain of the white balance
	@param float blueGain: blue gain of the white balance
	@return int
	*/
	int GenCameraNETVIR::setWhiteBalance(int camInd, float redGain,
		float greenGain, float blueGain) {
		SysUtil::warningOutput("GenCameraNETVIR::setWhiteBalance Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto exposure
	@param int ind: index of camera (-1 means all the cameras)
	@param Status autoExposure: if use auto exposure
	@return int
	*/
	int GenCameraNETVIR::setAutoExposure(int camInd, Status autoExposure) {
		SysUtil::warningOutput("GenCameraNETVIR::setAutoExposure Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto exposure level
	for pointgrey camera auto exposure level is adjusted by EV
	@param int ind: index of camera (-1 means all the cameras)
	@param float level: auto exposure level, average intensity of output
	signal AEAG should achieve
	@return int
	*/
	int GenCameraNETVIR::setAutoExposureLevel(int camInd, float level) {
		SysUtil::warningOutput("GenCameraNETVIR::setAutoExposureLevel Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set auto exposure compensation (only support PointGrey cameras)
	@param int ind: index of camera (-1 means all the cameras)
	@param Status status: if use auto EV value
	@param float relativeEV: only valid when the second argument is off.
	The reason why use relative EV value here is to directly set a absolute
	value is difficult
	@return int
	*/
	int GenCameraNETVIR::setAutoExposureCompensation(int camInd,
		Status status, float relativeEV) {
		SysUtil::warningOutput("GenCameraNETVIR::setAutoExposureCompensation Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set exposure time
	@param int ind: index of camera (-1 means all the cameras)
	@param int time: exposure time (in microseconds)
	@return int
	*/
	int GenCameraNETVIR::setExposure(int camInd, int time) {
		SysUtil::warningOutput("GenCameraNETVIR::setExposure Function unused");
		return -1;
		return 0;
	}

	/**
	@brief set/get bayer pattern
	@param int camInd: input camera index
	@param GenCamBayerPattern & bayerPattern: output bayer pattern
	@return int
	*/
	int GenCameraNETVIR::getBayerPattern(int camInd, GenCamBayerPattern & bayerPattern) {
		//TODO:To dynamic
		SysUtil::warningOutput("GenCameraNETVIR::getBayerPattern TODO!");
		bayerPattern = GenCamBayerPattern::BayerRGGB;
		return 0;
	}

	/*************************************************************/
	/*                     capturing function                    */
	/*************************************************************/

	/**
	@brief set capturing mode
	@param GenCamCaptureMode captureMode: capture mode
	@param int size: buffer size
	@return int
	*/
	int GenCameraNETVIR::setCaptureMode(GenCamCaptureMode captureMode,
		int bufferSize)
	{
		this->getCamInfos(camInfos);
		// init capture buffer
		this->captureMode = captureMode;
		this->bufferSize = bufferSize;
		if (captureMode == cam::GenCamCaptureMode::Continous ||
			captureMode == cam::GenCamCaptureMode::ContinousTrigger) {
			// create buffer for raw buffer type
			if (this->bufferType == GenCamBufferType::Raw) {
				SysUtil::warningOutput("GenCameraNETVIR::setCaptureMode Raw data is not implemented yet !");
				return -1;
			}
			// create buffer for JPEG buffer type
			else if (this->bufferType == GenCamBufferType::JPEG) {
				// resize vector
				this->bufferImgs.resize(bufferSize);
				for (size_t i = 0; i < bufferSize; i++) {
					this->bufferImgs[i].resize(this->cameraNum);
				}
				// pre-malloc jpeg data
				for (size_t i = 0; i < this->cameraNum; i++) {
					// pre-calculate compressed jpeg data size
					size_t maxLength = static_cast<size_t>(camInfos[i].width * camInfos[i].height * sizeRatio);
					for (size_t j = 0; j < bufferSize; j++) {
						this->bufferImgs[j][i].data = new char[maxLength];
						this->bufferImgs[j][i].maxLength = maxLength;
						this->bufferImgs[j][i].type = this->bufferType;
					}
				}
			}
		}
		else if (captureMode == cam::GenCamCaptureMode::Single ||
			captureMode == cam::GenCamCaptureMode::SingleTrigger) {
			SysUtil::warningOutput("GenCameraNETVIR::setCaptureMode Single mode is not implemented yet !");
			return -1;
		}
		return 0;
	}

	/**
	@brief wait for recording threads to finish
	@return int
	*/
	int GenCameraNETVIR::waitForRecordFinish()
	{
		// check capturing purpose 
		if (this->camPurpose != GenCamCapturePurpose::Recording) {
			SysUtil::warningOutput("This function is only valid in recording mode");
			return -1;
		}
		// check thread status
		if (this->isCaptureThreadRunning != true) {
			SysUtil::errorOutput("Capturing thread is not started !");
			return -1;
		}
		ths.join();
		SysUtil::infoOutput("GenCameraNETVIR::waitForRecordFinish Capturing thread exit successfully !");
		isCaptureThreadRunning = false;
		return 0;
	}

	/**
	@brief start capturing threads
	capturing threads captures images from cameras, and buffer to
	bufferImgs vector, if buffer type is jpeg, this function will start
	a thread to compress captured images into buffer vector
	@return int
	*/
	int GenCameraNETVIR::startCaptureThreads()
	{
		if (isInit == false)
		{
			SysUtil::errorOutput("GenCameraNETVIR::startCaptureThreads without init!");
			return -1;
		}
		if (captureMode == cam::GenCamCaptureMode::Continous ||
			captureMode == cam::GenCamCaptureMode::ContinousTrigger) {
			thBufferInds.resize(this->cameraNum);

			for (size_t i = 0; i < this->cameraNum; i++) {
				thBufferInds[i] = 0;
			}
			// start threads based on buffer type
			if (this->bufferType == GenCamBufferType::Raw) {
				SysUtil::warningOutput("GenCameraNETVIR::startCaptureThreads Raw data not supported yet");
				return -1;
			}
			else if (this->bufferType == GenCamBufferType::JPEG) {
				// start capturing threads
				ths = std::thread(&GenCameraNETVIR::capture_thread_JPEG_, this);
				isCaptureThreadRunning = true;
			}
			else if (this->bufferType == GenCamBufferType::RGB24 ||
				this->bufferType == GenCamBufferType::Raw16) {
				SysUtil::warningOutput("GenCameraNETVIR::startCaptureThreads RGB24 and Raw16 data not supported yet");
				return -1;
			}
		}
		else {
			SysUtil::warningOutput("GenCameraNETVIR::startCaptureThreads This function is only valid when capture mode is " \
				"Continous or ContinousTrigger !");
			return -1;
		}
		return 0;
	}

	/**
	@brief stop capturing threads
	@return int
	*/
	int GenCameraNETVIR::stopCaptureThreads()
	{
		thexit = 1;
		// make sure all the threads have exited
		if (this->camPurpose == cam::GenCamCapturePurpose::Streaming) {
			if (isCaptureThreadRunning == true) {
				for (size_t i = 0; i < this->cameraNum; i++) {
					ths.join();
					SysUtil::infoOutput("GenCameraNETVIR::stopCaptureThreads Capturing thread exit successfully!");
				}
				isCaptureThreadRunning = false;
			}
		}
		SysUtil::infoOutput("GenCameraNETVIR::stopCaptureThreads camera driver released successfully!");
		return 0;
	}
	/**
	@brief Load Config file finished in child thread
	@param QString _filePath: File name
	@param bool _flag: success or not
	@param std::vector<CameraServerUnitTypeDef> & _serverVec: ServerVectorList
	*/
	void GenCameraNETVIR::LoadConfigFileFinished(QString _filePath, bool _flag, std::vector<CameraServerUnitTypeDef>& _serverVec)
	{
		if (_flag == true)
		{
			config_file_status = 1;
			serverVec_ = _serverVec;
		}
		else
			config_file_status = -1;
	}
}