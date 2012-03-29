/**
 * @file macs.hpp
 *
 * Main include file. Contains all publicly visible data structures and
 * functions.
 */

#ifndef MACS_HPP
#define MACS_HPP

#include "macs-internals.hpp"


/**
 * MACS namespace. Contains publicly visible data structures and functions.
 */
namespace macs
{
    /**
     * Initialises the environment. This function requires a proper OpenGL
     * context to be set up. Afterwards you may not do any OpenGL operations,
     * otherwise, the result is undefined.
     *
     * @return true iff the environment is suitable and initialised.
     */
    bool init(void);


    /**
     * Returns the detected OpenGL version.
     *
     * @param major Major version number (e.g., 4 for OpenGL 4.2)
     * @param minor Minor version number (e.g., 2 for OpenGL 4.2)
     */
    void opengl_version(int &major, int &minor);

    /**
     * Returns the maximum number of textures allowed as output, i.e., the
     * number of draw buffers allowed.
     * You may output to more textures, if you do so, MACS will automatically
     * do multiple render passes.
     *
     * @return Maximum number of textures allowed as output per OpenGL render
     *         pass.
     */
    int max_output_textures(void);

    /**
     * Returns the maximum number of textures allowed as input, i.e., the number
     * of color buffer attachments per FBO allowed.
     * You may not use any more textures as input for calculations.
     *
     * @return That number.
     */
    int max_input_textures(void);



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



    /**
     * Represents the basic data structure.
     *
     * Textures are data buffers which you may write to, read from and which are
     * used by all MACS computations, both as input and output.
     *
     * Textures are two-dimensional. When creating them, you specify a width and
     * a height. The resulting buffer has width * height elements per channel,
     * one texture has four channels (R, G, B, A). When writing or reading data,
     * all four channels are affected.
     *
     * Physically, they represent a OpenGL textures, obviously. Thus, it is
     * preferable to use squared textures and powers of two as dimensions (i.e.,
     * 1024 x 1024 for 1e6 elements).
     */
    class texture
    {
        public:
            /**
             * Creates an empty texture. Note that its content as well as its
             * dimensions will not be defined, until you write data to it or
             * use the allocate() method.
             *
             * @param width Its width
             * @param height Its height
             *
             * @sa void texture::allocate(void)
             */
            texture(int width, int height);

            /**
             * Destroys a texture.
             */
            ~texture(void);


            /**
             * Actually creates the texture. This function physically allocates
             * the texture memory and also sets its dimensions, leaving the
             * content undefined. This function is supposed to be called when
             * creating a texture which is used for writing to. If the texture
             * already has been allocated (via allocate() or write()), this is
             * a no-op.
             *
             * @sa void texture::write(formats::rgba *src)
             */
            void allocate(void);

            /**
             * Fills the texture with data. This function physically allocates
             * the texture memory and set its dimensions, if it has not yet been
             * allocated. It is then filled with the data contained in the given
             * buffer. This method is meant to be used when the texture is
             * actually supposed to be used as a source. If you just want an
             * output texture, use the allocate() method.
             *
             * @param src Buffer containing the data the texture should be
             *            filled with. <tt>width * height</tt> formats::rgba
             *            objects are read from it, so it must be at least
             *            <tt>width * height * 16</tt> bytes in size.
             *
             * @sa void texture::allocate(void)
             */
            void write(const formats::f0123 *src);
            /// @overload void texture::write(formats::f0123 *src)
            void write(const formats::f2103 *src);
            /// @overload void texture::write(formats::f0123 *src)
            void write(const formats::f012 *src);
            /// @overload void texture::write(formats::f0123 *src)
            void write(const formats::f210 *src);
            /// @overload void texture::write(formats::f0123 *src)
            void write(const formats::f0 *src);

            /**
             * Reads data from a texture. The texture must have been allocated
             * and should be filled with data, either by a write() call or as a
             * result of a render pass.
             *
             * @param dst Buffer the data should be written to. It must be big
             *            enough to receive <tt>width * height</tt> elements.
             */
            void read(formats::f0123 *dst);
            /// @overload void texture::f0ead(formats::f0123 *dst)
            void read(formats::f2103 *dst);
            /// @overload void texture::f0ead(formats::f0123 *dst)
            void read(formats::f012 *dst);
            /// @overload void texture::f0ead(formats::f0123 *dst)
            void read(formats::f210 *dst);
            /// @overload void texture::f0ead(formats::f0123 *dst)
            void read(formats::f0 *dst);


            /**
             * @private
             *
             * Assigns this texture to the given texture unit. Must be used
             * right before the texture may be used as a sampler data source in
             * shaders. Note however, that any other method of this class may
             * leave any texture unit undefined.
             *
             * @param unit Texture unit this texture should be assigned to.
             *
             * @note Note that this function is not actually private but just
             *       marked as such because it is not meant to be called from
             *       external applications, since it is used only by the
             *       texture scheduler right before the render pass.
             */
            void _assign(int unit);


            friend class internals::prg_uniform;

        private:
            /// Texture width
            int width;
            /// Texture height
            int height;

            /// OpenGL texture ID
            unsigned id;

            /// True iff the texture's dimensions have been set.
            bool allocated;

            /**
             * Texture unit this texture has been assigned to. This must be
             * done before the texture may be used as a sampler data source in
             * shaders.
             */
            int unit;
    };
}

#endif
