// ----------------------------------------------------------------------
//
//  This file is part of FeS2.
//
//  FeS2 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  FeS2 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with FeS2.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------

#ifndef __YORST_STL_H
#define __YORST_STL_H

#include <iostream>
#include <sstream>

#include "globals.h"

//
// Formatting
//
#define FMT_ZEROPAD 1 /* pad with zero */
#define FMT_SIGN    2 /* unsigned/signed long */
#define FMT_PLUS    4 /* show plus */
#define FMT_SPACE   8 /* space if plus */
#define FMT_LEFT	  16 /* left justified */
#define FMT_SPECIAL	32 /* 0x */
#define FMT_LARGE	  64 /* use 'ABCDEF' instead of 'abcdef' */

//
// Division functions
//
#ifdef __x86_64__

#define do_div(n,base) ({                                       \
        W32 __base = (base);                            \
        W32 __rem;                                              \
        __rem = ((W64)(n)) % __base;                    \
        (n) = ((W64)(n)) / __base;                              \
        __rem;                                                  \
 })

#else

#define do_div(n,base) ({ \
	W32 __upper, __low, __high, __mod, __base; \
	__base = (base); \
	asm("":"=a" (__low), "=d" (__high):"A" (n)); \
	__upper = __high; \
	if (__high) { \
		__upper = __high % (__base); \
		__high = __high / (__base); \
	} \
	asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (__base), "0" (__low), "1" (__upper)); \
	asm("":"=A" (n):"a" (__low),"d" (__high)); \
	__mod; \
})

#endif

#define DeclareStringBufToStream(T) inline ostream& operator <<(ostream& os, const T& arg) { std::stringbuf sb; sb << arg; os << sb.str(); return os; }

  // Print bits as a string:
  struct bitstring {
    W64 bits;
    int n;
    bool reverse;
    
    bitstring() { }
    
    bitstring(const W64 bits, const int n, bool reverse = false) {
      assert(n <= 64);
      this->bits = bits;
      this->n = n;
      this->reverse = reverse;
    }
  };

  std::stringbuf& operator <<(std::stringbuf& os, const bitstring& bs);

  DeclareStringBufToStream(bitstring);

  struct bitmaskstring {
    W64 bits;
    W64 mask;
    int n;
    bool reverse;
    
    bitmaskstring() { }
    
    bitmaskstring(const W64 bits, W64 mask, const int n, bool reverse = false) {
      assert(n <= 64);
      this->bits = bits;
      this->mask = mask;
      this->n = n;
      this->reverse = reverse;
    }
  };
  
  std::stringbuf& operator <<(std::stringbuf& os, const bitmaskstring& bs);

  DeclareStringBufToStream(bitmaskstring);

  struct hexstring {
    W64 value;
    int n;
    
    hexstring() { }
    
    hexstring(const W64 value, const int n) {
      this->value = value;
      this->n = n;
    }
  };
  
  std::stringbuf& operator <<(std::stringbuf& os, const hexstring& hs);

  DeclareStringBufToStream(hexstring);

  struct bytestring {
    const byte* bytes;
    int n;
    int splitat;

    bytestring() { }

    bytestring(const byte* bytes, int n, int splitat = 16) {
      this->bytes = bytes;
      this->n = n;
      this->splitat = splitat;
    }
  };
  
  std::stringbuf& operator <<(std::stringbuf& os, const bytestring& bs);

  DeclareStringBufToStream(bytestring);

  struct bytemaskstring {
    const byte* bytes;
    W64 mask;
    int n;
    int splitat;

    bytemaskstring() { }

    bytemaskstring(const byte* bytes, W64 mask, int n, int splitat = 16) {
      assert(n <= 64);
      this->bytes = bytes;
      this->mask = mask;
      this->n = n;
      this->splitat = splitat;
    }
  };
  
  std::stringbuf& operator <<(std::stringbuf& os, const bytemaskstring& bs);

  DeclareStringBufToStream(bytemaskstring);

  struct intstring {
    W64s value;
    int width;

    intstring() { }

    intstring(W64s value, int width) {
      this->value = value;
      this->width = width;
    }
  };

  std::stringbuf& operator <<(std::stringbuf& os, const intstring& is);

  DeclareStringBufToStream(intstring);

  struct floatstring {
    double value;
    int width;
    int precision;
    
    floatstring() { }

    floatstring(double value, int width = 0, int precision = 6) {
      this->value = value;
      this->width = width;
      this->precision = precision;
    }
  };
  
  std::stringbuf& operator <<(std::stringbuf& os, const floatstring& fs);

  DeclareStringBufToStream(floatstring);

  struct padstring {
    const char* value;
    int width;

    padstring() { }

    padstring(const char* value, int width) {
      this->value = value;
      this->width = width;
    }
  };

  std::stringbuf& operator <<(std::stringbuf& os, const padstring& s);

  DeclareStringBufToStream(padstring);

  struct substring {
    const char* str;
    int length;

    substring() { }

    substring(const char* str, int start, int length) {
      int r = strlen(str);
      this->length = min(length, r - start);
      this->str = str + min(start, r);
    }
  };

  std::stringbuf& operator <<(std::stringbuf& os, const substring& s);

  DeclareStringBufToStream(substring);

