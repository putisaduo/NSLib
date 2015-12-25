#include <algorithm>

#include "HTMLHighLighter.h"
#include "CharConverter.h"

using namespace std;

#ifdef _UNICODE

#define OPEN_HILITE _T("<font color=red><b>")
#define CLOSE_HILITE _T("</b></font>")

#else

#define OPEN_HILITE "<font color=red><b>"
#define CLOSE_HILITE "</b></font>"

#endif

#define EXTRA_TEXT_LEN 50

#define MAX_TXT_LEN 100

namespace NSLib { namespace util {
  vector<NSLibString>* HTMLHighLighter::getKeywordList(const char_t* keywords) {
    vector<NSLibString> *list = new vector<NSLibString>;
    for ( size_t i = 0, len = stringLength(keywords); i < len; i++ ) {
      //skip space
      if ( isSpace(keywords[i]) ) continue;
      //found one
      size_t i_save = i;
      for ( ; i < len; i++ ) {
        if ( !isSpace(keywords[i]) ) continue;
        else break;
      }
      //add to the list
      list->push_back(NSLibString(keywords + i_save, i-i_save));
    }
    return list;
  }

  bool HTMLHighLighter::isChineseWord(const char_t *word) {
    for ( const char_t* ptr = word; *ptr != 0; ptr++ ) 
      if ( *ptr & 0xff00 ) return true;
    return false;
  }

  int HTMLHighLighter::matchWord(const char_t* content, const char_t* keyword, int from) {
    if ( content == NULL || keyword == NULL ) return -1;
    size_t clen = stringLength(content);
    size_t klen = stringLength(keyword);
#ifdef SUPPORT_ALL_WESTEN
    locale loc1 ( "Latvian");
#endif
        while (true) {
      int index = -1;
      for ( int ci = from; ci < (int)clen; ci++ ) {
        index = ci;
        for ( int ki = 0; ki < (int)klen; ki++ ) {
          char_t kch = keyword[ki];
          if ( !(kch & 0xff00) ) 
          {
#ifdef SUPPORT_ALL_WESTEN
            kch = use_facet<ctype<wchar_t> >(loc1).toupper(kch);
#else
            kch = toupper(kch);
#endif
          }
          char_t cch = content[ci + ki];
          if ( !(cch & 0xff00) ) 
          {
#ifdef SUPPORT_ALL_WESTEN
            cch = use_facet<ctype<wchar_t> >(loc1).toupper(cch);
#else
            cch = toupper(cch);
#endif
          }
          if ( cch != kch ) {
            index = -1;
            break;
          }
        }
        if ( index != -1 ) break;
      }

            if (index == -1) return -1; //not found

      char_t before = index != 0 ? *(content + index - 1) : ' ';
      char_t follow = index + klen < clen ? *(content + index + klen) : ' ';
      //avoid to match a substring of another english word
      if ( isChineseWord(keyword) ) 
        return index;
      else if ( (before & 0xff00 || !isalnum(before)) && 
              (follow & 0xff00 || !isalnum(follow)) )
                return index;
            else 
                from = index + klen;
        }
  }

  char_t* HTMLHighLighter::trim(const char_t* content) {
    if ( content == NULL || stringLength(content) == 0 ) return NULL;
    
    size_t clen = stringLength(content);
    char_t* trimed = new char_t[clen + 1];
    
    size_t ti = 0;
    for ( size_t ci = 0; ci < clen; ci++ ) {
      //skip space
      if ( isSpace(content[ci]) ) continue;
      //found one
      size_t ci_save = ci;
      for ( ; ci < clen; ci++ ) {
        if ( !isSpace(content[ci]) ) continue;
        else break;
      }

      stringNCopy(trimed + ti, content + ci_save, ci - ci_save);
      ti += ci - ci_save;
      trimed[ti] = ' ';
      ti ++;
    }
    trimed[ti] = 0;

    return trimed;
  }

