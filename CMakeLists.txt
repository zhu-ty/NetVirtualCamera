## CMakeLists.txt for Netvirtual Camera Driver
# author: Shane Yuan
# date: Dec 29, 2017

# author: zhu-ty
# date: Jan 7, 2018
#

message(
"
       
           ┌─┐       ┌─┐
        ┌──┘ ┴───────┘ ┴──┐
        │                 │
        │       ───       │
        │  ─┬┘       └┬─  │
        │                 │
        │       ─┴─       │
        │                 │
        └───┐         ┌───┘
            │         │
            │         │
            │         │
            │         └──────────────┐
            │                        │
            │                        ├─┐
            │                        ┌─┘    
            │                        │
            └─┐  ┐  ┌───────┬──┐  ┌──┘         
              │ ─┤ ─┤       │ ─┤ ─┤         
              └──┴──┘       └──┴──┘ 
                  神兽  保佑 
                  代码无BUG! 
  
")

cmake_minimum_required (VERSION 3.10)
set(name "NetVirtualCamera")
project ( ${name} )

# c++ version
set (CMAKE_CXX_STANDARD 11)

# opencv 
if ( WIN32 )
    set(OpenCV_DIR "E:/libs/OpenCV/x64/vc14/lib")
    set(OpenCV_CONFIG_PATH "E:/libs/OpenCV")
    # set(WIN_HEADER_PATH "${PROJECT_SOURCE_DIR}/winheadfile")
    # include_directories(${WIN_HEADER_PATH})
endif()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# cuda
find_package(CUDA)
set(CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS}
#    -O3 
	-gencode=arch=compute_61,code=\"sm_61,compute_61\"
    )


#add_definitions(-D"_WIN32_WINNT 0x0A00") 
add_definitions(-D"ASIO_STANDALONE")
include_directories("E:/libs/asio/include")

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

# #Csharp Socket
# set(CMAKE_CXX_FLAGS "/clr /FU\"C:\\Program Files (x86)\\Reference Assemblies\\Microsoft\\Framework\\.NETFramework\\v4.0\\mscorlib.dll\"")
# if(CMAKE_CXX_FLAGS_DEBUG MATCHES "/RTC1")
# string(REPLACE "/RTC1" " " CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
# endif()
# if(CMAKE_CXX_FLAGS MATCHES "/EHsc")
# string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
# endif()
set(CSHARP_SOCKET_LIBRARY "E:/Projects/NetVirtualCamera/NetVirtualCamera/CSharpSocketCppExport.lib")


# # pointgrey camera sdk
# set(PTGREY_SDK_INCLUDE_DIR "C:/Program Files/Point Grey Research/Spinnaker/include")
# set(PTGREY_SDK_LIBRARY_DEBUG "C:/Program Files/Point Grey Research/Spinnaker/lib64/vs2015/Spinnakerd_v140.lib")
# set(PTGREY_SDK_LIBRARY_RELEASE "C:/Program Files/Point Grey Research/Spinnaker/lib64/vs2015/Spinnaker_v140.lib")
# include_directories(${PTGREY_SDK_INCLUDE_DIR})

# # XIMEA camera
# set(XIMEA_SDK_INCLUDE_DIR "C:/XIMEA/API")
# set(XIMEA_SDK_LIBRARY_DEBUG "C:/XIMEA/API/x64/xiapi64.lib")
# set(XIMEA_SDK_LIBRARY_RELEASE "C:/XIMEA/API/x64/xiapi64.lib")
# include_directories(${XIMEA_SDK_INCLUDE_DIR})

set(MY_HEADER_FILES
    Exceptions.h
    helper_cuda.h
    helper_string.h
    NPPJpegCoder.h
    GenCameraDriver.h
    RealCameraDriver.h
    # # point grey camera
    # PointGrey/PointGreyCamera.h
    # # XIMEA camera
    # XIMEA/XIMEACamera.h
    # NetVirtual Camera
    NetVirtualCamera/Netbase.h
    NetVirtualCamera/NetCameraCore.h
    NetVirtualCamera/NetCameraHeader.h
    NetVirtualCamera/NetVirtualCamera.h
    NetVirtualCamera/CSharpSocketCppExport.h
    NetVirtualCamera/AsioSocket.h

    qthread_main.h
)

