//
// Created by sigsegv on 6/15/25.
//

#ifndef THEMASTER_CPPRESTSDK_H
#define THEMASTER_CPPRESTSDK_H

#ifdef FREEBSD
#include <cstdint>
#include <string>

namespace std {
    template<> struct char_traits<uint8_t> {
        using char_type  = uint8_t;
        using int_type   = unsigned int;
        using off_type   = std::streamoff;
        using pos_type   = std::streampos;
        using state_type = mbstate_t;

        static void assign(char_type& r, const char_type& a) noexcept { r = a; }
        static char_type to_char_type(int_type c) noexcept { return char_type(c); }
        static int_type  to_int_type(char_type c) noexcept { return c; }
        static bool eq(char_type a, char_type b) noexcept { return a == b; }
        static bool lt(char_type a, char_type b) noexcept { return a <  b; }
        static int compare(const char_type* s1,const char_type* s2,size_t n){
            for (; n--; ++s1, ++s2) if (!eq(*s1, *s2)) return lt(*s1,*s2)?-1:1;
            return 0;
        }
        static size_t length(const char_type* s){
            const char_type* p = s; while (*p) ++p; return size_t(p - s);
        }
        static const char_type* find(const char_type* s,size_t n,const char_type& a){
            for (; n--; ++s) if (eq(*s, a)) return s; return nullptr;
        }
        static char_type* move (char_type* r,const char_type* s,size_t n){
            return (char_type*)memmove(r, s, n);
        }
        static char_type* copy (char_type* r,const char_type* s,size_t n){
            return (char_type*)memcpy (r, s, n);
        }
        static char_type* assign(char_type* r,size_t n,char_type a){
            return (char_type*)memset(r, a, n);
        }
        static int_type  eof() noexcept { return ~0u; }
        static int_type  not_eof(int_type c) noexcept { return c == eof() ? 0 : c; }
    };
}
#endif

#endif //THEMASTER_CPPRESTSDK_H
