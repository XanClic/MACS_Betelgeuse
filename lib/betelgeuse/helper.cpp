#ifdef __WIN32
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

void asprintf(char **dst, const char *format, ...)
{
    va_list va_1, va_2;

    va_start(va_1, format);
    va_copy(va_2, va_1);

    int sz = vsnprintf(NULL, 0, format, va_1);

    if (sz <= 0)
    {
        // FIXME would be wrong, since there is nothing to fix but Windows
        sz = 65535;
    }

    *dst = new char[sz + 1];

    vsnprintf(*dst, sz + 1, format, va_2);

    va_end(va_1);
    va_end(va_2);
}
#endif
