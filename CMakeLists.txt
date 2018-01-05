# CMake entry point.
# Note that this CMakeLists.txt in fully independent from the one in the parent directory
# (since that's the goal of this tutorial !)

# For an introduction to CMake, see
# http://www.cmake.org/cmake/help/cmake_tutorial.html (at least steps 1 and 2)

cmake_minimum_required (VERSION 3.1)
set(name "GigaClient")
project ( ${name} )

#c++ version
set (CMAKE_CXX_STANDARD 11)

#OpenCV 
if ( WIN32 )
	set(OpenCV_DIR "E:/libs/OpenCV/x64/vc14/lib")
	set(OpenCV_CONFIG_PATH "E:/libs/OpenCV")
	#set(WIN_HEADER_PATH "${PROJECT_SOURCE_DIR}/winheadfile")
    #include_directories(${WIN_HEADER_PATH})
endif()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


#cuda
# find_package(CUDA)
# set(CUDA_NVCC_FLAGS
#     ${CUDA_NVCC_FLAGS}
# #    -O3 
# 	-gencode=arch=compute_61,code=\"sm_61,compute_61\"
#     )


#Qt5
if ( WIN32 )
	set(Qt5_DIR "E:/libs/Qt/5.6/msvc2015_64/lib/cmake/Qt5")
	#set(Qt5Widgets_DIR "E:/libs/Qt5.6.0/lib/cmake/Qt5Widgets")
endif()
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Find the QtWidgets library
find_package(Qt5 REQUIRED Widgets Network)
add_definitions(${Qt5_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${Qt5_EXECUTABLE_COMPILE_FLAGS}")


set(MY_HEADER_FILES
	# # cuda
	# cuda/CameraUtilKernel.h
	# cuda/NPPJpegCoderKernel.h
	# 
	base.h
	Camera.h
	CameraHeader.h
	# NPPJpegCoder.h
	# helper_cuda.h
	helper_string.h
)

set(MY_SOURCE_FILES 
	base.cpp
	Camera.cpp
	#NPPJpegCoder.cpp
)



include_directories(
	${PROJECT_SOURCE_DIR}		
#	${PROJECT_SOURCE_DIR}/cuda

	#Qt5
	${Qt5Widgets_INCLUDES}
	${Qt5Network_INCLUDES}
)

# cuda_add_executable(${name}
add_executable(${name}
	${MY_HEADER_FILES}
	${MY_SOURCE_FILES}
	main.cpp
	)

target_link_libraries(${name}
	${OpenCV_LIBS}
	# ${CUDA_LIBRARY} 	
	# ${CUDA_npp_LIBRARY} 
	# ${CUDA_nppc_LIBRARY} 
	# ${CUDA_nppi_LIBRARY}
	# ${CUDA_npps_LIBRARY}

	#Qt5
	Qt5::Widgets
	Qt5::Network
	)