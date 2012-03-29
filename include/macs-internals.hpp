/**
 * @file macs-internals.hpp
 *
 * Internal include file. Contains global variables in the internal namespace
 * used to track the global status.
 */


#ifndef MACS_INTERNALS_HPP
#define MACS_INTERNALS_HPP

#include <GL/gl.h>


/**
 * DEBUG macro. If enabled, generate debugging code.
 */
#define DEBUG


#ifdef DEBUG
#include <stdio.h>
/// Puts quotation marks around its argument.
#define quote(x) #x
/// Evalutes its argument before quoting it.
#define eval(x) quote(x)
/**
 * Debugging printf. When DEBUG is enabled (i.e., now)  generates a printf to
 * stderr, else no code is generated at all.
 *
 * @param ... Standard printf parameters (format string and further arguments).
 *
 * @return It's better if you don't care.
 */
#define dbgprintf(...) fprintf(stderr, "libmacs:" __FILE__ ":" eval(__LINE__) ": " __VA_ARGS__)
#else
/**
 * Debugging printf. When DEBUG is enabled, generates a printf to stderr, else
 * (i.e., now) no code is generated at all.
 */
#define dbgprintf(...)
#endif


namespace macs
{
    class texture;


    /**
     * Internal MACS namespace. None of its contents should be used from
     * external applications.
     */
    namespace internals
    {
        /// OpenGL major version
        extern int ogl_maj;
        /// OpenGL minor version
        extern int ogl_min;

        /// Draw buffer count
        extern int draw_bufs;

        /// Color buffer attachment count for FBOs
        extern int col_attach;

        /// Texture units available
        extern int tex_units;



        class program;


        /**
         * Shader object. A standard OpenGL shader object, either vertex or
         * fragment shader.
         */
        class shader
        {
            public:
                /**
                 * Shader type. Either it is a vertex or a fragment shader.
                 */
                enum type
                {
                    /// Fragment shader
                    fragment = GL_FRAGMENT_SHADER,
                    /// Vertex shader
                    vertex   = GL_VERTEX_SHADER
                };

                /**
                 * Creates a new shader of the desired type.
                 *
                 * @param t Shader type (fragment or vertex).
                 */
                shader(type t);

                /**
                 * Deletes a shader. If it is still in use by a program, it
                 * won't be actually deleted until that program has been
                 * unloaded.
                 */
                ~shader(void);


                /**
                 * Loads GLSL code. This will load the given string as GLSL code
                 * and attach it to this shader.
                 *
                 * @param src GLSL code to be loaded
                 */
                void load(const char *src);

                /**
                 * Loads GLSL code from a file. This will read the whole file
                 * given by the I/O stream and use the content as this shader's
                 * GLSL code.
                 *
                 * @note The I/O stream must support seeking.
                 *
                 * @param fp I/O stream to read from
                 */
                void load(FILE *fp);


                /**
                 * Compiles this shader. Recompiles the shader's current GLSL
                 * code.
                 *
                 * @return true iff the compilation has been successful.
                 */
                bool compile(void);


                friend class program;

            private:
                /// OpenGL shader ID
                unsigned id;
        };


        /**
         * Program uniform. Used in order to communicate with shader variables.
         */
        class prg_uniform
        {
            public:
                /**
                 * Creates a uniform object from its ID.
                 *
                 * @param id OpenGL uniform location ID.
                 */
                prg_uniform(unsigned id);


                /**
                 * Sets a sampler uniform.
                 *
                 * @param tex Texture to be loaded into the sampler
                 *
                 * @note The texture must be already assigned to a texture unit
                 *       and that unit may not be used otherwise before the
                 *       shader uses this sampler.
                 */
                void operator=(const texture *tex);

            private:
                /// OpenGL uniform location ID
                unsigned id;
        };


        /**
         * Program object. A GLSL program object which combines several shaders
         * and especially joins vertex and fragment shaders.
         */
        class program
        {
            public:
                /**
                 * Creates an empty program.
                 */
                program(void);

                /**
                 * Unloads the program. Deletes this program and releases all
                 * attached shaders.
                 */
                ~program(void);


                /**
                 * Attach a shader. Attaches the given shader to this program so
                 * it will be included upon linkage.
                 *
                 * @param sh Shader to be attached.
                 */
                void attach(shader *sh);

                /**
                 * Link the program. Links all attached shaders and thus makes
                 * this program usable.
                 *
                 * @return true iff the linking has been successful.
                 */
                bool link(void);


                /**
                 * Puts this shader into use. Calling this function will lead to
                 * this program being used by subsequent render operations.
                 */
                void use(void);


                /**
                 * Returns a uniform. Searches the given uniform and returns an
                 * object to access it.
                 *
                 * @param name Uniform identifier
                 */
                prg_uniform uniform(const char *name);

            private:
                /// OpenGL program ID
                unsigned id;
        };
    }
}

#endif
