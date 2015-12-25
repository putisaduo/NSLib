// dll.cpp : Defines the entry point for the console application.
//
#ifdef TR_LEAKS 
#ifdef _DEBUG
	int _lucene_BlockStop;
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif
#endif

#include "NSLib.h"
#include <iostream>

using namespace std;

#ifdef _WIN32
//#define INDEX "C:\\index"
#define INDEX "C:\\index\\SearchDB"
#else
#define INDEX "/home/fzhao/index"
#endif 

void dostuff(){
	//NSL_Init();
	//char word[] = {0xce, 0xb4, 0xc3, 0xfb, 0x00};
	//char word[] = {0xD8, 0x46, 0xDE, 0xF3};
	//char word[] = "研究";
	char word[] = {0xD8, 0x46, 0xDE, 0xF3, ' ','O', 'R', ' ', 0xd1, 0xd0, 0xbe, 0xbf, ' ','O', 'R', ' ', 0xcc, 0xec, 0xbd, 0xf2, 0x00};
	//char word[] = {0xa7, 0xf5, 0xc2, 0x45, 0xb3, 0xb9, 0x00};
	//char word[] = {0xc9, 0xcf, 0xba, 0xa3, ' ','O', 'R', ' ', 0xb1, 0xb1, 0xbe, 0xa9, ' ','O', 'R', ' ', 0xcc, 0xec, 0xbd, 0xf2, 0x00};
	//char word[] = "上海天津北京";
	//char word[] = {0xb2, 0x5b, 0xbb, 0x5c, '\\', 0x00};
	//char word[] = {0xb3, 0x5c, 0xa5, 0x69, 0x00};
	//char word[] = {0xba, 0xde, 0xb1, 0xb1, 0x00}; 
	//char word[] = "content: Edison";
	//char word[] = {0xbf, 0xb3, 0xb2, 0xb1, 0x20, 0xb4, 0xc1,0x00};
    //char word[] = {0xb8,0xea,0xb7,0xbd,0xaa,0xba,0xb9,0x42,0xa5,0xce,0x00};
    //char word[] = {0xb8,0x00};
    char* keywords = NSL_Get_Keywords(word, "gb2312");
	void* hits = NSL_Search(INDEX,keywords,"gb2312", "content", "");
    if ( hits != NULL ){
		cout << word << ">>Found: "<< NSL_HitCount(hits) << endl;
		printf( "     keywords: %s\n", keywords );
				
		char* val = NULL;
		int count = NSL_HitCount(hits);
		for ( int i = 0; i < count; i++ ) {
			void* doc = NSL_Hit(hits, i);
			char* val = NSL_Get_Field_HTMLHiLite(doc, "content", keywords, "gb2312", 0);
			if ( val != NULL ){
				printf( "     No: %d\n", i );
				printf( "Content: %s\n", val );
				NSL_Free_Field(val);
			}
		} 
	  }
	  else
		printf( "Error doing search\n");

      NSL_Free_Keywords(keywords);
	  NSL_ClearSearch(hits);
	
	
	//NSL_Cleanup();

	//NSL_Delete(INDEX,word,"gb2312", "contents");
}

void dostuff1()
{ 
	char word[] = {0xF0, 0xA3, 0x83, 0x99, 0x00, 0x00};
	//char word[] = "婬";
    //char* keywords = NSL_Get_Keywords(word);
	void* hits = NSL_Search(INDEX,word, "gb2312", "content", "");
    if ( hits != NULL ){
		cout << word << ">>Found: "<< NSL_HitCount(hits) << endl;
		//printf( "     keywords: %s\n", keywords );
				
		char* val = NULL;
		int count = NSL_HitCount(hits);
		for ( int i = 0; i < count; i++ ) {
			void* doc = NSL_Hit(hits, i);
			char* val = NSL_Get_Field(doc, "content", "gb2312");
			if ( val != NULL ){
				printf( "     No: %d\n", i );
				printf( "Content: %s\n", val );
			}
		} 
	  }
	  else
		printf( "Error doing search\n");

//    NSL_Free_Keywords(keywords);
	NSL_ClearSearch(hits);
}

int main(int argc, char* argv[])
{
	dostuff();
	//dostuff1();
	return 0;
}
