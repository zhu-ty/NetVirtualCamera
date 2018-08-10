/**
@brief Generic Camera Driver Class
Implementation of NetVirtual camera
@author zhu-ty
@date Jan 7, 2018
*/

#ifndef __GENERIC_CAMERA_DRIVER_NETVIR_HPP__
#define __GENERIC_CAMERA_DRIVER_NETVIR_HPP__

#include "GenCameraDriver.h"

// Network Core Part
#include "NetVirtualCamera/Netbase.h"
#include "NetVirtualCamera/NetCameraCore.h"
#include "NetVirtualCamera/NetCameraHeader.h"

#include <memory.h>

namespace cam {


	class GenCameraNETVIR : public QObject, public GenCamera {
		Q_OBJECT
	private:
		// Network threads and classes
		QThread communication_thread;
		CameraCommunication *communication_camera;
		// ServerVector
		std::vector<CameraServerUnitTypeDef> serverVec_;
		// ServerControlMessage ->use local message due to some thread problem
		// CameraControlMessage cameraControlMessage_;
		// Client ID
		int id_ = -1;
		// Local wait time for each operation (ms)
		int wait_time_local = 5000;
		// threads to capture images
		std::thread ths;
		bool isCaptureThreadRunning;
		// variable to exit all the threads (used for streaming mode)
		// thexit == 1, exit thread
		// thexit == 0, keep running
		int thexit;
		// used by wait_for_receive();
		// 0 : unused
		// 1 : waiting
		// -1 : receive done (including bad receive)
		std::vector<int> server_receiving_flag;
		// check if config was loaded properly
		// 0 : loading
		// 1 : success
		// -1 : failed
		int config_file_status = -1;
		// received data from each server
		std::vector<GenCameraControlData> data_receive;
	public:

	private:
		/**
		@brief thread capturing function (jpeg buffer)
		used for continous mode
		thread function to get images from camera
		*/
		void capture_thread_JPEG_();
		/**
		@brief wait for all server respond
		*/
		int wait_for_receive(float times  = 1);

		void print_server_vec();

		int64_t set_resize_factor(int64_t factor, cam::GenCamImgRatio ratio, int cam_idx);

		int get_sub_index(int global_index, int & server_index, int & camera_index);

		int build_control_message(int server_index, CameraControlMessage & message);

	public:
		GenCameraNETVIR();
		~GenCameraNETVIR();

		/***********************************************************/
		/*                   basic camera functions                */
		/***********************************************************/

		/**
		@brief A generic interface for a int value and a string value
		@param int value1: int value
		@param std::string value2: string value
		@return int
		*/
		int genSettingInterface(int value1, std::string value2) override;

		/**
		@brief init camera
		@return int
		*/
		int init() override;

		/**
		@brief start capture images
		@return int
		*/
		int startCapture() override;

		/**
		@brief stop capture images
		@return int
		*/
		int stopCapture() override;

		/**
		@brief release camera
		@return int
		*/
		int release() override;

		/**
		@brief get camera information
		@param std::vector<GenCamInfo> & camInfos: output camera infos
		@return int
		*/
		int getCamInfos(std::vector<GenCamInfo> & camInfos) override;

		/***********************************************************/
		/*                  camera setting functions               */
		/***********************************************************/
		/**
		@brief set frame rate
		@param float fps: input fps
		@param float exposureUpperLimitRatio: exposure upper limit time, make
		exposure upper limit time = 1000000us / fps * 0.8
		@return int
		*/
		int setFPS(int camInd, float fps, float exposureUpperLimitRatio = 0.8) override;

		/**
		@brief set auto white balance
		@param int ind: index of camera (-1 means all the cameras)
		@return int
		*/
		int setAutoWhiteBalance(int camInd) override;

