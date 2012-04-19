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
#include <list>


#include "macs-exceptions.hpp"
#include "macs-root.hpp"
#include "macs-internals.hpp"
#include "macs-algebraic-types.hpp"
#include "macs-format-types.hpp"
#include "macs-texture-types.hpp"


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
             * Creates a new render pass object, attaching the given objects
             * (e.g., textures) to it. The given render pass script (RPS) will
             * then be compiled and the render pass object (i.e., the FBO)
             * initialized.
             *
             * @param input Input object list.
             * @param output Output object list.
             * @param global_src Global render pass script source code (e.g.
             *                   functions).
             * @param shared_src Shared local RPS source code.
             * @param ... Values the output objects shall be set to
             *            (const char *).
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
            render(
                std::initializer_list<const in *> input,
                std::initializer_list<const out *> output,
                const char *global_src,
                const char *shared_src,
                ...
            );

            /**
             * Frees a render pass object.
             */
            ~render(void);


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
             * @note If you want to execute the same render pass object all over
             *       while altering the input object values (non-textures only)
             *       for each pass, you may do exactly that (you have to call
             *       <tt>prepare()</tt> just once).
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
             * @param comp Comparison function; defaults to pass::less_or_equal.
             *             The current function will not be altered if dt is
             *             false.
             */
            void use_depth(bool dt, comparison comp = less_or_equal);

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


            /**
             * Blending modes. Sets how fragment values are combined from
             * existing and newly written values. One factor is used for scaling
             * the old and one for the new value.
             */
            enum blend_fact
            {
                /// Don't use
                discard = GL_ZERO,
                /// Use directly
                use = GL_ONE,
                /// Multiply by source color
                source = GL_SRC_COLOR,
                /// Multiply by negated source color ((1, 1, 1, 1) - source)
                neg_source = GL_ONE_MINUS_SRC_COLOR,
                /// Multiply by destination color
                destination = GL_DST_COLOR,
                /// Multiply by negated destination color ((1, 1, 1, 1) - destination)
                neg_destination = GL_ONE_MINUS_DST_COLOR,
                /// Multiply by source alpha value
                src_alpha = GL_SRC_ALPHA,
                /// Multiply by negated source alpha value (1 - source alpha)
                neg_src_alpha = GL_ONE_MINUS_SRC_ALPHA,
                /// Multiply by destination alpha value
                dst_alpha = GL_DST_ALPHA,
                /// Multiply by negated destination alpha value (1 - destination alpha)
                neg_dst_alpha = GL_ONE_MINUS_DST_ALPHA
            };

            /**
             * Sets the blending function. On a write to a fragment, the
             * blending specifies how the existing buffer content and how the
             * newly written color is to be accounted, respectively. This
             * function specifies how to weigh both.
             *
             * @param src Specifies the source weight (newly written values).
             * @param dst Specifies the destination weight (values already
             *            contained in the frame buffer).
             */
            void blend_func(blend_fact src, blend_fact dst);


        private:
            /// Binds an FBO for drawing.
            void bind_fbo(int i);


            /// Number of FBOs
            int fbos;

            /// OpenGL FBO IDs
            GLuint *ids;

            /// Depth testing enabled
            bool de;
            /// Stencil testing enabled
            bool se;

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
            /// Blending factor for source values
            blend_fact bfsrc;
            /// Blending factor for destination values
            blend_fact bfdst;

            /// Input objects
            std::list<const in *> inp_objs;
            /// Output objects
            std::list<const out *> out_objs;

            /// Generated programs
            internals::program *prgs;

            /// True iff that's the case.
            bool freshly_prepared;
    };


    /**
     * Allows rendering to screen. You may want to display your result on
     * screen, e.g. via the <tt>texture::display()</tt> function. This function
     * enables onscreen rendering, after offscreen computations have been
     * carried out.
     *
     * @param backbuffer Must be true in order to render to the back buffer,
     *                   else this function will enable rendering to the front
     *                   buffer. You should set this parameter to true for
     *                   double and to false for single buffering.
     */
    void render_to_screen(bool backbuffer);
}

#endif
