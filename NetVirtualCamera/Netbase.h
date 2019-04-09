/***********************************************************************************************!
 * @file   :  base.h
 *
 * @brief  :  ͨ�û������塢����
 *
 * @author :  FrankHXW@gmail.com
 *
 * @date   :  2016/10/10 21:41
 *
 * @version:  V1.00
 *
 * @history:  V1.00����ɻ�������
 *
 * @detail :  ���������������͵Ķ��壬������Ϣ�����
 *
************************************************************************************************/
#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <QDebug>

/**************************************************************!
 *
 *	@brief: 
 *
 **************************************************************/
#define VARNAME(x) #x

//#ifndef max
//#define max(a,b)   (((a) > (b)) ? (a) : (b))
//#endif
//
//#ifndef min
//#define min(a,b)   (((a) < (b)) ? (a) : (b))
//#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846 
#endif

/**************************************************************!
 *
 *	@brief: ȡ�������ƺ궨��
 *
 **************************************************************/
#ifdef  __GNUC__
#define __FUNC__  __func__
#elif defined(_MSC_VER)
#define __FUNC__  __FUNCTION__
#else
#define __FUNC__ ""
#endif


 /**************************************************************!
 *
 *	@brief: ��Ϣ����궨�壬ֻ�ڵ���ģʽ��������޷���ֵ
 *
 **************************************************************/
#if defined(_MSC_VER)
#ifdef _DEBUG
#include <windows.h>
#include <sstream>
//#define TRACE(x)	OutputDebugStringA(LPCSTR(x))
#define TRACE(x)	qDebug()<<x
//#define TRACE(x)    std::cout <<x<<std::endl
#else
 #define TRACE(x)	OutputDebugStringA(LPCSTR(x))
//#define TRACE(x)    std::cout <<x<<std::endl
#endif

#else
#ifdef _DEBUG
#include <iostream>
#define TRACE(x)  std::cout << x<<std::endl
#else 
#define TRACE(x) 
#endif
#endif  


/**************************************************************!
*
*	@brief:  new-delete��ģ���ڴ��ͷ�
*
**************************************************************/
template<class T>
inline void SafeDelete(T* p)
{
	if (p != nullptr) {
		delete[]p;
		p = nullptr;
	}
}




/**************************************************************!
 *
 *	@brief: ������������
 *
 **************************************************************/
enum BaseErrorType
{
	No_Error = 0,						//everythig is ok
	Error_Common=-1,					//common error	
	Error_Invalid_Parameters=-2,		//invalid parameters		
	Error_Assert_Failed=-3,				//assert failed
	Error_ReadFile_Failed=-4,			//read file failed
	Error_WriteFile_Failed=-5,			//save file failed	
	Error_CreateFolder_Failed=-6,		//create folder failed
	Error_SocketSend_Failed=-7,			//socket send failed
	Error_SocketReceive_Failed=-8		//socket receive failed

};


 /***********************************************************************************************!
 * @method      :  CreatDirectory
 * @date        :  2016/10/11 20:36
 * @description :  ���������͵�ö�ٱ���תΪ�ַ���
 * @param t     :  _code���������
 * @return      :  ��������Ӧ���ַ���
 ************************************************************************************************/
const char *ErrorCodeToStr(int _code);


/***********************************************************************************************!
* @method      :  CreatDirectory
* @date        :  2016/10/11 20:36
* @description :  ��ƽ̨�½��ļ��У�֧�ֶ༶�ݹ��½�
* @param t     :  _path��Ŀ¼
* @return      :  _errorCode���ش������
************************************************************************************************/
#if defined(_MSC_VER)
#include <direct.h>  
#include <io.h>  
#define ACCESS _access  
#define MKDIR(a) _mkdir((a)) 
#elif defined(__GNUC__) 
#include <stdarg.h>  
#include <sys/stat.h>  
#define ACCESS access  
#define MKDIR(a) mkdir((a),0755)  
#endif 
BaseErrorType CreatDirectory(const char *_path);



/***********************************************************************************************!
 * @method      :  ErrorHandler
 * @date        :  2016/10/11 20:36
 * @description :  ������Ϣ������
 * @param t     :  _errorCode��������룬_message��������Ϣ,��Ҫ�ӻ��з�,   
 *				   _func�����ں���,_file�������ļ�, _line�������к�
 *				:  �����ǵ��Ի��Ƿ���ģʽ���������������Ϣ
 * @return      :  _errorCode���ش������
************************************************************************************************/
#ifdef _MSC_VER
#define _WINSOCKAPI_ 
#include <windows.h>
#include <sstream>
#else
#include <iostream>
std::cout << errorMessage.str();
#endif 
BaseErrorType ErrorHandler(BaseErrorType _errorCode, const char *_message, const char * _func, const char* _file, int _line);


/**************************************************************!
 *
 *	@brief: ���Ժ궨��
 *
 **************************************************************/
#define MY_ASSERT(expr)   if(!!(expr));else do{return ErrorHandler(BaseErrorType::Error_Assert_Failed,#expr,__FUNC__,__FILE__,__LINE__);}while(0)


 /**************************************************************!
 *
 *	@brief: ��������Ϣ�Ķ��Ժ궨��
 *
 **************************************************************/
#define MY_ASSERT_WITH_MESSAGE(expr,message)   if(!!(expr));else do{std::ostringstream errorMessage("");errorMessage << message << #expr;return ErrorHandler(BaseErrorType::Error_Assert_Failed,errorMessage.str().c_str(),__FUNC__,__FILE__,__LINE__);}while(0)

/**************************************************************!
 *
 *	@brief: ������Ϣ����궨��
 *
 **************************************************************/
#define MY_ERROR(errorCode,message) do{ return ErrorHandler(errorCode,message,__FUNC__ ,__FILE__, __LINE__);}while(0)