		/**
		@brief set auto white balance
		@param int ind: index of camera (-1 means all the cameras)
		@param float redGain: red gain of the white balance
		@param float greenGain: green gain of the white balance
		@param float blueGain: blue gain of the white balance
		@return int
		*/
		int setWhiteBalance(int camInd, float redGain,
			float greenGain, float blueGain) override;

		/**
		@brief set auto exposure
		@param int ind: index of camera (-1 means all the cameras)
		@param Status autoExposure: if use auto exposure
		@return int
		*/
		int setAutoExposure(int camInd, Status autoExposure) override;

		/**
		@brief set auto exposure level
		@param int ind: index of camera (-1 means all the cameras)
		@param float level: auto exposure level, average intensity of output
		signal AEAG should achieve
		@return int
		*/
		int setAutoExposureLevel(int camInd, float level) override;

		/**
		@brief set auto exposure compensation (only support PointGrey cameras)
		@param int ind: index of camera (-1 means all the cameras)
		@param Status status: if use auto EV value
		@param float relativeEV: only valid when the second argument is off.
		The reason why use relative EV value here is to directly set a absolute
		value is difficult
		@return int
		*/
		int setAutoExposureCompensation(int camInd,
			Status status, float relativeEV) override;

		/**
		@brief set brightness time
		@param int brightness: input brightness
		+1: brighten, -1: darken, 0: do nothing
		@return int
		*/
		int adjustBrightness(int camInd, int brightness) override;

		/**
		@brief set exposure time
		@param int ind: index of camera (-1 means all the cameras)
		@param int time: exposure time (in microseconds)
		@return int
		*/
		int setExposure(int camInd, int time) override;

		/**
		@brief set/get bayer pattern
		@param int camInd: input camera index
		@param GenCamBayerPattern & bayerPattern: output bayer pattern
		@return int
		*/
		int getBayerPattern(int camInd, GenCamBayerPattern & bayerPattern) override;

		/**
		@brief make setting effective
		by capturing some frames
		@param int k: capture image frames (default is 10)
		@return int
		*/
		int makeSetEffective(int k = 10) override;

		/*************************************************************/
		/*                     capturing function                    */
		/*************************************************************/
		/**
		@brief set capturing mode
		@param GenCamCaptureMode captureMode: capture mode
		@param int size: buffer size
		@return int
		*/
		int setCaptureMode(GenCamCaptureMode captureMode,
			int bufferSize);

		/**
		@brief wait for recording threads to finish
		@return int
		*/
		int waitForRecordFinish();

		/**
		@brief start capturing threads
		capturing threads captures images from cameras, and buffer to
		bufferImgs vector, if buffer type is jpeg, this function will start
		a thread to compress captured images into buffer vector
		@return int
		*/
		int startCaptureThreads();

		/**
		@brief stop capturing threads
		@return int
		*/
		int stopCaptureThreads();


		protected:
			/***********************************************************/
			/*                 Qt and network functions                */
			/***********************************************************/
		signals :
			/**
			@brief Network CameraControl Send
			@param CameraControlMessage & _cameraControlMessage: Message used to control the camera
			@param std::vector<CameraServerUnitTypeDef> & _serverVec: ServerVectorList
			*/
			void StartOperation(CameraControlMessage _cameraControlMessage,
				std::vector<CameraServerUnitTypeDef> &_serverVec);
			/**
			@brief Load Config file in child thread
			@param QString _file: File name
			*/
			void LoadConfigFile(QString _file);

		public slots:
			/**
			@brief Network CameraControl Receive (In main thread)
			@param CameraControlMessage & _cameraControlMessage: Server's respond pack
			*/
			void OperationFinished(CameraControlMessage _cameraControlMessage);
			/**
			@brief Load Config file finished in child thread
			@param QString _filePath: File name
			@param bool _flag: success or not
			@param std::vector<CameraServerUnitTypeDef> & _serverVec: ServerVectorList
			*/
			void LoadConfigFileFinished(QString _filePath, bool _flag, std::vector<CameraServerUnitTypeDef> &_serverVec);
	};

};

#endif
