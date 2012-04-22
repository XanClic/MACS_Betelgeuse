/**
 * @file macs-internals.hpp
 *
 * Internal include file. Contains global variables in the internal namespace
 * used to track the global status.
 */


#ifndef MACS_INTERNALS_HPP
#define MACS_INTERNALS_HPP

#include <cstdio>


#ifdef __WIN32
#include <GL/glew.h>
#endif

#include <GL/gl.h>

#ifndef __WIN32
#include <GL/glext.h>
#endif


#include "macs-config.hpp"
#include "macs-exceptions.hpp"


#ifdef DEBUG
/// Puts quotation marks around its argument.
#define quote(x) #x
/// Evalutes its argument before quoting it.
#define eval(x) quote(x)
/// Debugging printf (no-op on !defined(DEBUG))
#define dbgprintf(format, ...) fprintf(stderr, "libmacs:" __FILE__ ":%s():" eval(__LINE__) ": " format, __func__, ##__VA_ARGS__)
#else
/// Debugging printf (no-op on !defined(DEBUG))
#define dbgprintf(...)
#endif


namespace macs
{
    class in;
    class out;
    class textures_in;
    class texture;
    class texture_array;


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

        /// Draw buffer count allowed
        extern int draw_bufs;
        /// Color buffer attachment count for FBOs
        extern int col_attach;

        /// Minimum of draw_bufs and col_attach (number of output units).
        extern int out_units;

        /// Texture units available
        extern int tex_units;

        /// Width for every render element
        extern int width;
        /// Height for every render element
        extern int height;



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

                /// Shader source copy
                char *src;
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
                 * Sets this uniform.
                 *
                 * @param obj Object to be loaded into this uniform.
                 *
                 * @note Textures must be already assigned to a texture unit
                 *       and that unit may not be used otherwise before the
                 *       shader uses this sampler.
                 */
                void operator=(const in *obj) throw(exc::invalid_type, exc::texture_not_assigned);

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


        /**
         * Represents a texture mapping unit. Every input texture has to be
         * assigned to a TMU. This class is used for managing one such slot.
         */
        class tmu
        {
            public:
                /**
                 * Simple constructor.
                 *
                 * @param unit Hardware texture unit index.
                 */
                tmu(int unit = 0);


                /**
                 * Assigns a texture to this unit.
                 *
                 * @param tex Texture to be assigned.
                 */
                void operator=(const textures_in *tex);

                /**
                 * Converts this TMU to the assigned texture.
                 *
                 * @return Assigned texture.
                 */
                operator const textures_in *(void) const
                { return assigned; }

                /**
                 * Changes the physical TMU this object correspondends to.
                 *
                 * @param tmu_i Physical TMU index.
                 */
                void reassign(int tmu_i)
                { unit = tmu_i; }


            private:
                /// Hardware TMU index.
                int unit;
                /// Texture which has been assigned.
                const textures_in *assigned;
        };

        /**
         * Manages all TMUs. This class is used for managing all those input
         * slots.
         */
        class tmu_manager
        {
            public:
                /**
                 * Initializes the manager and all TMUs.
                 *
                 * @param units Number of units to be managed.
                 */
                tmu_manager(int units);

                /// Basic deconstructor.
                ~tmu_manager(void);


                /**
                 * Marks all TMUs as loosely assigned.
                 */
                void loosen(void);

                /**
                 * Tries to assign this texture to a TMU. If a unit is found
                 * which is already assigned to this texture, this will mark
                 * that TMU as definitely assigned.
                 *
                 * @param tex Texture to assign a TMU to.
                 *
                 * @return True, iff a TMU has been found which has already been
                 *         assigned to it.
                 */
                bool operator&=(const textures_in *tex);

                /**
                 * Assigns this texture to a TMU. Finds a loosely assigned TMU
                 * and assigns it to that texture.
                 *
                 * @param tex Texture to assign a TMU to.
                 *
                 * @note This function will also find a new TMU if another has
                 *       already been set to the given texture, so you should
                 *       use <tt>operator&=</tt> before.
                 */
                void operator+=(const textures_in *tex);

                /**
                 * The last function in the managing cycle. It essentially
                 * just disables loosely assigned units.
                 */
                void update(void);


                /**
                 * Indexing function.
                 *
                 * @param index TMU to be accessed.
                 *
                 * @return Pointer to TMU object.
                 */
                tmu &operator[](int index)
                { return tmus[index]; }


            private:
                /// TMU count
                int units;
                /// TMU array
                tmu *tmus;
                /// True iff definitely assigned
                bool *definitely;
        };


        /// Simple vertex shader which just pipes input XY to output.
        extern shader *basic_vertex_shader;
        /**
         * Simple program. It uses the basic vertex shader and a standard
         * fragment shader.
         */
        extern program *basic_pipeline;

        /// Central TMU manager.
        extern tmu_manager *tmu_mgr;


        /**
         * Draws a quad. Draws a textured quad to the whole framebuffer
         * (implying that the modelview and projection matrices are both
         * identity matrices or a shader is used which circumvents this).
         */
        void draw_quad(void);
    }
}

#include "macs.hpp"
#include "macs-root.hpp"

#endif
