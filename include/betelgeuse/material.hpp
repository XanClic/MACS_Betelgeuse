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
        /// Main color
        macs::types::vec3 color;
        /// Ambient lighting
        macs::types::vec3 ambient;
        /// Roughness
        float r;
        /// Isotropy
        float p;
    };
}

#endif
