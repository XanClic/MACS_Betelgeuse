/**
 * @file macs.hpp
 *
 * Main include file. Contains all publicly visible data structures and
 * functions.
 */

#ifndef MACS_HPP
#define MACS_HPP

#include <cstdint>
#include <exception>
#include <initializer_list>
#include <vector>

#include "macs-internals.hpp"


/**
 * MACS namespace. Contains publicly visible data structures and functions.
 */
namespace macs
{
    /**
     * Initialises the environment. This function requires a proper OpenGL
     * context to be set up. Afterwards you may not do any OpenGL operations,
     * otherwise, the result is undefined. The width and height specified will
     * be the one used for every texture and similar formats.
     *
     * @param width Fundamental width
     * @param height Fundamental height
     *
     * @return true iff the environment is suitable and initialised.
     */
    bool init(int width, int height);


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



    /**
     * Base class for many other classes.
     *
     * This class is the main class for many other classes which is used when
     * accepting several objects of different type together (e.g. in
     * initializer lists).
     */
    class root
    {
        public:
            /// Derived classes
            enum types
            {
                /**
                 * Basic 2D texture.
                 *
                 * @sa texture
                 */
                texture,

                /**
                 * Array of 2D textures.
                 *
                 * @sa texture_array
                 */
                texture_array,

                /**
                 * Combined stencil/depth buffer.
                 *
                 * @sa stencildepth
                 */
                stencildepth
            };

            /// Derived class type
            types type;
    };



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
    class texture: public root
    {
        public:
            /**
             * Creates an empty texture. Note that its content as well as its
             * dimensions will not be defined, until you write data to it or
             * use the allocate() method. The texture's size will be the one
             * specified during the <tt>macs::init()</tt> call.
             *
             * @param name Name which is used to denote this texture in scripts
             *
             * @sa void texture::allocate(void)
             */
            texture(const char *name);

            /**
             * Destroys a texture.
             */
            ~texture(void);


            /**
             * Actually creates the texture. This function physically allocates
             * the texture memory and also sets its dimensions, leaving the
             * content undefined. This function is supposed to be called when
             * creating a texture which is used for writing to. If the texture
             * already has been allocated (via <tt>allocate()</tt> or
             * <tt>write()</tt>), this is a no-op.
             *
             * @sa void texture::write(formats::f0123 *src)
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


            /// Texture name
            const char *name;


            friend class render;
            friend class internals::tmu;

        private:
            /// OpenGL texture ID
            GLuint id;

            /// True iff the texture's dimensions have been set.
            bool allocated;
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
    class texture_array: public root
    {
        public:
            /**
             * Creates an empty texture array. As with normal textures, its
             * contents and dimensions are undefined until allocated or
             * written to.
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
             * Allocates the texture array.
             *
             * @sa void texture::allocate(void)
             */
            void allocate(void);

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


            /// Texture array name
            const char *name;


            friend class render;
            friend class internals::tmu;

        private:
            /// Subtexture count
            int elements;

            /// OpenGL texture ID
            GLuint id;

            /// True iff the texture's dimensions have been set.
            bool allocated;
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
    class stencildepth: public root
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



    /**
     * Represents a render pass.
     *
     * A render pass is the core of MACS. It takes textures as input and output
     * and a script which describes how to process the input data and what to
     * write to the output.
     *
     * Physically, a render pass is described by a framebuffer object.
     */
    class render
    {
        public:
            /**
             * Creates a new render pass object.
             */
            render(void);

            /**
             * Frees a render pass object.
             */
            ~render(void);


            /**
             * Attaches several objects (i.e., textures) as input to this render
             * pass object. The given objects will be attached and may
             * subsequently be read from in the render pass script.
             *
             * @param objs List of objects to be attached as input.
             *
             * @note Note that no object may be attached both as input and
             *       output.
             */
            void input(std::initializer_list<root *> objs);
            /// @overload void render::input(std::initializer_list<root *> objs)
            void input(root *obj)
            { this->input({ obj }); }

            /**
             * Alias for <tt>input()</tt> and provided for convenience only.
             * @sa void render::input(std::initializer_list<root *> objs)
             */
            void operator<<(std::initializer_list<root *> objs)
            { this->input(objs); }
            /// @overload void render::operator<<(std::initializer_list<root *> objs)
            void operator<<(root *obj)
            { this->input(obj); }


