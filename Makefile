GPP=    g++
GCC=    gcc

NSLIB = ~/projects/NSLib/
Analysis = ${NSLIB}/analysis
Document = ${NSLIB}/document
Index = ${NSLIB}/index
QueryParser = ${NSLIB}/queryParser
Search = ${NSLIB}/search
Store = ${NSLIB}/store
Util = ${NSLIB}/util

# paths for all the header files
INCL_DIR  = -I /usr/include -I /usr/local/include -I ${NSLIB}

CFLAGS= -fPIC -fexceptions -DNDEBUG -W -O3 -std=c++11 -I $(INCL_DIR)
#CFLAGS= -fPIC -fexceptions -DNDEBUG -W -g -std=c++11 -I $(INCL_DIR)


OBJS = NSLib.o StdHeader.o $(Analysis)/StandardTokenizer.o \
	$(Analysis)/ChineseAnalyzer.o $(Analysis)/ChineseTokenizer.o \
	$(Analysis)/StandardAnalyzer.o $(Analysis)/StandardFilter.o \
	$(Document)/DateField.o $(Document)/Document.o $(Document)/Field.o \
	$(Index)/DocumentWriter.o $(Index)/FieldInfos.o $(Index)/FieldsReader.o \
	$(Index)/FieldsWriter.o $(Index)/IndexReader.o $(Index)/IndexWriter.o \
	$(Index)/SegmentInfos.o $(Index)/SegmentMergeInfo.o \
	$(Index)/SegmentMergeQueue.o $(Index)/SegmentMerger.o \
	$(Index)/SegmentReader.o $(Index)/SegmentsReader.o \
	$(Index)/SegmentTermDocs.o $(Index)/SegmentTermEnum.o \
	$(Index)/SegmentTermPositions.o $(Index)/Term.o $(Index)/TermInfo.o \
	$(Index)/TermInfosReader.o $(Index)/TermInfosWriter.o \
	$(QueryParser)/Lexer.o $(QueryParser)/MultiFieldQueryParser.o \
	$(QueryParser)/QueryParserBase.o $(QueryParser)/QueryParser.o \
	$(QueryParser)/QueryToken.o $(QueryParser)/TokenList.o \
	$(Search)/BooleanQuery.o $(Search)/BooleanScorer.o $(Search)/DateFilter.o \
	$(Search)/ExactPhraseScorer.o $(Search)/FilteredTermEnum.o \
	$(Search)/FuzzyQuery.o $(Search)/HitQueue.o $(Search)/Hits.o \
	$(Search)/IndexSearcher.o $(Search)/MultiSearcher.o $(Search)/MultiTermQuery.o \
	$(Search)/PhrasePositions.o $(Search)/PhraseQuery.o $(Search)/PhraseScorer.o \
	$(Search)/PrefixQuery.o $(Search)/RangeQuery.o $(Search)/Similarity.o \
	$(Search)/SloppyPhraseScorer.o $(Search)/TermQuery.o $(Search)/TermScorer.o \
	$(Search)/TopDocs.o $(Search)/WildcardQuery.o $(Search)/WildcardTermEnum.o \
	$(Store)/FSDirectory.o $(Store)/InputStream.o $(Store)/Lock.o \
	$(Store)/OutputStream.o $(Store)/RAMDirectory.o \
	$(Util)/Arrays.o $(Util)/BitVector.o $(Util)/CharConverter.o $(Util)/dict.o \
	$(Util)/dirent.o $(Util)/FastCharStream.o $(Util)/HTMLHighLighter.o \
	$(Util)/Misc.o $(Util)/Reader.o $(Util)/StringBuffer.o $(Util)/WordSegmenter.o

LIB= libNSLib

# type "make all" to compile the project
all:    $(LIB)

libNSLib:	$(OBJS)
	ar rvu $(LIB).a $(OBJS)
	ranlib $(LIB).a
	cp $(LIB).a ..

%.o:%.cpp
	$(GPP) $(CFLAGS)  -c $< -o $@

%.o:%.cc
	$(GPP) $(CFLAGS)  -c $< -o $@

# type "make clean" to clean all files generated during the compiling process
clean:
	rm -f *.o core 
	rm $(LIB).a

