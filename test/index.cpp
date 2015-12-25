// test.cpp : Defines the entry point for the console application.
//
/*
#ifdef TR_LEAKS 
#ifdef COMPILER_MSVC
#ifdef _DEBUG
	int _lucene_BlockStop;
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif
#endif
#endif
*/
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
//@@Chinese Extension -- BEGIN
//#include <tchar.h>
//@@Chinese Extension -- END
#include "NSLib.h"
//#include <fstream>
#include <iostream>

using namespace std;
#ifdef _WIN32
#define INDEX "c:\\index"
#else
#define INDEX "/home/fzhao/index"
#endif
/*
int main(int argc, char* argv[])
{
	NSL_Init();
	//Dumper Debug
	#ifdef TR_LEAKS 
	#ifdef COMPILER_MSVC
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );//| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF );
	#endif
	#endif
	#endif
	
	char err[errlen];

	//NSL_Delete(INDEX,"beautiful","contents");
	//return;

	//ifstream ifs;
	//ifs.open("1.txt", ios::in);
	//if (!ifs.is_open()) exit(0);

	//size_t flen = ifs.seekg(0, ios::end).tellg();
	//ifs.seekg(0, ios::beg);
	//char* str = new char[flen+1];
	//memset(str, 0, flen);
	//ifs.read(str, flen);
	//str[flen] = 0;

	//ifs.close();

	//add a document
	NSL_Open(INDEX,1);

	for ( int i=0;i<1;i++){
		void* doc = NSL_New_Document();
		char str[] = {0xD8, 0x46, 0xDE, 0xF3, 0xD8, 0x68, 0xDE, 0x1A,
			0xd1, 0xd0, 0xbe, 0xbf, 0xcb, 0xf9, 0xb5, 0xc4, 0xb9, 0xca, 0xca, 0xc2, 0x00};

		NSL_Add_Field(doc, "content", str, "gb2312", 1,1,1); //not stored
		//NSL_Add_Field(doc, "key", "ab", "big5", 1,1,1); //not stored
		//NSL_Add_Field(doc, "content", "上海有金贸大厦", "gb2312", 1,1,1); //not stored

		//__time64_t ltime;
		//_time64( &ltime );
		//NSL_Add_Date(doc, "date",ltime,1,0,0); //stored only

		//NSL_Add_Field("note","This is a stored note.",23,1,0,0); //stored only

		char docinfo[1024];
		//NSL_Document_Info(docinfo,1024);
		//printf("Adding value: %s\n",docinfo);
		//printf("Adding %s\n",_ctime64( &ltime ));

		if ( NSL_Insert_Document(INDEX, doc) == 0)
			printf ("Document inserted...\n");
		else{
			//NSL_ErrStr(err,errlen);
			printf ("Document insert failed - %s!\n", err);
		}
		printf("\n");

		NSL_Free_Document(doc);
		doc = NSL_New_Document();
		NSL_Add_Field(doc, "content", "上海北京天津 great wall", "gb2312", 1,1,1); //not stored
		if ( NSL_Insert_Document(INDEX, doc) == 0)
			printf ("Document inserted...\n");
		else{
			//NSL_ErrStr(err,errlen);
			printf ("Document insert failed - %s!\n", err);
		}
		printf("\n");
		NSL_Free_Document(doc);
	}

	if ( NSL_Optimize(INDEX) < 0 ){
		//NSL_ErrStr(err,errlen);
		printf ("Failed optimizing index - %s", err);
	}

	NSL_Close(INDEX);

	//do a search
	//if ( NSL_Search(INDEX,"space","gb2312", "contents") ){
	//	printf ("Found %d documents\n",NSL_HitCount());

	//	int i=1;
	//	do{
	//		const type_long dt = NSL_GetDateField("date");

	//		char* note = NULL;
	//		size_t ln = 0;
	//		NSL_GetField("note",&note,&ln, "gb2312");

	//		printf("%d. note: %s, date: %s",i, note, _ctime64(&dt));

	//		i++;
	//	}while(NSL_NextHit());

	//}else{
	//	NSL_ErrStr(err,errlen);
	//	printf ("Search failed - %s!\n",err);
	//}
	return 0;	
	
	//Debuggin techniques:
	//In the Watch window, type the following expression in the Name column: 
	//_crtBreakAlloc
	//_cout << endl << endl << _T("Time taken: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;

}
*/
int main(int argc, char* argv[])
{
	//NSL_Init();
	
	char_t ch = 0x0451;
	locale loc1 ( "Latvian");
	char_t ch1 = use_facet<ctype<wchar_t> >(loc1).toupper(ch);
	cout << ch << "-- Convert to Upper--" << ch1 << endl;

	//char_t str[] = {0x0072, 0x0073, 0x00};
	//cout << *str;
	//use_facet<ctype<char_t> >(loc1).toupper(&str[0], &str[wcslen(str)]);
	//cout << "--- The uppercase string is: " << *str << endl;
	
	char err[errlen];
	int ret = 0;

	//add a document
	NSL_WOpen(INDEX,1);

	for ( int i=0;i<1;i++){
		void* doc = NSL_New_Document();

		// Chinese 4 Bytes
		//char_t str[] = {0xD846, 0xDEF3, 0xD868, 0xDE1A, 0xD85B, 0xDEB6, 0xD84C, 0xDCD9, 0x0000};
		
		// Mongolian
		char_t str[] = {0xE24A, 0xE283, 0xE27E, 0x0020, 0xE27F, 0xE247, 0xE270, 0x0020, 0xE27D, 0xE24F, 0x0020};

		// WestenChars
		//char_t str[] = {0x0044, 0x00E9, 0x0063, 0x0072, 0x0065, 0x0074, 0x0020, 0x0073, 0x0075, 0x0072, 0x0020, 0x0075, 0x20, 0x64, 0xe9, 0x6a, 0xe0, 0x20, 0x0451, 0x00};
		NSL_WAdd_Field(doc, L"content", str, 1,1,1); //not stored
		//NSL_WAdd_Field(doc, L"content", L"上海北京天津 great wall", 1,1,1); //not stored

		//char docinfo[1024];

		if ( NSL_WInsert_Document(INDEX, doc) == 0)
			printf ("Document inserted...\n");
		else
			printf ("Document insert failed - %s!\n", err);
		
		printf("\n");
		
		//getchar();

		NSL_Free_Document(doc);

		//getchar();

		doc = NSL_New_Document();
		NSL_WAdd_Field(doc, L"content", L"入桀曰椉 天津 great wall", 1,1,1); //not stored
		//NSL_WAdd_Field(doc, L"content", L"上海北京天津 great wall", 1,1,1); //not stored
		if ( (ret=NSL_WInsert_Document(INDEX, doc)) == 0)
			printf ("Document inserted...\n");
		else
		{
			//NSL_ErrStr(err,errlen);
			//printf ("Document insert failed - %s!\n", err);
		}
		printf("\n");
		NSL_Free_Document(doc);
	}

	if ( NSL_Optimize(INDEX) < 0 ){
		printf ("Failed optimizing index - %s", err);
	}

	NSL_Close(INDEX);

	//do a search
	//char_t word[] = {0x46, 0xD8, 0xF3, 0xDE, ' ','O', 'R', ' ', 0xd1, 0xd0, 0xbe, 0xbf, ' ','O', 'R', ' ', 0xcc, 0xec, 0xbd, 0xf2, 0x00};
	//char_t word[] = L"上海";//{0xD846, 0xDEF3, 0x0000};
	//char_t word[] = {0xE24A, 0xE283, 0xE27E, 0x0000};
	char_t word[] = L"椉";
	//char_t word[] = {0xbc97, 0x0000}; //d846de63
	//char_t word[] = {0x64, 0xe9, 0x6a, 0xe0, 0x00};
	//char_t word[] = {0x0073, 85/*0x0075*/, 0x0072, 0x0000};
/*
	//char_t* keywords = NSL_WGet_Keywords(word);
	char_t* keywords = NSL_WGet_Keywords(word);
	for ( int i=0;i<10000;i++)
	{
		void* hits = NSL_WSearch(INDEX,word,L"content");
		if ( hits != NULL ){
			cout << word << ">>Found: "<< NSL_HitCount(hits) << endl;
			printf( "     keywords: %s\n", keywords );
			/*		
			char* val = NULL;
			int count = NSL_HitCount(hits);
			for ( int i = 0; i < count; i++ ) {
				void* doc = NSL_Hit(hits, i);
				char* val = NSL_WGet_Field_HTMLHiLite(doc, L"content", keywords, 0);
				if ( val != NULL ){
					cout << "     No: " << i << "\n";
					cout << "Content: " <<  val << "\n";
					NSL_Free_Field(val);
				}
			} 
			*/
/*		}
		else
			printf( "Error doing search\n");

		NSL_ClearSearch(hits);
	}
	NSL_WFree_Keywords(keywords);
*/
	return 0;	
}

