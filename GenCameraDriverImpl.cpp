/**
@brief Generic Camera Driver Class (CameraType)
@author zhu-ty
@date Jan 8, 2018
*/

#include "GenCameraDriver.h"
//#include "XIMEA/XIMEACamera.h"
//#include "PointGrey/PointGreyCamera.h"
#include "NetVirtualCamera/NetVirtualCamera.h"

namespace cam {
	/**
	@breif function to init camera array
	@return
	*/
	std::shared_ptr<GenCamera> cam::createCamera(CameraModel camModel) {
		//if (camModel == CameraModel::XIMEA_xiC) {
		//	std::shared_ptr<GenCameraXIMEA> cameraPtr = std::make_shared<GenCameraXIMEA>();
		//	return std::static_pointer_cast<GenCamera>(cameraPtr);
		//}
		//else if (camModel == CameraModel::PointGrey_u3) {
		//	std::shared_ptr<GenCameraPTGREY> cameraPtr = std::make_shared<GenCameraPTGREY>();
		//	return std::static_pointer_cast<GenCamera>(cameraPtr);
		//}
		if (camModel == CameraModel::Network) {
			std::shared_ptr<GenCameraNETVIR> cameraPtr = std::make_shared<GenCameraNETVIR>();
			return std::static_pointer_cast<GenCamera>(cameraPtr);
		}
	}
};