set(MY_SOURCE_FILES 
    # # point grey camera
    # PointGrey/PointGreyCamera.cpp
    # # XIMEA camera
    # XIMEA/XIMEACamera.cpp
    # NetVirtual Camera
    NetVirtualCamera/Netbase.cpp
    NetVirtualCamera/NetCameraCore.cpp
    NetVirtualCamera/NetVirtualCamera.cpp

    NetVirtualCamera/AsioSocket.cpp

    # C++ source file
    NPPJpegCoder.cpp
    # generic camera driver
    GenCameraDriver.cpp
    GenCameraDriverImpl.cpp
    RealCameraDriver.cpp
    # main function
    main.cpp

    qthread_main.cpp
)

# source_group("PointGrey" FILES
#     PointGrey/PointGreyCamera.h
#     PointGrey/PointGreyCamera.cpp 
# )

# source_group("XIMEA" FILES
#     XIMEA/XIMEACamera.h
#     XIMEA/XIMEACamera.cpp
# )

source_group("NetVirtual" FILES
    NetVirtualCamera/Netbase.h
    NetVirtualCamera/NetCameraCore.h
    NetVirtualCamera/NetCameraHeader.h
    NetVirtualCamera/NetVirtualCamera.h
    NetVirtualCamera/CSharpSocketCppExport.h
    NetVirtualCamera/Socket.h
    NetVirtualCamera/Netbase.cpp
    NetVirtualCamera/NetCameraCore.cpp
    NetVirtualCamera/NetVirtualCamera.cpp
    NetVirtualCamera/Socket.cpp
)

source_group("cuda" FILES
    cuda/NPPJpegCoderKernel.h
    cuda/NPPJpegCoderKernel.cu
)


include_directories(
	${PROJECT_SOURCE_DIR}	
	${PROJECT_SOURCE_DIR}/cuda	
	# ${PROJECT_SOURCE_DIR}/XIMEA	
    # ${PROJECT_SOURCE_DIR}/PointGrey	
    ${PROJECT_SOURCE_DIR}/NetVirtualCamera

    ${Qt5Widgets_INCLUDES}
	${Qt5Network_INCLUDES}
)

cuda_add_executable(${name}
    ${MY_HEADER_FILES}
    ${MY_SOURCE_FILES}
)

target_link_libraries(${name}
    ${OpenCV_LIBS}
    ${CUDA_LIBRARY} 	
	${CUDA_npp_LIBRARY} 
	${CUDA_nppc_LIBRARY} 
    ${CUDA_nppicc_LIBRARY}
    ${CUDA_nppicom_LIBRARY}
	${CUDA_nppidei_LIBRARY}
	${CUDA_nppif_LIBRARY}
	${CUDA_nppig_LIBRARY}
	${CUDA_nppim_LIBRARY}
	${CUDA_nppist_LIBRARY}
	${CUDA_nppisu_LIBRARY}
	${CUDA_nppitc_LIBRARY}
    ${CUDA_npps_LIBRARY} 
    ${CSHARP_SOCKET_LIBRARY}
    # debug ${PTGREY_SDK_LIBRARY_DEBUG} ${XIMEA_SDK_LIBRARY_DEBUG}
    # optimized ${PTGREY_SDK_LIBRARY_RELEASE} ${XIMEA_SDK_LIBRARY_RELEASE}
    Qt5::Widgets
    Qt5::Network
    

)

#add_custom_command(
#        TARGET ${name} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#                ${PROJECT_SOURCE_DIR}/NetVirtualCamera/config.xml
#                ${CMAKE_CURRENT_BINARY_DIR}/config.xml)

add_custom_command(
        TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/NetVirtualCamera/CSharpSocket.dll
                ${PROJECT_BINARY_DIR}/Debug/CSharpSocket.dll)

add_custom_command(
        TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/NetVirtualCamera/CSharpSocket.dll
                ${PROJECT_BINARY_DIR}/Release/CSharpSocket.dll)
                