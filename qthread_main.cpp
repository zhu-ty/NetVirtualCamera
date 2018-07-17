#include "qthread_main.h"

int preview(int argc, char* argv[]) {
	// init buffer
	std::vector<cam::Imagedata> imgdatas(2);
	std::vector<cv::Mat> imgs(2);
	// init camera
	std::vector<cam::GenCamInfo> camInfos;
	std::shared_ptr<cam::GenCamera> cameraPtr 
		= cam::createCamera(cam::CameraModel::XIMEA_xiC);
	cameraPtr->init();
	// set camera setting
	cameraPtr->startCapture();
	cameraPtr->setFPS(-1, 20);
	cameraPtr->setAutoExposure(-1, cam::Status::on);
	cameraPtr->setAutoExposureLevel(-1, 40);
	
	cameraPtr->setAutoWhiteBalance(-1);
	cameraPtr->makeSetEffective();
	// set capturing setting
	cameraPtr->setCaptureMode(cam::GenCamCaptureMode::Continous, 20);
	// get camera info
	cameraPtr->getCamInfos(camInfos);
	cameraPtr->startCaptureThreads();
	// capture frames
	for (size_t i = 0; i < 2000; i++) {
		cameraPtr->captureFrame(imgdatas);
		imgs[0] = cv::Mat(camInfos[0].height, camInfos[0].width,
			CV_8U, reinterpret_cast<void*>(imgdatas[0].data));
		imgs[1] = cv::Mat(camInfos[1].height, camInfos[1].width,
			CV_8U, reinterpret_cast<void*>(imgdatas[1].data));
		cv::Mat show1, show2;
		cv::resize(imgs[0], show1, cv::Size(400, 300));
		cv::resize(imgs[1], show2, cv::Size(400, 300));
		//cv::imshow("1", show1);
		//cv::imshow("2", show2);
		//cv::waitKey(1);
	}
	cameraPtr->stopCaptureThreads();
	cameraPtr->release();
	return 0;
}

int record(int argc, char* argv[]) {
	std::vector<cam::GenCamInfo> camInfos;
	//std::shared_ptr<cam::GenCamera> cameraPtr
	//	= cam::createCamera(cam::CameraModel::PointGrey_u3);
	std::shared_ptr<cam::GenCamera> cameraPtr
		= cam::createCamera(cam::CameraModel::Network);
	cameraPtr->genSettingInterface(1, 
		"E:/Project/GigaRenderNetwork/Common/network_camera_driver/ConfigSample/config.new.xml");
	cameraPtr->init();
	cam::SysUtil::sleep(1000);
	// set camera setting
	//cam::SysUtil::sleep(5000);
	cameraPtr->startCapture();
	cameraPtr->setFPS(-1, 12);
	cameraPtr->setAutoExposure(-1, cam::Status::on);
	cameraPtr->setAutoExposureCompensation(-1, cam::Status::on, 0);
	cameraPtr->setAutoWhiteBalance(-1);
	cameraPtr->makeSetEffective();
	// set capturing setting
	cameraPtr->setCamBufferType(cam::GenCamBufferType::JPEG);
	cameraPtr->setJPEGQuality(85, 0.5);
	cameraPtr->getCamInfos(camInfos);
	cam::SysUtil::sleep(1000);
	cameraPtr->setCaptureMode(cam::GenCamCaptureMode::Continous, 200);
	cameraPtr->setCapturePurpose(cam::GenCamCapturePurpose::Recording);
	cameraPtr->setVerbose(true);

	cameraPtr->getCamInfos(camInfos);

	cam::SysUtil::sleep(1000);
	cameraPtr->startCaptureThreads();
	// wait for recoding to finish
	cameraPtr->waitForRecordFinish();
	cameraPtr->saveImages("test");
	//cameraPtr->saveVideos("test");
	cameraPtr->stopCaptureThreads();
	cameraPtr->release();
	return 0;
}

void MyThread::run()
{
	//std::cout << "myThread run() start to execute";
	//std::cout << "     current thread ID:" << QThread::currentThreadId() << '\n';
	record(0, nullptr);
	exec();
}