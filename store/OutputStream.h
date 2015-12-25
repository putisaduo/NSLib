#ifndef _NSLib_store_OutputStream_
#define _NSLib_store_OutputStream_
#include "StdHeader.h"

namespace NSLib{ namespace store{
	
	// A random-access output stream 
	class OutputStream{
	private:
		l_byte_t* buffer;
		long_t bufferStart;			  // position in file of buffer
		int bufferPosition;		  // position in buffer

	public:
		OutputStream();
		//OutputStream(OutputStream& clone);
		virtual ~OutputStream();

		//virtual OutputStream& clone()=0;

		// OutputStream-like methods @see java.io.InputStream 
		void writeByte(const l_byte_t b);

		void writeBytes(const l_byte_t* b, const int length);

		void writeInt(const int i);

		void writeVInt(const int vi);

		void writeLong(const long_t i);

		void writeVLong(const long_t vi);

		void writeString(const char_t* s);

		void writeChars(const char_t* s, const int start, const int length);
		
		virtual void close();

		// RandomAccessFile-like methods @see java.io.RandomAccessFile 
		long_t getFilePointer();

		virtual void seek(const long_t pos);

		virtual long_t Length() = 0;

	protected:
		void flush();

		virtual void flushBuffer(const l_byte_t* b, const int len) = 0;
	};
}}
#endif