            /**
             * Attaches several textures as output to this render pass object.
             * The given textures will be attached and may subsequently be
             * written to in the render pass script.
             *
             * @param objs List of textures to be attached as input.
             *
             * @note Note that no object may be attached both as input and
             *       output.
             *
             * @note Note further that only plain 2D textures and stencildepth
             *       buffers (only one at a time) may be used as output from
             *       render passes.
             */
            void output(std::initializer_list<root *> objs);
            /// @overload void render::output(std::initializer_list<root *> objs)
            void output(root *obj)
            { this->output({ obj }); }

            /**
             * Alias for <tt>output()</tt> and provided for convenience only.
             * @sa void render::output(std::initializer_list<root *> objs)
             */
            void operator>>(std::initializer_list<root *> objs)
            { this->output(objs); }
            /// @overload void render::operator>>(std::initializer_list<root *> objs)
            void operator>>(root *obj)
            { this->output(obj); }


            /**
             * Binds this render object for usage. Before doing anything with a
             * render pass object, you have to bind it. Only one render pass
             * object may be bound at a time, and
             * <tt>macs::render_to_screen()</tt> destroys all those bindings.
             *
             * @sa void render_to_screen(void)
             */
            void bind(void);


            /**
             * Specifies the render pass script (RPS). This specifies the
             * computation to be carried out upon rendering. This script will
             * not be translated/compiled until execution, thus every syntax
             * error (etc.) will not be detected until then.
             * Furthermore, when calling this function, every input and
             * output object will be attached to this frame buffer. Thus,
             * you have to call this function after specifying all the
             * inputs and outputs.
             *
             * @param src Render pass script source code
             *
             * @return True iff successful
             *
             * @note Right now, there is no translation from RPS to GLSL, it
             *       will simply be piped through (with several lines added
             *       before). Thus, the RPS simply has to be a valid GLSL
             *       program (however, the attached objects will already be
             *       defined). This may be changed in future, though.
             *
             * @note It may be impossible to find a correct distribution of
             *       input and output objects among the hardware ressources,
             *       especially, if you specify more input textures than
             *       texture units available. Try to dynamically adjust your
             *       code to fit to the units available, because MACS cannot
             *       do this for you.
             *
             * @sa int max_input_textures(void)
             */
            bool compile(const char *src);


            /**
             * Prepares this object for usage. This will modify the OpenGL
             * states so this render pass may be executed. If you want to do
             * anything which modifies the render output and which should be
             * executed before the rendering, use it between the call of
             * this function and the call of the <tt>execute()</tt> function.
             * Such modifications include clearing output buffers.
             *
             * @sa void render::execute(void)
             */
            void prepare(void);


            /**
             * Executes this render pass. You have to call the
             * <tt>prepare()</tt> function before doing so (of course, the
             * render pass object must be compiled before).
             *
             * @sa void render::prepare(void)
             *
             * @note Since the attached ressources are distributed among the
             *       limited hardware ressources, it might be neccessary to do
             *       more than one physical render pass, especially if you use
             *       more output textures than physically allowed.
             *
             * @sa int max_output_textures(void)
             */
            void execute(void);


            /**
             * Clear the attached output buffers. Sets all fragments of all
             * attached output textures to the given value.
             *
             * @param value Value the textures should be set to.
             */
            void clear_output(formats::f0123 value);


            /**
             * Stencil/depth buffer comparison function. Iff
             * "<tt>new value <op> old value</tt>" is true, the output is
             * altered.
             */
            enum comparison
            {
                /// new < old
                less = GL_LESS,
                /// new <= old
                less_or_equal = GL_LEQUAL,
                /// new > old
                greater = GL_GREATER,
                /// new >= old
                greater_or_equal = GL_GEQUAL,
                /// new == old
                equal = GL_EQUAL,
                /// new != old
                not_equal = GL_NOTEQUAL,
                /// Never alter output
                never = GL_NEVER,
                /// Always alter output
                always = GL_ALWAYS
            };

            /**
             * Enables or disables depth testing. If enabling, also sets the
             * comparison function.
             *
             * @param dt Enables depth testing iff true, else disables it.
             * @param comp Comparison function; defaults to pass::less. The
             *             current function will not be altered if dt is false.
             */
            void use_depth(bool dt, comparison comp = less);

            /**
             * Clear the attached depth buffer. Sets all fragments of the
             * attached depth buffer to the given value.
             *
             * @param value Value the depth buffer should be set to.
             */
            void clear_depth(formats::f0 value);

