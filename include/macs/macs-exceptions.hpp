/**
 * @file macs-exceptions.hpp
 *
 * Exceptions which may occur.
 */

#ifndef MACS_EXCEPTIONS_HPP
#define MACS_EXCEPTIONS_HPP

#include <exception>

namespace macs
{
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

        /**
         * Shader compilation/linking failed exception. Thrown if linking or
         * compiling a shader (i.e., a render pass script) failed.
         */
        static class shader_compilation_linking_failed: std::exception
        {
            public:
                /// Returns an error description.
                virtual const char *what(void) const throw()
                { return "Shader compilation or linking failed."; }
        }
        /// Shader compilation/linking failed exception instance
        shader_fail;

        /**
         * Texture not declared exception. Thrown if a texture is tried to be
         * defined, but has not been declared before (by a texture replacement
         * object).
         */
        static class texture_not_declared: std::exception
        {
            public:
                /// Returns an error description.
                virtual const char *what(void) const throw()
                { return "Requested texture has not been declared before."; }
        }
        /// Texture not declared exception instance
        tex_nd;
    }
}

#endif