#define stringbuf_smallbufsize 256
//  class std::stringbuf;

  std::stringbuf& operator <<(std::stringbuf& os, const char* v);
  std::stringbuf& operator <<(std::stringbuf& os, const char v);
/*
  class std::stringbuf {
  public:
    std::stringbuf() { buf = null; reset(); }
    std::stringbuf(int length) {
      buf = null;
      reset(length);
    }

    void reset(int length = std::stringbuf_smallbufsize);

    ~std::stringbuf();

    int remaining() const {
      return (buf + length) - p;
    }

    operator char*() const {
      return buf;
    }

    void resize(int newlength);

    void expand() {
      resize(length*2);
    }

    void reserve(int extra);

    int size() const { return p - buf; }
    bool empty() const { return (size() == 0); }
    bool set() const { return !empty(); }

    std::stringbuf& operator =(const char* str) {
      if unlikely (!str) {
        reset();
        return *this;
      }
      reset(strlen(str)+1);
      *this << str;
      return *this;
    }

    std::stringbuf& operator =(const std::stringbuf& str) {
      const char* s = (const char*)str;
      if unlikely (!s) {
        reset();
        return *this;
      }
      reset(strlen(s)+1);
      *this << s;
      return *this;
    }

    bool operator ==(const std::stringbuf& s) {
      return strequal((char*)(*this), (char*)s);
    }

    bool operator !=(const std::stringbuf& s) {
      return !strequal((char*)(*this), (char*)s);
    }

  public:
    char smallbuf[std::stringbuf_smallbufsize];
    char* buf;
    char* p;
    int length;
  };
*/
  //
  // Inserters
  //

char* format_number(char* buf, char* end, W64 num, int base, int size, int precision, int type);
int format_integer(char* buf, int bufsize, W64s v, int size = 0, int flags = 0, int base = 10, int precision = 0);
int format_float(char* buf, int bufsize, double v, int precision = 6, int pad = 0);

//char* operator char*() const (std::stringbuf& os, const char* v); jld

#define DefineIntegerInserter(T, signedtype) \
  static inline std::stringbuf& operator <<(std::stringbuf& os, const T v) { \
    char buf[128]; \
    format_integer(buf, sizeof(buf), ((signedtype) ? (W64s)v : (W64)v)); \
    return os << buf; \
  }

  DefineIntegerInserter(signed short, 1);
  DefineIntegerInserter(signed int, 0);
  DefineIntegerInserter(signed long, 0);
  DefineIntegerInserter(signed long long, 0);
  DefineIntegerInserter(unsigned short, 0);
  DefineIntegerInserter(unsigned int, 0);
  DefineIntegerInserter(unsigned long, 0);
  DefineIntegerInserter(unsigned long long, 0);

#define DefineFloatInserter(T, digits) \
  static inline std::stringbuf& operator <<(std::stringbuf& os, const T v) { \
    char buf[128]; \
    format_float(buf, sizeof(buf), v, digits); \
    return os << buf; \
  }

  DefineFloatInserter(float, 6);
  DefineFloatInserter(double, 16);

  static inline std::stringbuf& operator <<(std::stringbuf& os, const bool v) {
    return os << (int)v;
  }

#undef DefineInserter

#define PrintOperator(T) static inline ostream& operator <<(ostream& os, const T& obj) { return obj.print(os); }

  static inline std::stringbuf& operator <<(std::stringbuf& os, const string& s) {
    string output = os.str() + s;
    os.str(output);
    os.pubseekpos(output.length(), ios_base::out);
    return os;
  }

  static inline std::stringbuf& operator <<(std::stringbuf& os, const std::stringbuf& sb) {
    os << sb.str();
    return os;
  }
 
  template <class T>
  static inline std::stringbuf& operator <<(std::stringbuf& os, const T* v) {
    char buf[128];
    format_integer(buf, sizeof(buf), (W64)(Waddr)v, 0, FMT_SPECIAL, 16);
    return os << buf;
  }

  //
  // A much more intuitive syntax than STL provides:
  //
  template <class T>
  static inline std::stringbuf& operator ,(std::stringbuf& os, const T& v) {
    return os << v;
  }

#endif
