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
		// TODO : Can't set fps for each camera right now, now use: the fastest camera's fps
		int choose = 0;
		for (int i = 0; i < camInfos.size(); i++)
			if (camInfos[i].fps > camInfos[choose].fps)
				choose = i;
		double time = 1000.0 / static_cast<double>(camInfos[choose].fps);

		//used for frame test
		double currentTime = 0;
		double lastTime = 0;
		int nbFrames = 0;


		for (;;) {

			currentTime = static_cast<double>(clock()) / CLOCKS_PER_SEC;
			nbFrames++;
			if (currentTime - lastTime >= 3.0)
			{
				// If last prinf() was more than 1 sec ago, printf and reset timer
				// printf("Main Thread: %f ms/frame\n", 1000.0 / double(nbFrames));
				printf("Capture  Thread: %.2f frame/s\n", double(nbFrames) / (currentTime - lastTime));
				nbFrames = 0;
				lastTime = currentTime;
			}



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
				CameraControlMessage cameraControlMessage__;
				cameraControlMessage__.serverIndex_ = serverIndex;
				int boxIndex = 0;
				cameraControlMessage__.boxIndex_ = 0;
				cameraControlMessage__.command_ = Communication_Camera_Get_Image;//Get Image
				cameraControlMessage__.status_ = Communication_Camera_Get_Image_Invalid;
				cameraControlMessage__.imageType_ = 2;
				cameraControlMessage__.imageResizedFactor_ = 0;
				cameraControlMessage__.cameraIndex_ = 0;
				cameraControlMessage__.cameraAmount_ = serverVec_[serverIndex].boxVec_[0].cameraAmount_;
				cameraControlMessage__.images_jpeg_raw.clear();
				cameraControlMessage__.images_jpeg_ratio.clear();
				cameraControlMessage__.images_jpeg_len.clear();
				for (int cameraIndex = 0; cameraIndex < cameraControlMessage__.cameraAmount_; cameraIndex++)
				{
					//if (cameraIndex >= 9)
					//{
					//	SysUtil::errorOutput("cameraIndex >= 9, now is:");
					//	std::cout << cameraIndex << std::endl;
					//	SysUtil::errorOutput("Control Message:");
					//	std::cout << cameraControlMessage__.cameraAmount_ << endl;
					//	SysUtil::errorOutput("ServerIndex:");
					//	std::cout << serverIndex << endl;
					//	SysUtil::errorOutput("cameraIndex:");
					//	std::cout << cameraIndex << endl;
					//	print_server_vec();
					//}
						
					cameraControlMessage__.images_jpeg_raw.push_back(
						bufferImgs[thBufferInds[camInd]][camInd].data);
					cameraControlMessage__.images_jpeg_ratio.push_back(
						(int *)&bufferImgs[thBufferInds[camInd]][camInd].ratio);
					cameraControlMessage__.images_jpeg_len.push_back(
						&bufferImgs[thBufferInds[camInd]][camInd].length);
					//cameraControlMessage__.imageResizedFactor_ |= ((this->imgRatios[camInd] ))
					cameraControlMessage__.imageResizedFactor_ = set_resize_factor(cameraControlMessage__.imageResizedFactor_, this->imgRatios[camInd], cameraIndex);
					camInd++;
				}
				//SysUtil::infoOutput(cv::format("Set Factor:0x%llx", cameraControlMessage__.imageResizedFactor_));
				server_receiving_flag[serverIndex] = 1;

				emit StartOperation(cameraControlMessage__, serverVec_);
				
			}
			int receive_flag_ret = wait_for_receive();
			// end time
			end_time = clock();
			float waitTime = time - static_cast<double>(end_time - begin_time) / CLOCKS_PER_SEC * 1000;

			if (receive_flag_ret == 0)
			{
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
			}
			if (waitTime > 0) {
				SysUtil::sleep(waitTime);
			}
		}
		SysUtil::infoOutput("GenCameraNETVIR::capture_thread_JPEG_:: Capturing thread for camera all finish, exit successfully !");
	}

	int GenCameraNETVIR::wait_for_receive(float times)
	{
		int ret = 0;
		//for (int j = 0; j < serverVec_.size(); j++)
		//{
		//	if (server_receiving_flag[j] == 0)
		//	{
		//		return -1;
		//	}
		//}
		for (int i = 0;;)
		{
			bool breakflag = true;
			for (int j = 0; j < serverVec_.size(); j++)
			{
				if (server_receiving_flag[j] != -1)
				{
					breakflag = false;
				}
			}
			if (breakflag == true)
				break;
			else if (i > wait_time_local * times)
			{
				SysUtil::warningOutput("GenCameraNETVIR::wait_for_receive wait too long, check your network");
				ret = -2;
				break;
			}
			else
			{
				i += 5;
				SysUtil::sleep(5);
			}
		}
		//for (int j = 0; j < serverVec_.size(); j++)
		//{
		//	server_receiving_flag[j] = 0;
		//}
		return ret;
	}

	void GenCameraNETVIR::print_server_vec()
	{
		for (int i = 0; i < serverVec_.size(); i++)
		{
			SysUtil::infoOutput("Server Vec Output::");
			std::cout << " id: " << serverVec_[i].id_ << " ip: " << serverVec_[i].ip_ << " port: " << serverVec_[i].port_ << " boxamount: " << serverVec_[i].boxAmount_ << endl;
			for (int j = 0; j < serverVec_[i].boxVec_.size(); j++)
			{
				std::cout << " box id: " << serverVec_[i].boxVec_[j].id_ << " box camera amount: " << serverVec_[i].boxVec_[j].cameraAmount_ << endl;
			}
		}
	}

	int64_t GenCameraNETVIR::set_resize_factor(int64_t factor, cam::GenCamImgRatio ratio, int cam_idx)
	{
		if (cam_idx > 15)
			SysUtil::errorOutput("GenCameraNETVIR::set_resize_factor only support up to 16 cameras");
		//骚操作 每4个bit代表一个相机的resize factor
		factor |= ((int64_t)ratio) << (cam_idx * 4);
		return factor;
	}

	int GenCameraNETVIR::get_sub_index(int global_index, int & server_index, int & camera_index)
	{
		server_index = -1;
		camera_index = -1;
		if (global_index >= 0)
		{
			for (int i = 0; i < serverVec_.size(); i++)
			{
				if (global_index >= serverVec_[i].boxVec_[0].cameraAmount_)
				{
					global_index = global_index - serverVec_[i].boxVec_[0].cameraAmount_;
				}
				else
				{
					server_index = i;
					camera_index = global_index;
					break;
				}
			}
		}
		return 0;
	}

	int GenCameraNETVIR::build_control_message(int server_index, CameraControlMessage & message)
	{
		message.requestorId_ = id_;
		message.boxIndex_ = 0;
		message.cameraIndex_ = 0;
		message.operateAllFlag_ = false;
		message.cameraAmount_ = serverVec_[server_index].boxVec_[0].cameraAmount_;
		message.serverIndex_ = server_index;
		message.command_ = Communication_Camera_Open_Camera;
		message.status_ = Communication_Camera_Open_Camera_Invalid;
		message.openCameraOperationIndex_ = -1;
		return 0;
	}

	// constructor
	GenCameraNETVIR::GenCameraNETVIR() {
		this->camModel = CameraModel::Network;
		communication_camera = new CameraCommunication();
		communication_camera->moveToThread(&communication_thread);
		//communication_camera->StartStopTimer(true);
		//Qt Connect
		qRegisterMetaType<CameraControlMessage>("CameraControlMessage");   //�Զ����ź���ۺ�������
		qRegisterMetaType<std::vector<CameraServerUnitTypeDef> >("std::vector<CameraServerUnitTypeDef> &");
		QObject::connect(this,
			SIGNAL(StartOperation(CameraControlMessage, std::vector<CameraServerUnitTypeDef> &)),
			communication_camera,
			SLOT(StartOperation(CameraControlMessage, std::vector<CameraServerUnitTypeDef> &)),
			Qt::QueuedConnection);
		QObject::connect(communication_camera, SIGNAL(OperationFinished(CameraControlMessage)),
			this, SLOT(OperationFinished(CameraControlMessage)), Qt::DirectConnection);

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
	void GenCameraNETVIR::OperationFinished(CameraControlMessage _cameraControlMessage)
	{
		if (_cameraControlMessage.requestorId_ == id_ || _cameraControlMessage.requestorId_ == 0) {
			//cameraControlMessage_ = _cameraControlMessage;
			Communication_Camera_Command command = _cameraControlMessage.command_;
			Communication_Camera_Status status = _cameraControlMessage.status_;
			int serverIndex = _cameraControlMessage.serverIndex_;
			int boxIndex = _cameraControlMessage.boxIndex_;
			int cameraIndex = _cameraControlMessage.cameraIndex_;
			int cameraAmount = _cameraControlMessage.cameraAmount_;
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
					//data_receive[cameraControlMessage_.serverIndex_] = cameraControlMessage_.gendata_;
					memcpy(&data_receive[_cameraControlMessage.serverIndex_], &_cameraControlMessage.gendata_, sizeof(GenCameraControlData));
					server_receiving_flag[serverIndex] = -1;
					switch (status) {
					case Communication_Camera_Open_Camera_Ok: {
						std::string atmp = ("[Server" + QString::number(serverIndex, 10) + "] reply (OpenCameraCommand) ").toStdString();
						atmp = atmp + _cameraControlMessage.genfunc_ + " OK.";
						SysUtil::infoOutput(atmp);
						//this->isCapture = true; //TODO: not here!!!
						break;
					}
					case Communication_Camera_Open_Camera_Invalid: {
						strTmp << "Failed to open Server_" << serverIndex << "-Cam_" << cameraIndex << "-Id_" << cameraId 
							<< "! With function"<< _cameraControlMessage.genfunc_;
						SysUtil::warningOutput(("[Warning]" + QString::fromStdString(strTmp.str())).toStdString());
						//this->isCapture = false;
						break;
					}
					case Communication_Camera_Action_Overtime: {
						strTmp << "Overtime to open Server_" << serverIndex << "-Cam_" << cameraIndex << "-Id_" << cameraId 
							<< "! With function" << _cameraControlMessage.genfunc_;
						SysUtil::warningOutput(("[Warning]" + QString::fromStdString(strTmp.str())).toStdString());
						//this->isCapture = false;
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
					server_receiving_flag[serverIndex] = -1;
					switch (status) {
					case Communication_Camera_Get_Image_Ok: {
						int32_t imagesize = _cameraControlMessage.imageSize_;
						if (isVerbose)
						{
							SysUtil::infoOutput(("server" + QString::number(serverIndex, 10) +
								"getImage OK,imageSize=" + QString::number(imagesize, 10)).toStdString());
						}
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
						//SysUtil::infoOutput(("[Server" + QString::number(serverIndex, 10) + "]Colse Camera").toStdString());
						std::string atmp = ("[Server" + QString::number(serverIndex, 10) + "] reply (ColseCameraCommand) ").toStdString();
						atmp = atmp + "close camera" + " OK.";
						SysUtil::infoOutput(atmp);
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
		for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
		{
			CameraControlMessage __message;
			build_control_message(serverIndex, __message);
			SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) makeSetEffective %d to [Server%d]", 
				k, serverIndex));
			__message.genfunc_ = "makeSetEffective";
			__message.gendata_.param_func.param_int[0] = k;
			server_receiving_flag[serverIndex] = 1;
			emit StartOperation(__message, serverVec_);
		}
		wait_for_receive();
		for (int i = 0; i < serverVec_.size(); i++)
			if (data_receive[i].void_func.return_val != 0)
				return data_receive[i].void_func.return_val;
		return 0;
	}

	int GenCameraNETVIR::genSettingInterface(int value1, std::string value2)
	{
		if (value1 == 1) //means the config file path
		{
			communication_camera->configFileName_ = value2;
			return 0;
		}
		else if (value1 == 2) //means to upload the image ratio to the server
		{
			int camFullIdx = 0;
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				CameraControlMessage __message;
				build_control_message(serverIndex, __message);
				SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setImageRatios to [Server%d]", serverIndex));
				__message.genfunc_ = "setImageRatios";
				for (int i = 0; i < serverVec_[serverIndex].boxVec_[0].cameraAmount_; i++)
				{
					__message.gendata_.param_func.param_int[i] = (int)this->imgRatios[camFullIdx];
					camFullIdx++;
				}
				server_receiving_flag[serverIndex] = 1;
				emit StartOperation(__message, serverVec_);
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		else
		{
			SysUtil::warningOutput("GenCameraNETVIR::genSettingInterface unknown int type");
			SysUtil::warningOutput("Use value1 = 1 to set config file path");
			SysUtil::warningOutput("Use value1 = 2 to upload the imgRatio");
		}
		return 0;
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
		this->data_receive.resize(serverVec_.size());
		this->cameraNum = 0;
		for (int i = 0; i < serverVec_.size(); i++)
		{
			this->cameraNum += serverVec_[i].boxVec_[0].cameraAmount_;
			server_receiving_flag.push_back(0);
		}
		this->imgRatios.resize(this->cameraNum);
		for (int i = 0; i < this->cameraNum; i++)
			this->imgRatios[i] = cam::GenCamImgRatio::Full;
		this->isInit = true;
		for (int i = 0;;)
		{
			if (serverVec_[0].connectedFlag_ == true)//TODO: should check every server!
			{
				SysUtil::infoOutput("Connected!");
				break;
			}
			else if (i > wait_time_local * 10)
			{
				SysUtil::warningOutput("GenCameraNETVIR::init connection wait too long, check your network");
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
		CameraCommunicationThread::socketReadWaitForMs_ = 5000 * MAX_CAMERA_NUM;
		for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
		{
			CameraControlMessage __message;
			build_control_message(serverIndex, __message);
			SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) init to [Server%d]", serverIndex));
			__message.genfunc_ = "init";
			server_receiving_flag[serverIndex] = 1;
			emit StartOperation(__message, serverVec_);
		}
		wait_for_receive(MAX_CAMERA_NUM);
		for (int i = 0; i < serverVec_.size(); i++)
			if (data_receive[i].void_func.return_val != 0)
				return data_receive[i].void_func.return_val;
		CameraCommunicationThread::socketReadWaitForMs_ = 5000;
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

		for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
		{
			CameraControlMessage __message;
			build_control_message(serverIndex, __message);
			SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) getCamInfos to [Server%d]", serverIndex));
			__message.genfunc_ = "getCamInfos";
			server_receiving_flag[serverIndex] = 1;
			emit StartOperation(__message, serverVec_);
		}
		wait_for_receive();
		int camIndex = 0;
		for (int i = 0; i < serverVec_.size(); i++)
			for (int j = 0; j < serverVec_[i].boxVec_[0].cameraAmount_; j++)
			{
				std::string tmp_str(data_receive[i].caminfo_func.camInfos[j].sn);
				camInfos[camIndex].sn = tmp_str;
				camInfos[camIndex].width = data_receive[i].caminfo_func.camInfos[j].width;
				camInfos[camIndex].height = data_receive[i].caminfo_func.camInfos[j].height;
				camInfos[camIndex].fps = data_receive[i].caminfo_func.camInfos[j].fps;
				camInfos[camIndex].autoExposure = (Status)data_receive[i].caminfo_func.camInfos[j].autoExposure;
				camInfos[camIndex].bayerPattern = (GenCamBayerPattern)data_receive[i].caminfo_func.camInfos[j].bayerPattern;
				camInfos[camIndex].redGain = data_receive[i].caminfo_func.camInfos[j].redGain;
				camInfos[camIndex].greenGain = data_receive[i].caminfo_func.camInfos[j].greenGain;
				camInfos[camIndex].blueGain = data_receive[i].caminfo_func.camInfos[j].blueGain;
				camInfos[camIndex].isWBRaw = data_receive[i].caminfo_func.camInfos[j].isWBRaw;
				this->camInfos[camIndex] = camInfos[camIndex];
				camIndex++;
			}
		for (int i = 0; i < serverVec_.size(); i++)
			if (data_receive[i].void_func.return_val != 0)
				return data_receive[i].void_func.return_val;
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
		if (serverVec_.size() == 0 ) {
			std::ostringstream strTmp("");
			strTmp << "CameraServer_" << 0 << " is not connected!\nPlease check connection!";
			SysUtil::warningOutput(strTmp.str());
		}
		else
		{
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				CameraControlMessage __message;
				build_control_message(serverIndex, __message);
				SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) startCapture to [Server%d]", serverIndex));
				__message.genfunc_ = "startCapture";
				server_receiving_flag[serverIndex] = 1;
				emit StartOperation(__message, serverVec_);
			}
			wait_for_receive(5);
			//return 
			//this->getCamInfos(camInfos);
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			this->isCapture = true;
			return 0;
		}
		return -1;
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
				CameraControlMessage __message;
				__message.requestorId_ = id_;
				__message.boxIndex_ = 0;
				__message.cameraIndex_ = 0;
				__message.operateAllFlag_ = false;
				__message.cameraAmount_ = serverVec_[serverIndex].boxVec_[0].cameraAmount_;

				//std::string atmp = ("[Client ] send  Server" + QString::number(serverIndex, 10) + "(CloseCameraCommand) close camera").toStdString();
				std::string atmp = ((QString) "[Client ] send  " + "(CloseCameraCommand) close camera" +
					" to" + " [Server" + QString::number(serverIndex, 10) + "]").toStdString();
				SysUtil::infoOutput(atmp);

				//SysUtil::infoOutput(("[Camera]Closing server" + QString::number(serverIndex, 10) + "'s camera...").toStdString());
				__message.serverIndex_ = serverIndex;
				__message.command_ = Communication_Camera_Close_Camera;//Open Camera
				__message.status_ = Communication_Camera_Close_Camera_Invalid;
				__message.openCameraOperationIndex_ = -1; //-1�����޸����в���
				emit StartOperation(__message, serverVec_);
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
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setFPS init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setFPS %d %f %f to [Server%d]", camera_idx_searched, fps, exposureUpperLimitRatio, serverIndex));
					__message.genfunc_ = "setFPS";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_float[0] = fps;
					__message.gendata_.param_func.param_float[1] = exposureUpperLimitRatio;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
	}

	/**
	@brief set auto white balance
	@param int ind: index of camera (-1 means all the cameras)
	@return int
	*/
	int GenCameraNETVIR::setAutoWhiteBalance(int camInd) {
		//SysUtil::warningOutput("GenCameraNETVIR::setAutoWhiteBalance Function unused");
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setAutoWhiteBalance init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setAutoWhiteBalance %d to [Server%d]", camera_idx_searched, serverIndex));
					__message.genfunc_ = "setAutoWhiteBalance";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
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
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setWhiteBalance init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setWhiteBalance %d %f %f %f to [Server%d]",
						camera_idx_searched, redGain, greenGain, blueGain, serverIndex));
					__message.genfunc_ = "setWhiteBalance";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_float[0] = redGain;
					__message.gendata_.param_func.param_float[1] = greenGain;
					__message.gendata_.param_func.param_float[2] = blueGain;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
	}

	/**
	@brief set auto exposure
	@param int ind: index of camera (-1 means all the cameras)
	@param Status autoExposure: if use auto exposure
	@return int
	*/
	int GenCameraNETVIR::setAutoExposure(int camInd, Status autoExposure) {
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setAutoExposure init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setAutoExposure %d %d to [Server%d]",
						camera_idx_searched, (int)autoExposure, serverIndex));
					__message.genfunc_ = "setAutoExposure";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_enum[0] = (int)autoExposure;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
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
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setAutoExposureLevel init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setAutoExposureLevel %d %f to [Server%d]",
						camera_idx_searched, level, serverIndex));
					__message.genfunc_ = "setAutoExposureLevel";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_float[0] = level;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
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
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setAutoExposureCompensation init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setAutoExposureCompensation %d %d %f to [Server%d]",
						camera_idx_searched, (int)status, relativeEV, serverIndex));
					__message.genfunc_ = "setAutoExposureCompensation";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_enum[0] = (int)status;
					__message.gendata_.param_func.param_float[0] = relativeEV;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive(10.0);
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
	}

	/**
	@brief set brightness time
	@param int brightness: input brightness
	+1: brighten, -1: darken, 0: do nothing
	@return int
	*/
	int GenCameraNETVIR::adjustBrightness(int camInd, int brightness) {
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setAutoExposureCompensation init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) adjustBrightness %d %d to [Server%d]",
						camera_idx_searched, brightness, serverIndex));
					__message.genfunc_ = "adjustBrightness";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_int[1] = (int)brightness;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
	}

	/**
	@brief set exposure time
	@param int ind: index of camera (-1 means all the cameras)
	@param int time: exposure time (in microseconds)
	@return int
	*/
	int GenCameraNETVIR::setExposure(int camInd, int time) {
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::setExposure init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setExposure %d %d to [Server%d]",
						camera_idx_searched, time, serverIndex));
					__message.genfunc_ = "setExposure";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_int[1] = time;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
	}

	/**
	@brief set/get bayer pattern
	@param int camInd: input camera index
	@param GenCamBayerPattern & bayerPattern: output bayer pattern
	@return int
	*/
	int GenCameraNETVIR::getBayerPattern(int camInd, GenCamBayerPattern & bayerPattern) {
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::getBayerPattern init first please");
			return -1;
		}
		else
		{
			int server_idx_searched, camera_idx_searched;
			get_sub_index(camInd, server_idx_searched, camera_idx_searched);
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				if (serverIndex == server_idx_searched || server_idx_searched == -1)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) getBayerPattern %d to [Server%d]",
						camera_idx_searched, serverIndex));
					__message.genfunc_ = "getBayerPattern";
					__message.gendata_.param_func.param_int[0] = camera_idx_searched;
					__message.gendata_.param_func.param_enum[0] = (int)bayerPattern;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
			}
			wait_for_receive();
			bayerPattern = (GenCamBayerPattern)data_receive[(server_idx_searched == -1) ? 0 : server_idx_searched].param_func.param_enum[0];
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
			return 0;
		}
		return -1;
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
		if (this->isInit == false)
		{
			SysUtil::warningOutput("GenCameraNETVIR::getBayerPattern init first please");
			return -1;
		}
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
					size_t maxLength;
					if(camInfos[i].sn.substr(0,3) != "RAW")
						maxLength = static_cast<size_t>(camInfos[i].width * camInfos[i].height * this->sizeRatio);
					else 
						maxLength = static_cast<size_t>(camInfos[i].width * camInfos[i].height * 2);
					for (size_t j = 0; j < bufferSize; j++) {
						this->bufferImgs[j][i].data = new char[maxLength];
						this->bufferImgs[j][i].maxLength = maxLength;
						this->bufferImgs[j][i].type = this->bufferType;
					}
				}
				//first send quality!

				for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setJPEGQuality %d %f to [Server%d]",
						this->JPEGQuality, this->sizeRatio, serverIndex));
					__message.genfunc_ = "setJPEGQuality";
					__message.gendata_.param_func.param_int[0] = this->JPEGQuality;
					__message.gendata_.param_func.param_float[0] = this->sizeRatio;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
				wait_for_receive();

				for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
				{
					CameraControlMessage __message;
					build_control_message(serverIndex, __message);
					SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) setCaptureMode %d %d to [Server%d]",
						(int)captureMode, bufferSize, serverIndex));
					__message.genfunc_ = "setCaptureMode";
					__message.gendata_.param_func.param_enum[0] = (int)captureMode;
					__message.gendata_.param_func.param_int[0] = bufferSize;
					server_receiving_flag[serverIndex] = 1;
					emit StartOperation(__message, serverVec_);
				}
				wait_for_receive();
				for (int i = 0; i < serverVec_.size(); i++)
					if (data_receive[i].void_func.return_val != 0)
						return data_receive[i].void_func.return_val;
				return 0;
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
		else
		{
			for (int serverIndex = 0; serverIndex < serverVec_.size(); serverIndex++)
			{
				CameraControlMessage __message;
				build_control_message(serverIndex, __message);
				SysUtil::infoOutput(SysUtil::format("[Client ] send  (OpenCameraCommand) startCaptureThreads to [Server%d]", serverIndex));
				__message.genfunc_ = "startCaptureThreads";
				server_receiving_flag[serverIndex] = 1;
				emit StartOperation(__message, serverVec_);
			}
			wait_for_receive();
			for (int i = 0; i < serverVec_.size(); i++)
				if (data_receive[i].void_func.return_val != 0)
					return data_receive[i].void_func.return_val;
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
				//for (size_t i = 0; i < this->cameraNum; i++) {
					ths.join();
					SysUtil::infoOutput("GenCameraNETVIR::stopCaptureThreads Capturing thread exit successfully!");
				//}
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