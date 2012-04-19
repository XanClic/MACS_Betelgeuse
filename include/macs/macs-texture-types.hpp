/**
 * @file macs-texture-types.hpp
 *
 * Contains several texture types. These are used as big vectors to do SIMD
 * (or even MIMD) calculations on in render passes.
 */

#ifndef MACS_TEXTURE_TYPES_HPP
#define MACS_TEXTURE_TYPES_HPP

#include "macs-internals.hpp"
#include "macs-root.hpp"

namespace macs
{
    class render;


    /**
     * Represents the basic data structure.
     *
     * Textures are data buffers which you may write to, read from and which are
     * used by all MACS computations, both as input and output.
     *
     * Textures are two-dimensional. They are created with the width and heigt
     * specified upon calling <tt>macs::init()</tt>. The resulting buffer has
     * <tt>width * height</tt> elements per channel, one texture has four
     * channels (R, G, B, A). When writing or reading data, all four channels
     * are affected.
     *
     * Physically, they represent a OpenGL textures, obviously. Thus, it is
     * preferable to use squared textures and powers of two as dimensions (i.e.,
     * 1024 x 1024 for 1e6 elements).
     */
    class texture: public textures_in, public textures_out
    {
        public:
            /**
             * Creates an empty texture. The texture's size will be the one
             * specified during the <tt>macs::init()</tt> call.
             *
             * @param name Name which is used to denote this texture in scripts
             */
            texture(const char *name);

            /**
             * Destroys a texture.
             */
            ~texture(void);


            /**
             * Fills the texture with data. This function fills the texture
             * memory with the data contained in the given buffer.
             *
             * @param src Buffer containing the data the texture should be
             *            filled with. <tt>width * height</tt> formats::rgba
             *            objects are read from it, so it must be at least
             *            <tt>width * height * 16</tt> bytes in size.
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
             * and should be filled with data, either by a <tt>write()</tt> call
             * or as a result of a render pass.
             *
             * @param dst Buffer the data should be written to. It must be big
             *            enough to receive <tt>width * height</tt> elements.
             */
            void read(formats::f0123 *dst);
            /// @overload void texture::read(formats::f0123 *dst)
            void read(formats::f2103 *dst);
            /// @overload void texture::read(formats::f0123 *dst)
            void read(formats::f012 *dst);
            /// @overload void texture::read(formats::f0123 *dst)
            void read(formats::f210 *dst);
            /// @overload void texture::read(formats::f0123 *dst)
            void read(formats::f0 *dst);


            /**
             * Displays this texture's contents. This will draw a quad spanning
             * the whole screen with the texture on it.
             */
            void display(void);


            friend class render;
            friend class internals::tmu;

        private:
            /// OpenGL texture ID
            GLuint id;
    };


    /**
     * Represents an array of textures.
     *
     * If you want to use many textures at once which contain similar data, you
     * may consider using an array of textures. This class defines such
     * data structures.
     *
     * Physically, they represent 3D textures where the R coordinate is used to
     * index a 2D texture.
     */
    class texture_array: public textures_in
    {
        public:
            /**
             * Creates an empty texture array.
             *
             * @param name Name which is used to denote this array in scripts
             * @param textures Texture count
             */
            texture_array(const char *name, int textures);

            /**
             * Destroys a texture array.
             */
            ~texture_array(void);


            /**
             * Fills the texture array with data.
             *
             * @param src <tt>width * height * textures</tt> formats::rgba
             *            objects are read from this buffer.
             *
             * @sa void texture::write(formats::f0123 *src)
             */
            void write(const formats::f0123 *src);
            /// @overload void texture_array::write(formats::f0123 *src)
            void write(const formats::f2103 *src);
            /// @overload void texture_array::write(formats::f0123 *src)
            void write(const formats::f012 *src);
            /// @overload void texture_array::write(formats::f0123 *src)
            void write(const formats::f210 *src);
            /// @overload void texture_array::write(formats::f0123 *src)
            void write(const formats::f0 *src);

            /**
             * Reads data from a texture array.
             *
             * @param dst <tt>width * height * textures</tt> elements are
             *            written to this buffer.
             *
             * @sa void texture::read(formats::f0123 *dst)
             */
            void read(formats::f0123 *dst);
            /// @overload void texture_array::read(formats::f0123 *dst)
            void read(formats::f2103 *dst);
            /// @overload void texture_array::read(formats::f0123 *dst)
            void read(formats::f012 *dst);
            /// @overload void texture_array::read(formats::f0123 *dst)
            void read(formats::f210 *dst);
            /// @overload void texture_array::read(formats::f0123 *dst)
            void read(formats::f0 *dst);


            friend class render;
            friend class internals::tmu;

        private:
            /// Subtexture count
            int elements;

            /// OpenGL texture ID
            GLuint id;
    };


    /**
     * Represents a combined depth/stencil buffer attachment.
     *
     * A render pass may include a stencil. This is basically an additional
     * one-channel write-only texture which is however not accessible in the
     * pass script and which is accessed bitwise. When a fragment passes the
     * stencil (and depth) test, the buffer may be modified. Else, all buffers
     * remain unaltered.
     *
     * The stencil test compares a reference value with the value already
     * present in the stencil buffer. An arbitrary mask is applied to every
     * stencil operation, including these comparisons.
     *
     * Furthermore, a render pass may include a depth buffer. This is as well
     * basically an additional one-chennel write-only texture which one refers
     * to using the "depth" identifier in pass scripts. The written depth value
     * is then compared using one of several functions to the value it already
     * contains. If the comparison is false, the output textures will not be
     * altered.
     *
     * The stencil test is done before the depth test.
     *
     * Note that some functions one would normally expect to be contained in
     * this class may actually be part of the render pass class. Currently this
     * affects the functions used to clear the buffers.
     *
     * @sa pass::clear_stencil(uint8_t value)
     * @sa pass::clear_depth(formats::f0 value)
     */
    class stencildepth: public textures_out
    {
        public:
            /**
             * Creates a stencil/depth buffer.
             */
            stencildepth(void);
            /**
             * Frees a stencil/depth buffer.
             */
            ~stencildepth(void);

            /**
             * Creates a texture from the depth part of this buffer. This
             * function transforms the depth buffer into a one-channel texture.
             * Note that it requires this buffer to be bound to an active render
             * pass object.
             *
             * @param name The name of the texture to be created.
             *
             * @return The one-channel texture created.
             *
             * @note This function is relatively CPU-heavy, because it converts
             *       the combined stencil/depth buffer data to sole depth FP
             *       values before transferring them to a new texture. Thus it
             *       should be used for debugging purposes only.
             */
            class texture *depth_to_texture(const char *name);


            friend class render;

        private:
            /// OpenGL buffer ID
            GLuint id;
    };
}

#include "macs.hpp"

#endif
