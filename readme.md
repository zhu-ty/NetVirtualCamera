# NetVirtualCamera
Solve the limits of cameras connect to a single computer.

Project from [yuanxy92/GenCameraDriver](https://github.com/yuanxy92/GenCameraDriver)
## Libraries
- OpenCV >= 3.0
- Windows Visual Studio 2015 (Linux or other Visual Studio Version should be OK, but untested, C++11 feature is needed for multi-threads feature ), do not use Visual Studio 2017 now!
- Qt >= 5 (Will be removed in the future)
- Cuda 8.0
- OpenGL will be used in the future to decompress the jpeg data and render images.

## Notice
Not fully done yet, project is designed to be a remote camera driver, this shoule be used with zhu-ty/CameraDriver2

There may be other bugs. QAQ

zhu-ty

``` cmake .. -G "Visual Studio 14 2015 Win64"
