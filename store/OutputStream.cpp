#include "StdHeader.h"
#include "OutputStream.h"

#include "util/Arrays.h"

using namespace std;
using namespace NSLib::util;
namespace NSLib{ namespace store{


	OutputStream::OutputStream()
	{
		buffer = new l_byte_t[ NSLIB_STREAM_BUFFER_SIZE ];
		bufferStart = 0;
		bufferPosition = 0;
	}
	
	OutputStream::~OutputStream(){
	}

	void OutputStream::close(){
		flush();
		//_DELETE( buffer );
		delete[] buffer;  // new[] (in constructor above) must be matched by
                                  // delete[] --RGR
                
                  
		bufferStart = 0;
		bufferPosition = 0;
	}

	/** OutputStream-like methods @see java.io.InputStream */
	void OutputStream::writeByte(const l_byte_t b) {
		if (bufferPosition >= NSLIB_STREAM_BUFFER_SIZE)
			flush();
		buffer[bufferPosition++] = b;
	}

	void OutputStream::writeBytes(const l_byte_t* b, const int length) {
		if ( length < 0 )
			_THROWC( "IO Argument Error. Value must be a positive value.");
		for (int i = 0; i < length; i++)
			writeByte(b[i]);
	}

	void OutputStream::writeInt(const int i) {
		writeByte((l_byte_t)(i >> 24));
		writeByte((l_byte_t)(i >> 16));
		writeByte((l_byte_t)(i >>  8));
		writeByte((l_byte_t) i);
	}

	void OutputStream::writeVInt(const int vi) {
		//TODO: FIX THIS!!!!
		uint i = vi;
		while ((i & ~0x7F) != 0) {
			writeByte((l_byte_t)((i & 0x7f) | 0x80));
			i >>= 7; // TODO: check if this is correct (i >>>= 7;)
		}
		writeByte( (l_byte_t)i );
	}

	void OutputStream::writeLong(const long_t i) {
		writeInt((int) (i >> 32));
		writeInt((int) i);
	}

	void OutputStream::writeVLong(const long_t vi) {
		//TODO: FIX THIS!!!!
		ulong_t i = vi;
		while ((i & ~0x7F) != 0) {
			writeByte((l_byte_t)((i & 0x7f) | 0x80));
			i >>= 7; //TODO: check if this is correct (i >>>= 7;)
		}
		writeByte((l_byte_t)i);
	}

	void OutputStream::writeString(const char_t* s) {
		int length = 0;
		if ( s != NULL ) length = stringLength(s);
		writeVInt(length);
		writeChars(s, 0, length);
	}

	void OutputStream::writeChars(const char_t* s, const int start, const int length){
		if ( length < 0 || start < 0 )
			_THROWC( "IO Argument Error. Value must be a positive value.");

		const int end = start + length;
		for (int i = start; i < end; i++) {
			const int code = (int)s[i];
			if (code >= 0x01 && code <= 0x7F)
				writeByte((l_byte_t)code);
			else if (((code >= 0x80) && (code <= 0x7FF)) || code == 0) {
				writeByte((l_byte_t)(0xC0 | (code >> 6)));
				writeByte((l_byte_t)(0x80 | (code & 0x3F)));
			} else {
				writeByte((l_byte_t)(0xE0 | (((uint)code) >> 12))); //TODO: used to be - writeByte((l_byte_t)(0xE0 | (code >>> 12)));
				writeByte((l_byte_t)(0x80 | ((code >> 6) & 0x3F)));
				writeByte((l_byte_t)(0x80 | (code & 0x3F)));
			}
		}
	}
	

	long_t OutputStream::getFilePointer() {
		return bufferStart + bufferPosition;
	}

	void OutputStream::seek(const long_t pos) {
		flush();
		bufferStart = pos;
	}

	void OutputStream::flush() {
		flushBuffer(buffer, bufferPosition);
		bufferStart += bufferPosition;
		bufferPosition = 0;
	}

}}
