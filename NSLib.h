#include "StdHeader.h"

#ifndef _NSLIB_DLL_
#define _NSLIB_DLL_

#ifdef _WIN32
  #define type_long __int64

  #ifdef NSLIBDLL_EXPORTS
    #define NSLIBDLL_API __declspec(dllexport)
  #elif NSLIBDLL_INCLUDED
    #define NSLIBDLL_API
  #else
    #define NSLIBDLL_API __declspec(dllimport) //far PASCAL
  #endif
#else
  #define NSLIBDLL_API
  typedef long long type_long;
#endif

#include <wchar.h>

#ifdef  __cplusplus
extern "C" {
#endif
  #define errlen 500
  
  //NSLIBDLL_API void NSL_CallBack_Error ( char* error );
  NSLIBDLL_API void NSL_Init();
  NSLIBDLL_API void NSL_Cleanup();
  NSLIBDLL_API int NSL_Open(const char* dir, int create);
  NSLIBDLL_API int NSL_WOpen(const fchar_t* wdir, int create);
  NSLIBDLL_API int NSL_Close(const char* dir);
  NSLIBDLL_API int NSL_WClose(const fchar_t* wdir);
  NSLIBDLL_API int NSL_Optimize(const char* dir);
  NSLIBDLL_API int NSL_WOptimize(const fchar_t* wdir);
  NSLIBDLL_API int NSL_Delete(const char* dir, const char* query, const char* qenc, const char* field);
  NSLIBDLL_API int NSL_WDelete(const fchar_t* wdir, const char_t* wquery, const char_t* field);

  NSLIBDLL_API void* NSL_New_Document();
  NSLIBDLL_API void  NSL_Free_Document(void* doc);
  NSLIBDLL_API int NSL_Add_Field(void* doc, const char* field, const char* value, const char* venc, int store, int index, int token);
  NSLIBDLL_API int NSL_WAdd_Field(void* doc, const char_t* wfield, const char_t* wvalue, int store, int index, int token);
  NSLIBDLL_API int NSL_Add_Date(void* doc, const char* field, type_long value, int store, int index, int token);
  NSLIBDLL_API int NSL_WAdd_Date(void* doc, const char_t* wfield, type_long value, int store, int index, int token);
  NSLIBDLL_API int NSL_Insert_Document(const char* dir, void* doc);
  NSLIBDLL_API int NSL_WInsert_Document(const fchar_t* wdir, void* doc);
  //NSLIBDLL_API void NSL_Document_Info(char* info,  const int len);
  NSLIBDLL_API char* NSL_wideToChar(const char16_t* wstr);

  NSLIBDLL_API void* NSL_Search(const char* dir, const char* query, const char* qenc, const char* field, const char* groupby);
  NSLIBDLL_API void* NSL_SearchWQ(const char* dir, const char_t* wquery, const char* field, const char* groupby);
  NSLIBDLL_API void* NSL_WSearch(const fchar_t* wdir, const char_t* wquery, const char_t* wfield, const char_t* wgroupby);
  NSLIBDLL_API char* NSL_Get_Keywords(const char* query, const char* qenc);
  NSLIBDLL_API char_t* NSL_WGet_Keywords(const char_t* wquery);
  NSLIBDLL_API void  NSL_Free_Keywords(char* keywords);
  NSLIBDLL_API void  NSL_WFree_Keywords(char_t* wkeywords);
  NSLIBDLL_API int NSL_HitCount(const void* hits);
  NSLIBDLL_API const char* NSL_HitGroupby(const void* hits);
  NSLIBDLL_API void* NSL_Hit(const void* hits, int hitPos); //return a document, that must not be freed.
  NSLIBDLL_API void NSL_ClearSearch(void* hits);
  NSLIBDLL_API char* NSL_Get_Field(const void* doc, const char* field, const char* enc);
  NSLIBDLL_API char_t* NSL_WGet_Field(const void* doc, const char_t* wfield);
  NSLIBDLL_API char* NSL_Get_Field_HTMLHiLite(const void* doc, const char* field, const char* keywords, const char* enc, int detail);
  NSLIBDLL_API char_t* NSL_WGet_Field_HTMLHiLite(const void* doc, const char_t* wfield, const char_t* wkeywords, int detail);
  NSLIBDLL_API type_long NSL_Get_DateField(const void* doc, const char* field);
  NSLIBDLL_API type_long NSL_WGet_DateField(const void* doc, const char_t* wfield);
  NSLIBDLL_API void NSL_Free_Field(char* value);
  NSLIBDLL_API void NSL_WFree_Field(char_t* wvalue);

  //NSLIBDLL_API void NSL_ErrStr(char* pl, int len);

#ifdef  __cplusplus
}
#endif

#endif
