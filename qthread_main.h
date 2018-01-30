/**
@brief Qthread Test
@author zhu-ty
@date Dec 29, 2017
*/

#ifndef __QTHREAD_MAIN_H__
#define __QTHREAD_MAIN_H__

// include std
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <queue>
//#include <thread>
#include <memory>

#include <QWidget>
#include <QApplication>
#include <QThread>

// opencv
//#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
// cuda
//#ifdef _WIN32
//#include <windows.h>
//#endif
//#include <cuda.h>
//#include <cuda_runtime.h>
//#include <device_launch_parameters.h>

#include "GenCameraDriver.h"

int preview(int argc, char* argv[]);

int record(int argc, char* argv[]);

class MyThread : public QThread
{
	Q_OBJECT
public:
	MyThread(QObject* parent = nullptr) {}
protected:
	void run() override;
};

#endif