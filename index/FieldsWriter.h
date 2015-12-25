#include "StdHeader.h"
#ifndef _NSLib_index_FieldsWriter_
#define _NSLib_index_FieldsWriter_

#include "util/VoidMap.h"
#include "store/Directory.h"
#include "store/OutputStream.h"
#include "document/Document.h"
#include "document/Field.h"
#include "FieldInfos.h"

namespace NSLib{ namespace index {
	class FieldsWriter {
	private:
		FieldInfos& fieldInfos;
		OutputStream* fieldsStream;
		OutputStream* indexStream;

		const static fchar_t* segmentname( const fchar_t* segment, const fchar_t* ext );
	public:
		FieldsWriter(Directory& d, const fchar_t* segment, FieldInfos& fn);
		~FieldsWriter();

		void close();

		void addDocument(NSLib::document::Document& doc);
	};
}}
#endif
