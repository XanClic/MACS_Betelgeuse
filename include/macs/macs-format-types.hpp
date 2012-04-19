/**
 * @file macs-format-types.hpp
 *
 * Contains color formats classes.
 */

#ifndef MACS_FORMAT_TYPES_HPP
#define MACS_FORMAT_TYPES_HPP

namespace macs
{
    /**
     * Namespace for buffer formats. A buffer format specifies how many channels
     * are used when transferring data from or to a texture and in which order
     * they appear in the buffer.
     */
    namespace formats
    {
        /**
         * Defines a color format.
         *
         * @param name Name, e.g., rgba, bgra or rgb
         * @param channels number of channels, e.g., 4 for rgba or 3 for bgr
         * @param ... RGBA component order
         */
        #define create_color_format(name, channels, ...) \
            struct name \
            { \
                public: \
                    /** Non-initializing constructor. */ \
                    name(void) {} \
                    /** Constructor initializing based on an initializer list. */ \
                    name(std::initializer_list<float> init) \
                    { int i = 0; for (float v: init) { _f[i++] = v; } } \
                    /** Delegates indexing accesses to _f[]. */ \
                    inline float &operator[](int index) \
                    { return _f[index]; } \
                    \
                    union \
                    { \
                        /** Channels in RGBA convention. */ \
                        struct { float __VA_ARGS__; }; \
                        \
                        /** Channels as an array */ \
                        float _f[channels]; \
                    }; \
            }

        /**
         * RGBA color format. Standard 0123 color format with all four channels.
         */
        create_color_format(f0123, 4, r, g, b, a);
        /**
         * BGRA color format. 2103 color format, which is however not too rarely
         * used.
         */
        create_color_format(f2103, 4, b, g, r, a);
        /**
         * RGB color format. 012 color format, if you don't need the fourth channel.
         */
        create_color_format(f012, 3, r, g, b);
        /**
         * BGR color format. 210 color format, if you don't need alpha but want to
         * use BGR instead of RGB.
         */
        create_color_format(f210, 3, b, g, r);
        /**
         * R color format. 0 color format, if you need one channel only.
         */
        create_color_format(f0, 1, r);


        #undef create_color_format
    }
}

#endif
