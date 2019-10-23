/***********************************************************************************************!
* @file   :  base.cpp
*
* @brief  :  xxxxxxxxxxxxxxxxxx
*
* @author :  FrankHXW@gmail.com
*
* @date   :  2016/10/10 21:41
*
* @version:  V1.00
*
* @history:  V1.00xxxxxxxxxxxxxx
*
* @detail :  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*
************************************************************************************************/
#include "Netbase.h"

const char *ErrorCodeToStr(int _code)
{
	switch (_code) {
	case No_Error:							return	"Ok";
	case Error_Common:						return 	"Common error";
	case Error_Invalid_Parameters:			return  "Invalid parameters";
	case Error_Assert_Failed:				return	"Assert failed";
	case Error_ReadFile_Failed:				return  "Read file failed";
	case Error_WriteFile_Failed:			return  "Write file failed";
	case Error_CreateFolder_Failed:			return  "Create folder failed";
	case Error_SocketSend_Failed:			return  "Socket send failed";
	case Error_SocketReceive_Failed:		return  "Socket receive failed";
	default:								return  "Unknown error ";
	}
}


BaseErrorType CreatDirectory(const char *_path)
{
	int32_t length = int32_t(strlen(_path));
	char *path = new char[length + 2];
	memcpy(path, _path, length);
	//xxxxxxxxxxxx/  
	if (path[length - 1] != '\\' && path[length - 1] != '/') {
		path[length] = '/';
		path[length + 1] = '\0';
	}
	//xxxxxxxx  
	for (int32_t i = 0; i <= length; ++i) {
		if (path[i] == '\\' || path[i] == '/') {
			path[i] = '\0';
			//xxxxxxxxxx,xxxx  
			int32_t error = ACCESS(path, 0);
			if (error != 0) {
				error = MKDIR(path);
				if (error != 0) {
					return Error_CreateFolder_Failed;
				}
			}
			//xxxxlinux,xxxxxx\xxxx/  
			path[i] = '/';
		}
	}
	delete[]path;
	return No_Error;
}


BaseErrorType ErrorHandler(BaseErrorType _errorCode, const char *_message, const char * _func, const char* _file, int _line)
{
	std::ostringstream errorMessage;
	errorMessage << ErrorCodeToStr(_errorCode) << ": " << _message << " in function: " << _func << ",file: " << _file << ",line: " << _line << std::endl;
#ifdef _MSC_VER
	OutputDebugStringA(LPCSTR(errorMessage.str().c_str()));
//	std::cout << errorMessage.str() << std::endl;
#else
#include <iostream>
	std::cout << errorMessage.str()<<endl;
#endif  
	//	if (_errorCode == ErrorCode::Error_Assert_Failed)
	//		throw errorMessage.str();
	return _errorCode;
}