            /**
             * Stencil operations. Specifies how the buffer should be changed
             * upon a certain event.
             */
            enum stencil_op
            {
                /// Keep current value
                keep = GL_KEEP,
                /// Set value to zero
                clear = GL_ZERO,
                /// Set to the current reference value
                replace = GL_REPLACE,
                /// Saturated increment
                inc_sat = GL_INCR,
                /// Wrapped increment
                inc_wrap = GL_INCR_WRAP,
                /// Saturated decrement
                dec_sat = GL_DECR,
                /// Wrapped decrement
                dec_wrap = GL_DECR_WRAP,
                /// Binary invert
                invert = GL_INVERT
            };

            /**
             * Enables or disables stencil testing. If enabling, also sets the
             * comparison function.
             *
             * @param st Enables stencil testing iff true, else disables it.
             * @param comp Comparison function; defaults to pass::not_equal. The
             *             current function will not be altered if dt is false.
             */
            void use_stencil(bool st, comparison comp = not_equal);

            /**
             * Clear the attached stencil buffer. Sets all fragments of the
             * attached stencil buffer to the given value.
             *
             * @param value Value the stencil buffer should be set to.
             */
            void clear_stencil(uint8_t value);

            /**
             * Sets the stencil operations. Specifies what is to be done when
             * certain tests passes and others fail.
             *
             * @param sf Specifies the operation for when the stencil test
             *           fails (default: pass::keep).
             * @param df Specifies the operation for when the stencil test
             *           passes, but the depth test fails (default: pass::keep).
             * @param dp Specifies the operation for when both the stencil and
             *           the depth test pass (default: pass::replace).
             */
            void stencil_operation(stencil_op sf, stencil_op df, stencil_op dp);

            /**
             * Sets the stencil reference value and mask.
             *
             * @param ref This value is used in comparisons as the one the
             *            current stencil value is compared to (default: 0xFF).
             * @param mask This mask is applied to every stencil operation,
             *             including comparisons (default: 0xFF).
             */
            void stencil_values(uint8_t ref, uint8_t mask);


        private:
            /// OpenGL FBO ID
            GLuint id;

            /// Depth testing enabled
            bool de;
            /// Stencil testing enabled
            bool se;

            /// Attached stencil/depth buffer
            stencildepth *sd;
            /// Depth comparison function
            comparison dcf;
            /// Stencil comparison function
            comparison scf;
            /// Stencil reference value
            uint8_t sref;
            /// Stencil mask
            uint8_t smask;
            /// Stencil operation on stencil fail
            stencil_op sosf;
            /// Stencil operation on depth fail
            stencil_op sodf;
            /// Stencil operation on depth pass
            stencil_op sodp;

            /// Input objects
            std::vector<root *> inp_objs;
            /// Output objects
            std::vector<root *> out_objs;

            /// Generated program
            internals::program *prg;
    };


    /**
     * Allows rendering to screen. You may want to display your result on
     * screen, e.g. via the <tt>texture::display()</tt> function. This function
     * enables onscreen rendering, after offscreen computations have been
     * carried out.
     */
    void render_to_screen(void);



    /**
     * MACS exceptions.
     */
    namespace exc
    {
        /**
         * Invalid type exception. Thrown if an invalid type was given to
         * a function (e.g., a stencil/depth buffer as input to a render pass
         * object).
         */
        static class invalid_type: std::exception
        {
            public:
                /// Returns an error description.
                virtual const char *what(void) const throw()
                { return "Invalid type given."; }
        }
        /// Invalid type exception instance
        inv_type;

        /**
         * Invalid execution order exception. Thrown if certain methods have
         * been called in the wrong order on an object (e.g., if you attach
         * objects to a render pass object after it has been compiled).
         */
        static class invalid_execution_order: std::exception
        {
            public:
                /// Returns an error description.
                virtual const char *what(void) const throw()
                { return "Methods executed in invalid order."; }
        }
        /// Invalid execution order exception instance
        inv_exec_order;

        /**
         * Hardware ressource limit exceeded exception. Thrown if the logical
         * objects cannot be distributed among the available physical hardware
         * ressources (e.g., more input textures than texture units available).
         */
        static class ressource_limit_exceeded: std::exception
        {
            public:
                /// Returns an error description.
                virtual const char *what(void) const throw()
                { return "Physical ressource limit has been exceeded."; }
        }
        /// Hardware ressource limit exceeded exception instance
        rsrc_lim_exc;

        /**
         * Texture not assigned exception. Thrown if a texture should be used as
         * input to a computation, but has not yet been assigned to a texture
         * unit. This should never occur, however.
         */
        static class texture_not_assigned: std::exception
        {
            public:
                /// Returns an error description.
                virtual const char *what(void) const throw()
                { return "Requested texture has not been assigned to a TMU."; }
        }
        /// Texture not assigned exception instance
        tex_na;
    }
}

#endif
