/**
 * @file material.hpp
 *
 * Contains the material definition. A material is obviously used to assign a
 * material to an object instance.
 */

#ifndef BETELGEUSE_MATERIAL_HPP
#define BETELGEUSE_MATERIAL_HPP

#include <macs/macs.hpp>


namespace betelgeuse
{
    /**
     * Material class. A material defines both surface and interior attributes
     * (such as the refraction coefficient) for object instances.
     */
    struct material
    {
        /// True iff color is textured
        bool color_texed;
        /// True iff ambient lighting is textured
        bool ambient_texed;
        /// True iff roughness/isotropy is textured
        bool rp_texed;


        // unions would be nicer, they do however not work.

        struct
        {
            /// Constant value
            macs::types::vec3 flat;
            /// Textured
            macs::texture *tex;
        } /** Main color */ color;

        struct
        {
            /// Constant value
            macs::types::vec3 flat;
            /// Textured
            macs::texture *tex;
        } /** Ambient lighting */ ambient;

        struct
        {
            /// Constant values
            macs::types::vec2 flat;
            /// Textured
            macs::texture *tex;
        } /** Roughness/isotropy */ rp;
    };
}

#endif
