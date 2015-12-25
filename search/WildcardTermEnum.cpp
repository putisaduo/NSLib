#include "StdHeader.h"

#include "WildcardTermEnum.h"

using namespace NSLib::index;
namespace NSLib { namespace search {

  bool WildcardTermEnum::termCompare(Term* term) {
    if ( term!=NULL && stringCompare(field, term->Field())==0 ) {
      const char_t* searchText = term->Text();
      //if ( stringFind(searchText,pre)==searchText ) {
      if ( stringFind(searchText,pre) == 0 ) {
        return wildcardEquals(text, 0, searchText, preLen);
      }
    }
    endEnum = true;
    return false;
  }
  
  /** Creates new WildcardTermEnum */
  WildcardTermEnum::WildcardTermEnum(IndexReader& reader, Term* term):
      FilteredTermEnum(reader,term),
      field ( stringDuplicate(term->Field()) ),
      _text ( stringDuplicate(term->Text()) ),
      fieldMatch(false),
      endEnum(false),
      preLen(0),
      pre ( stringDuplicate(term->Text()) )
  {
    text = _text;
    searchTerm = term->pointer();
      
    const char_t* sidx = stringFindChar( text, WILDCARD_STRING );
    const char_t* cidx = stringFindChar( text, WILDCARD_CHAR );
    const char_t* tidx = sidx;
    if (sidx == NULL)
      tidx = cidx;
    else if (cidx-text >= 0)
      tidx = min(tidx, cidx);

    int idx = (int)(tidx - text);

    pre[idx]=0; //trim end
    preLen = (int)stringLength(pre);
    text += preLen;

    Term* t = new Term(searchTerm->Field(), pre);
    setEnum( &reader.getTerms( t ));
    t->finalize();
  }

  WildcardTermEnum::~WildcardTermEnum(){
  }
  
  float WildcardTermEnum::difference() {
    return 1.0f;
  }
  
  bool WildcardTermEnum::EndEnum() {
    return endEnum;
  }

  /**
   * Determines if a word matches a wildcard pattern.
   * <small>Work released by Granta Design Ltd after originally being done on
   * company time.</small>
   */
  bool WildcardTermEnum::wildcardEquals(const char_t* pattern, int patternIdx, const char_t* str, int stringIdx)
  {
    for (int p = patternIdx; ; ++p) {
      for (int s = stringIdx; ; ++p, ++s) {
        // End of str yet?
        bool sEnd = (s >= stringLength(str));
        // End of pattern yet?
        bool pEnd = (p >= stringLength(pattern));

        // If we're looking at the end of the str...
        if (sEnd) {
          // Assume the only thing left on the pattern is/are wildcards
          bool justWildcardsLeft = true;

          // Current wildcard position
          int wildcardSearchPos = p;
          // While we haven't found the end of the pattern,
          // and haven't encountered any non-wildcard characters
          while (wildcardSearchPos < stringLength(pattern) && justWildcardsLeft) {
            // Check the character at the current position
            char_t wildchar = pattern[wildcardSearchPos];
            // If it's not a wildcard character, then there is more
            // pattern information after this/these wildcards.
            if (wildchar != WildcardTermEnum::WILDCARD_CHAR && wildchar != WILDCARD_STRING)
              justWildcardsLeft = false;
            else
              // Look at the next character
              wildcardSearchPos++;
          }

          // This was a prefix wildcard search, and we've matched, so
          // return true.
          if (justWildcardsLeft)
            return true;
        }

        // If we've gone past the end of the str, or the pattern,
        // return false.
        if (sEnd || pEnd)
          break;

              // Match a single character, so continue.
        if (pattern[p] == WildcardTermEnum::WILDCARD_CHAR)
          continue;

        //
        if (pattern[p] == WILDCARD_STRING) {
          // Look at the character beyond the '*'.
          ++p;
          // Examine the str, starting at the last character.
          for (int i = stringLength(str); i >= s; --i) {
            if (wildcardEquals(pattern, p, str, i))
              return true;
          }
          break;
        }
        if (pattern[p] != str[s])
          break;
      }
      return false;
    }
  }

  void WildcardTermEnum::close()
  {
    FilteredTermEnum::close();
    
    searchTerm->finalize();
    searchTerm = NULL;
    
    delete[] field;
    delete[] _text;
    
    delete[] pre;
    searchTerm->finalize();
  }
}}
