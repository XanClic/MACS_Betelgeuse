/**
 * @file betelgeuse.hpp
 *
 * Main include file. Includes all files with all structures you'll ever need.
 */

#ifndef BETELGEUSE_HPP
#define BETELGEUSE_HPP

#include "material.hpp"
#include "objects.hpp"
#include "light.hpp"
#include "scene.hpp"


/**
 * Main betelgeuse namespace.
 */
namespace betelgeuse
{
    /**
     * Initializes everything. Call this function before anything else.
     *
     * @param width Width of the output. This will specify how big the image is
     *              this raytracer will generate.
     * @param height Output height.
     * @param double_buffering Set this to true, iff you do double buffering
     *                         (else false, of course).
     *
     * @return True iff successful.
     */
    bool init(int width, int height, bool double_buffering = true);


    extern bool _dbl_buf;
}

#endif
