#include "WordSegmenter.h"
#include <ctype.h>
#include <locale.h>

namespace NSLib {namespace util{
  WordSegmenter::WordSegmenter(ChineseReader* reader, bool del) 
    :m_reader(reader), m_del(del)
  {
  }

  WordSegmenter::~WordSegmenter() {
    if ( m_del) delete m_reader;
  }

  bool WordSegmenter::ischnalnum(char_t ch) {
#ifdef _WIN32
    return isAlNum(ch) != 0;
#else
    bool ret = false;
    const char* old_ctype = setlocale(LC_CTYPE, "");
    setlocale(LC_CTYPE, LANG_CHT);
    ret = isAlNum(ch);
    setlocale(LC_CTYPE, LANG_CHS);
    ret |= isAlNum(ch);
    setlocale(LC_CTYPE, old_ctype);
    return ret;
#endif
  }

  bool WordSegmenter::ischnspace(char_t ch) {
#ifdef _WIN32
    return isSpace(ch) != 0;
#else
    bool ret = false;
    const char* old_ctype = setlocale(LC_CTYPE, "");
    setlocale(LC_CTYPE, LANG_CHT);
    ret = isSpace(ch);
    setlocale(LC_CTYPE, LANG_CHS);
    ret |= isSpace(ch);
    setlocale(LC_CTYPE, old_ctype);
    return ret;
#endif
  }

  bool WordSegmenter::iswestenalnum(char_t ch)
  {
#ifdef SUPPORT_ALL_WESTEN
    // Russian
    if ((ch>=0x0401 && ch<=0x044F)||(ch>=0x0451 && ch<=0x045F)||(ch>=0x0490 && ch<=0x0493)
      || (ch>=0x0496 && ch<=0x0497)||(ch>=0x049A && ch<=0x049D)||(ch>=0x04A2 && ch<=0x04A3)
      || (ch>=0x04AE && ch<=0x04AF)||(ch>=0x04B0 && ch<=0x04B3)||(ch>=0x04B8 && ch<=0x04BB)
      || (ch>=0x04D8 && ch<=0x04D9)||(ch>=0x04E8 && ch<=0x04E9))
    {
      return true;
    }

    // Other Latin Charactors
    if ((ch>=0x00C0 && ch<=0x00D6)||(ch>=0x00D8 && ch<=0x00F6)||(ch>=0x00F8 && ch<=0x017F)
      ||(ch>=0x01A0 && ch<=0x01A1)||(ch>=0x01AF && ch<=0x01B0)||(ch>=0x01CD && ch<=0x01DC)
      ||(ch>=0x01FA && ch<=0x01FF)|| ch==0x018F || ch==0x0192 || ch==0x0251 || ch==0x0261)
    {
      return true;
    }

    // Hebrew
    if ((ch>=0x0591 && ch<=0x05C7)||(ch>=0x05D0 && ch<=0x05EA)||(ch>=0x05F0 && ch<=0x05F4)
      // Following is Alphabetic
      ||(ch>=0xFB00 && ch<=0xFB06)||(ch>=0xFB13 && ch<=0xFB17)||(ch>=0xFB1D && ch<=0xFB36)
      ||(ch>=0xFB38 && ch<=0xFB3C)|| ch==0xFB3E || ch==0xFB40 || ch==0xFB41 || ch==0xFB43 
      || ch==0xFB44 ||(ch>=0xFB46 && ch<=0xFB4F))
    {
      return true;
    }
#endif
    if (ch & 0xff00)
      return false;
    return isalnum(ch);
  }

