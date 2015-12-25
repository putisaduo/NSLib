#ifndef _NSLib_util_enc_
#define _NSLib_util_enc_

enum Encoding {
    UNKNOWN = -1,
    ISO8859_1 = 0,
    GB2312 = 1,
    BIG5 = 2,
    NUM_ENC
};

#ifdef _WIN32
 #define LANG_ENG "English"
 #define LANG_CHT "cht"
 #define LANG_CHS "chs"
#elif __GNUC__
 #define LANG_ENG "en_US.UTF8"
 #define LANG_CHT "zh_TW.BIG5"
 #define LANG_CHS "zh_CN.GB2312"
#endif

#endif