  char_t* HTMLHighLighter::highlight(const char_t* content, const char_t* keywords, bool detail) {
    if ( content == NULL || stringLength(content) == 0 ) return NULL;
    if ( keywords == NULL || stringLength(keywords) == 0 ) return NULL;
    
    int starts[52], ends[52];
    int start = 0, end = 0;
    int n = 0, from = 0;
    int k = 0;
        NSLibString result;
    char_t* hilite = NULL;

    //trim content
    char_t* trimedContent = trim(content); 

    //get keyword list    
    vector<NSLibString> *keywordList = getKeywordList(keywords);

    //locate matched text strings
    while ( n < 50) {
      int index = 0x7fffffff, keyNo = -1;
      for (int i = 0; i < (int)keywordList->size(); i++) {
        NSLibString key = keywordList->at(i);
        int tmp = matchWord(trimedContent, key.c_str(), from);
        if (index > tmp && tmp != -1) {
          index = tmp;
          keyNo = i;
        }
      }
      if (keyNo == -1)
        break;

      starts[n] = index;
      ends[n] = index + (keywordList->at(keyNo)).length();

      from = index + 1;

      n++;
    }

    //no matched keywords are found
    if ( n <= 0) {
      if ( stringLength(trimedContent) <= MAX_TXT_LEN ) {
        hilite = stringDuplicate(trimedContent);
      } else {
        //content is too long
        char_t* value = new char_t[MAX_TXT_LEN+4];
        memcpy(value, trimedContent, sizeof(char_t)*MAX_TXT_LEN);
        //append ellipses
        for ( int i = 0; i < 3; i++ ) value[MAX_TXT_LEN + i] = '.';
        value[MAX_TXT_LEN + 3] = 0;

        hilite = value;
      }

      goto done;
    }

    k = detail ? 45 : 3; //number of keywords that will appear

    //merge
    for (int i = 0; i < n - 1; i++) {
      if (starts[i + 1] <= ends[i]) {
        ends[i] = ends[i + 1];
        for (int j = i + 1; j < n; j++) {
          starts[j] = starts[j + 1];
          ends[j] = ends[j + 1];
        }
        n--;
        i--;
      }
    }
    
    k = n < k ? n : k;
    if (n > 2 && !detail) {
      for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
          if (ends[j] - starts[j] > ends[i] - starts[i] && starts[j] >= 0) {
            int tmp = ends[i];
            ends[i] = ends[j];
            ends[j] = tmp;
            tmp = starts[i];
            starts[i] = starts[j];
            starts[j] = tmp;
          }
        }
      }
      n = k;
      for (int i = 0; i < k - 1; i++) {
        for (int j = i + 1; j < k; j++) {
          if (starts[i] > starts[j]) {
            int tmp = ends[i];
            ends[i] = ends[j];
            ends[j] = tmp;
            tmp = starts[i];
            starts[i] = starts[j];
            starts[j] = tmp;
          }
        }
      }
    }

    // Add <b> to the original text
    from = 0;
    for (int i = 0; i < n; i++) {
      if (starts[i] > 0) {
        if (!detail) {
          int s = starts[i] - EXTRA_TEXT_LEN;
          if (s > from) {
            from = s;
            for (int c = from; c < starts[i]; c++) {
              char_t ch = trimedContent[c];
              if (ch < 'a' || ch > 'z') {
                from = c;
                break;
              }
            }
          }
          if ((from > 0 && i == 0) || (i > 0 && from > ends[i - 1]))
          result.append(_T("..."));
        }
        result.append(NSLibString(trimedContent + from, starts[i] - from));
      }
      result.append(OPEN_HILITE);
      result.append(NSLibString(trimedContent + starts[i], ends[i] - starts[i]));
      result.append(CLOSE_HILITE);

      from = ends[i];

      ///////////// Feng 2001/05/14
      int tail = stringLength(trimedContent) - 1;
      if (detail) {
        if (ends[i] <= tail && i == n - 1)
        result.append(NSLibString(trimedContent + ends[i], tail - ends[i]));
      } else {
        end = ends[i] + EXTRA_TEXT_LEN;
        if (i < n - 1 && starts[i + 1] < stringLength(trimedContent))
          tail = starts[i + 1];

        if (end > tail)
          end = tail;
        if (ends[i] == tail)
          result.append(NSLibString(trimedContent + ends[i], end - ends[i]));
          //result += summary.substring(ends[i], end);
        else if (end >= 0 && ends[i] >= 0) {
          for (int c = end; c > ends[i]; c--) {
            char_t ch = trimedContent[c];
            if (ch < 'a' || ch > 'z') {
              end = c;
              break;
            }
          }
          if (ends[i] < end) {
            result.append(NSLibString(trimedContent + ends[i], end - ends[i]));
            result.append(_T("..."));
          }
        }
      }
    }

    //hilite = CharConverter::wideToChar(result.c_str(), enc);
    hilite = stringDuplicate(result.c_str());

done:
    //clean it up
    delete[] trimedContent;
    delete keywordList;

    return hilite;
  }
}}
