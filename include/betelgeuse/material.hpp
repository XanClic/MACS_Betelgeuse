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
        /// True iff ambient lighting is textured
        bool ambient_texed;
        /// True iff mirroring reflections are textured
        bool mirror_texed;
        /// True iff refractions are textured
        bool refract_texed;

        /// Refraction coefficient
        float n;

        // unions would be nicer, they do however not work.

        struct
        {
            /// Constant value
            macs::types::vec3 flat;
            /// Textured
            macs::texture *tex;
        } /** Ambient lighting */ ambient;

        struct
        {
            /// Constant value
            macs::types::vec3 flat;
            /// Textured
            macs::texture *tex;
        } /** Per-channel mirror reflection */ mirror;

        struct
        {
            /// Constant value
            macs::types::vec4 flat;
            /// Textured
            macs::texture *tex;
        } /** Per-channel refraction, alpha is coefficient */ refract;


        struct
        {
            /// True iff layer color is textured
            bool color_texed;
            /// True iff layer roughness/isotropy is textured
            bool rp_texed;

            struct
            {
                /// Constant value
                macs::types::vec3 flat;
                /// Textured
                macs::texture *tex;
            } /** Layer color */ color;

            struct
            {
                /// Constant value
                macs::types::vec2 flat;
                /// Textured
                macs::texture *tex;
            } /** Layer roughness/isotropy */ rp;
        } /** Surface layers */ layer[2];
    };
}

#endif
