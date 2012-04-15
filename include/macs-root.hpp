/**
 * @file macs-root.hpp
 *
 * Contains base classes. Those are used to combine several different classes
 * to achieve compile-time type safety in lists.
 */

#ifndef MACS_ROOT_HPP
#define MACS_ROOT_HPP

#include "macs-config.hpp"

namespace macs
{
    class render;

#ifdef DEBUG
    namespace internals
    {
        class tmu;
    }
#endif


    /**
     * Base class for all input classes.
     *
     * These may be used as input for render passes and thus be read from render
     * pass scripts.
     */
    class in
    {
        public:
            /// Derived classes
            enum i_types
            {
                /**
                 * Basic 2D texture.
                 *
                 * @sa macs::texture
                 */
                t_texture,

                /**
                 * Array of 2D textures.
                 *
                 * @sa macs::texture_array
                 */
                t_texture_array,

                /**
                 * 4-element vector.
                 *
                 * @sa macs::types::vec4
                 */
                t_vec4,

                /**
                 * 3-element vector.
                 *
                 * @sa macs::types::vec3
                 */
                t_vec3,

                /**
                 * 4x4 matrix.
                 *
                 * @sa macs::types::mat4
                 */
                t_mat4,

                /**
                 * 3x3 matrix.
                 *
                 * @sa macs::types::mat3
                 */
                t_mat3,

                /// Scalar floating point number.
                t_float
            };

            /// Derived class type
            i_types i_type;


            friend class render;
#ifdef DEBUG
            friend class internals::tmu;
#endif

        protected:
            /// Name used to denote this object in render pass scripts.
            const char *i_name;
    };

    /**
     * Base class for all output classes.
     *
     * These may be used as output from render passes and thus be written to in
     * render pass scripts.
     */
    class out
    {
        public:
            /// Derived classes
            enum o_types
            {
                /**
                 * Basic 2D texture.
                 *
                 * @sa macs::texture
                 */
                t_texture,

                /**
                 * Combined stencil/depth buffer.
                 *
                 * @sa macs::stencildepth
                 */
                t_stencildepth
            };

            /// Derived class type
            o_types o_type;


            friend class render;

        protected:
            /// Name used to denote this object in render pass scripts.
            const char *o_name;
    };


    /**
     * Base class for all algebraic types.
     *
     * These are only usable as input objects.
     */
    class algebraic: public in
    {
    };

    /// Represents textures usable as input.
    class textures_in: public in
    {
    };

    /// Represents textures usable as output.
    class textures_out: public out
    {
    };
}

#include "macs.hpp"
#include "macs-internals.hpp"

#endif
