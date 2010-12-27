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

#include "ptlsim-stl.h"

/** @ todo
char* operator char* &() (stringbuf& os, const char* v) {
  return os.str().c_str();
}
*/

int format_float(char* buf, int bufsize, double v, int precision, int pad) {
  char format[32];
  snprintf(format, sizeof(format), "%%.%df", precision);
  snprintf(buf, bufsize, format, v);
  return strlen(buf);
}

char* format_number(char* buf, char* end, W64 num, int base, int size, int precision, int type) {
	char c,sign,tmp[66];
	const char *digits;
	static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	digits = (type & FMT_LARGE) ? large_digits : small_digits;
	if (type & FMT_LEFT)
		type &= ~FMT_ZEROPAD;
	if (base < 2 || base > 36)
		return NULL;
	c = (type & FMT_ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & FMT_SIGN) {
		if ((signed long long) num < 0) {
			sign = '-';
			num = - (signed long long) num;
			size--;
		} else if (type & FMT_PLUS) {
			sign = '+';
			size--;
		} else if (type & FMT_SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & FMT_SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0)
		tmp[i++] = digits[do_div(num,base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(FMT_ZEROPAD+FMT_LEFT))) {
		while(size-->0) {
			if (buf <= end)
				*buf = ' ';
			++buf;
		}
	}
	if (sign) {
		if (buf <= end)
			*buf = sign;
		++buf;
	}
	if (type & FMT_SPECIAL) {
		if (base==8) {
			if (buf <= end)
				*buf = '0';
			++buf;
		} else if (base==16) {
			if (buf <= end)
				*buf = '0';
			++buf;
			if (buf <= end)
				*buf = digits[33];
			++buf;
		}
	}
	if (!(type & FMT_LEFT)) {
		while (size-- > 0) {
			if (buf <= end)
				*buf = c;
			++buf;
		}
	}
	while (i < precision--) {
		if (buf <= end)
			*buf = '0';
		++buf;
	}
	while (i-- > 0) {
		if (buf <= end)
			*buf = tmp[i];
		++buf;
	}
	while (size-- > 0) {
		if (buf <= end)
			*buf = ' ';
		++buf;
	}
	return buf;
}


int format_integer(char* buf, int bufsize, W64s v, int size, int flags, int base, int precision) {
  if (size < 0) size = bufsize-1;
  if ((v < 0) & (base == 10)) flags ^= FMT_SIGN;
  char* end = format_number(buf, buf + bufsize - 2, v, base, min(bufsize-1, size), precision, flags);
  // null terminate
  *end = 0;
  return (end - buf);
}

stringbuf& operator <<(stringbuf& os, const hexstring& hs) {
  static const char* hexdigits = "0123456789abcdef";
  char buf[128+1];
  assert(hs.n <= 64);

  int n = ceil(hs.n, 4) / 4;

  W64 v = hs.value;
  int j = n-1;
  buf[n] = 0;

  while (j >= 0) {
    buf[j--] = hexdigits[v & 0xf];
    v >>= 4;
  }

  os << buf;
  return os;
}
 
stringbuf& operator <<(stringbuf& os, const padstring& s) {
  
  int len = strlen(s.value); 
  bool leftalign = (s.width < 0);
  int width = (s.width) ? abs(s.width) : len;
  width = max(width - len, 0);
  
  //int reqbytes = max(len, width) + 1;
  //os.reserve(reqbytes);
  
  if (leftalign) {
    os.sputn(s.value, len);
    for (int i = 0; i < width; i++){
      os.sputc(' ');
    }
    //memcpy(os.p, s.value, len);
    //os.p += len;
    //memset(os.p, ' ', width);
    //os.p += width;
  } else {
    for (int i = 0; i < width; i++){
      os.sputc(' ');
    }
    os.sputn(s.value, len);
    //memset(os.p, ' ', width);
    //os.p += width;
    //memcpy(os.p, s.value, len);
    //os.p += len;
  }
  //*os.p = 0;
  os.sputc(0);
  
  return os;
}

stringbuf& operator <<(stringbuf& os, const char* v) {
  if unlikely (!v) v = "<null>";
  int bytes = strlen(v);
  //os.reserve(bytes);
  //memcpy(os.p, v, bytes);
  //os.p += bytes - 1;
  os.sputn(v, bytes);
  return os;
}

stringbuf& operator <<(stringbuf& os, char v) {
  //os.reserve(2);
  //os.p[0] = v;
  //os.p[1] = 0;
  //os.p++;
  os.sputc(v);
  return os;
}

stringbuf& operator <<(stringbuf& os, const bitstring& bs) {
  if (bs.reverse) {
    // LSB first:
    for (int i = 0; i < bs.n; i++) os << (char)(bit(bs.bits, i) + '0');
  } else {
    // MSB first (default):
    for (int i = bs.n-1; i >= 0; i--) os << (char)(bit(bs.bits, i) + '0');
  }
    
  return os;
}

stringbuf& operator <<(stringbuf& os, const bitmaskstring& bs) {
  if (bs.reverse) {
    // LSB first:
    for (int i = 0; i < bs.n; i++) os << (char)(bit(bs.mask, i) ? (bit(bs.bits, i) + '0') : 'x');
  } else {
    // MSB first (default):
    for (int i = bs.n-1; i >= 0; i--) os << (char)(bit(bs.mask, i) ? (bit(bs.bits, i) + '0') : 'x');
  }

  return os;
}

  
stringbuf& operator <<(stringbuf& os, const bytestring& bs) {
  foreach (i, bs.n) {
    os << hexstring(bs.bytes[i], 8);
    if (((i % bs.splitat) == (bs.splitat-1)) && (i != bs.n-1)) 
      os << '\n'; 
    else if (i != bs.n-1)
      os << " ";
  }

  return os;
}

stringbuf& operator <<(stringbuf& os, const bytemaskstring& bs) {
  foreach (i, bs.n) {
    if (bit(bs.mask, i))
      os << hexstring(bs.bytes[i], 8);
    else os << "XX";
    if (((i % bs.splitat) == (bs.splitat-1)) && (i != bs.n-1)) 
      os << '\n'; 
    else if (i != bs.n-1)
      os << " ";
  }

  return os;
}

stringbuf& operator <<(stringbuf& os, const intstring& is) {

  char buf[128];
  int len = format_integer(buf, sizeof(buf), is.value);
  bool leftalign = (is.width < 0);
  int width = (is.width) ? abs(is.width) : len;
  width = max(width - len, 0);

  //int reqbytes = max(len, width) + 1;
  //os.reserve(reqbytes);

  if (leftalign) {
    os.sputn(buf, len);
    for (int i = 0; i < width; i++){
      os.sputc(' ');
    }
    //memcpy(os.p, buf, len);
    //os.p += len;
    //memset(os.p, ' ', width);
    //os.p += width;
  } else {
    for (int i = 0; i < width; i++){
      os.sputc(' ');
    }
    os.sputn(buf, len);
    //memset(os.p, ' ', width);
    //os.p += width;
    //memcpy(os.p, buf, len);
    //os.p += len;
  }
  //*os.p = 0;
  os.sputc(0);

  return os;
}

stringbuf& operator <<(stringbuf& os, const floatstring& fs) {

  char buf[128];
  int len = format_float(buf, sizeof(buf), fs.value, fs.precision);
  bool leftalign = (fs.width < 0);
  int width = (fs.width) ? abs(fs.width) : len;

  //int reqbytes = max(len, width) + 1;
  //os.reserve(reqbytes);

  width = max(width - len, 0);
  
  if (leftalign) {
    os.sputn(buf, len);
    for (int i = 0; i < width; i++){
      os.sputc(' ');
    }
    //memcpy(os.p, buf, len);
    //os.p += len;
    //memset(os.p, ' ', width);
    //os.p += width;
  } else {
    for (int i = 0; i < width; i++){
      os.sputc(' ');
    }
    os.sputn(buf, len);
    //memset(os.p, ' ', width);
    //os.p += width;
    //memcpy(os.p, buf, len);
    //os.p += len;
  }
  //*os.p = 0;
  os.sputc(0);

  return os;
}

stringbuf& operator <<(stringbuf& os, const substring& s) {
  //os.reserve(s.length + 1);
  //memcpy(os.p, s.str, s.length);
  //os.p += s.length;
  //*os.p = 0;
  os.sputn(s.str, s.length);
  os.sputc(0);

  return os;
}

