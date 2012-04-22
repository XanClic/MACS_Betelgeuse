/**
 * @file objects.hpp
 *
 * Objects definitions. These are both the generic definitions for objects as
 * well as the instance class for one single object instantiated.
 */

#ifndef BETELGEUSE_OBJECTS_HPP
#define BETELGEUSE_OBJECTS_HPP

#include <list>

#include <macs/macs.hpp>

#include "material.hpp"


namespace betelgeuse
{
    class scene;
    class object;


    /**
     * Object instance class. An object instance designates one physical object
     * visible in the scene.
     */
    class instance
    {
        public:
            /// Basic constructor.
            instance(object *obj);
            /// Basic deconstructor.
            ~instance(void);

            /**
             * Updates the transformation. When writing to the transformation
             * matrix, you have to call this function afterwards to carry out
             * the actual transformation.
             *
             * @sa instance::trans
             */
            void update_transformation(void);

            /**
             * Transformation matrix.
             *
             * @sa void instance::update_transformation(void)
             */
            macs::types::mat4 trans;

            /// Material assigned to this instance.
            material mat;


            friend class scene;

        private:
            /// Inverse transformation matrix.
            macs::types::mat4 inv_trans;
            /// Normal matrix (transposed inverse).
            macs::types::mat3 normal;

            /// Object class this instance belongs to.
            object *obj;
    };

    /**
     * Object type class. An object is not visible in the scene, as it specifies
     * an object class rather than one single instance. To create visible
     * objects, you have to create instances instead.
     *
     * @sa instance
     */
    class object
    {
        public:
            /**
             * Object type constructor.
             *
             * @param min_isct Function body to find the first intersection.
             * @param isct Function body to return true iff a given line
             *             intersects (given by <tt>start</tt> and <tt>dir</tt>
             *             <tt>vec3</tt> parameters
             *             (<tt>end = start + dir</tt>).
             * @param uv Function body to return a UV coordinate when given an
             *           intersection point.
             * @param norm Function body to return a surface normal when given
             *             an intersection point.
             * @param tang Function body to return a surface tangent when given
             *             an intersection point.
             */
            object(const char *min_isct, const char *isct, const char *uv, const char *norm, const char *tang = NULL);
            /// Basic deconstructor.
            ~object(void);

            /// Creates an instance of this object.
            instance *instantiate(void);


            friend class scene;
            friend class instance;

        private:
            /// Global source code (min_isct, norm and tang functions).
            char *global_src;
            /// Intersection (and basically everything) render object.
            macs::render *isct;
            /// Current transformation matrix object.
            macs::types::named<macs::types::mat4> cur_trans;
            /// Current inverse transformation matrix object.
            macs::types::named<macs::types::mat4> cur_inv_trans;
            /// Current normal matrix object.
            macs::types::named<macs::types::mat3> cur_normal;

            /// Currend material color flat/texture switch.
            macs::types::named<bool> cur_color_flat_tex;
            /// Currend material ambient color flat/texture switch.
            macs::types::named<bool> cur_ambient_flat_tex;
            /// Currend material roughness/isotropy flat/texture switch.
            macs::types::named<bool> cur_rp_flat_tex;

            /// Current material color object.
            macs::types::named<macs::types::vec3> cur_color_flat;
            /// Current material ambient color object.
            macs::types::named<macs::types::vec3> cur_ambient_flat;
            /// Current material roughness/isotropy object.
            macs::types::named<macs::types::vec2> cur_rp_flat;

            /// Current material color texture.
            macs::texture *cur_color_tex;
            /// Current material ambient color texture.
            macs::texture *cur_ambient_tex;
            /// Current material roughness/isotropy texture.
            macs::texture *cur_rp_tex;

            /// List of available instances.
            std::list<instance *> insts;

            /// Global source code for shadowing (line_intersects)
            char *global_shadow_src;

            /// Shadow rendering object.
            macs::render *shadow;
    };
}

#endif