  char_t* WordSegmenter::next(char_t* buf, size_t len) {
    char_t* ptr = buf;
    char_t ch = 0;
    char_t tmp_buf[2];
    while ( true ) {
  start:
      if ( m_reader->eof() ) {
        *ptr = 0; //terminate this string
        break;
      }
      ch = m_reader->peek();

#ifdef SUPPORT_TIBETAN //U+0F00 --- U+0FCF, words is seperated by SPACE and U+0F0B
      if (ch>=0x0F00 && ch<=0x0FCF) // Tibetan
        goto Tibetan;
#endif

#ifdef SUPPORT_MONGOLIAN // U+1800 --- U+18A9, words is seperated by SPACE
      if ((ch>=0xE235 && ch<=0xE293)||(ch>=0xE301 && ch<=0xE353)) // Mongolian
        goto Mongolian;
#endif
      //english?
      //if ( !(ch & 0xff00) && iswestenalnum(ch) ) goto english;
      if ( iswestenalnum(ch) ) goto english;

      //chinese?
      //if ( (ch & 0xff00) && ischnalnum(ch) ) goto chinese1;
      if (ch & 0xff00) goto chinese1;

      //ignored
      m_reader->next();
      goto start;

#ifdef SUPPORT_TIBETAN //U+0F00 --- U+0FCF, words is seperated by SPACE and U+0F0B
  Tibetan:
      *ptr++ = m_reader->next(); //copy this char to the buf
      if ( ptr == buf + len - 1 ) { //is there space left on buf? 
        while ( !m_reader->eof() ) { //skip the rest of word
          ch = m_reader->next();
          if (ch<0x0F00 || ch>0x0FCF || ch==0x0F0B) 
            break ;
        }
        *ptr = 0; //terminate this string
        break;
      }
      //next
      if ( m_reader->eof()) {
        *ptr = 0; //terminate this string
        break;
      }
      ch = m_reader->peek();
      if (ch>=0x0F00 && ch<=0x0FCF)
        goto Tibetan;
      *ptr = 0; //terminate this string
      break;
#endif

#ifdef SUPPORT_MONGOLIAN // U+1800 --- U+18A9, words is seperated by SPACE
  Mongolian:
      *ptr++ = m_reader->next(); //copy this char to the buf
      if ( ptr == buf + len - 1 ) { //is there space left on buf? 
        while ( !m_reader->eof() ) { //skip the rest of word
          ch = m_reader->next();
          if (ch!=0x0020) 
            break ;
        }
        *ptr = 0; //terminate this string
        break;
      }
      if ( m_reader->eof()) {
        *ptr = 0; //terminate this string
        break;
      }
      ch = m_reader->peek();
      //determine which state is the next
      if ((ch>=0xE235 && ch<=0xE293)||(ch>=0xE301 && ch<=0xE353))
        goto Mongolian;

      *ptr = 0; //terminate this string
      break;
#endif

  english:
      //copy this char to the buf
      *ptr++ = m_reader->next();
      
      //is there space left on buf?
      if ( ptr == buf + len - 1 ) {
        //skip the rest of word
        while ( !m_reader->eof() ) {
          if (!iswestenalnum(m_reader->next())) break ;
        }
        *ptr = 0; //terminate this string
        break;
      }

      //next
      if ( m_reader->eof() ) {
        *ptr = 0; //terminate this string
        break;
      }
      ch = m_reader->peek();

      //determine which state is the next
      if ( /*!(ch & 0xff00) &&*/iswestenalnum(ch)) goto english ;
      *ptr = 0; //terminate this string
      break;

  chinese1:
      //copy this char to the buf
      *ptr++ = m_reader->next();
      char_t ch0 = *(ptr-1);
      
      //D800 - DBFF, DC00 -DFFF is 4 bytes charactor

      //is there space left on buf?
      if ( ptr == buf + len - 1 ) {
        *ptr = 0; //terminate this string
        break;
      }
      bool bExistSpace = false;
      //skip space
      while ( !m_reader->eof() && ischnspace(m_reader->peek()) ) {
        m_reader->next();
        bExistSpace = true;
      }

      //next
      if ( !m_reader->eof() ) {
        ch = m_reader->peek();

#ifdef SUPPORT_4_BYTE
        if (!bExistSpace && ch0>=0xD800 && ch0<=0xDBFF && ch>=0xDC00 && ch<=0xDFFF) {
          // 4 bytes Chinese4 charactor
          *(ptr) = m_reader->peek();
            
          //is a word, next reader
          size_t old_pos = m_reader->position();
          m_reader->next();

          ptr++;
          *ptr = 0; //terminate this string
          break;
        }
#endif

        //chinese?
        if ( (ch & 0xff00) && ischnalnum(ch) ) {
          // chinese2
          *(ptr) = m_reader->peek();
          if ( isWord(buf) ) {
            //is a word, next reader
            size_t old_pos = m_reader->position();
            tmp_buf[0] = m_reader->peek(); 
            m_reader->next();
            if ( !m_reader->eof() ) {
              tmp_buf[1] = m_reader->peek();
              if ( isWord(tmp_buf) ) 
                m_reader->seek(old_pos);
            }
            ptr++;
          }
        }
      }
      *ptr = 0; //terminate this string
      break;
    }

    return (ptr == buf)? NULL : buf;
  }

  bool WordSegmenter::isWord(char_t *str) {
    int start = DICT_INDEX[str[0]][0];
    int end = DICT_INDEX[str[0]][1];
    
    //return false if no word starts with this character
    if ( start == -1 ) return false;

    for ( int i = start; i < end; i++ ) {
      char_t ch = ((char_t*)&DICT_STRING)[i];
      if ( ch == str[1] ) return true;
    }

    return false;
  }

}}
