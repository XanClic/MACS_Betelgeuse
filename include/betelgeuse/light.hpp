/**
 * @file light.hpp
 *
 * Defines the light class.
 */

#ifndef BETELGEUSE_LIGHT_HPP
#define BETELGEUSE_LIGHT_HPP

#include <macs/macs.hpp>


namespace betelgeuse
{
    class scene;


    /**
     * Light class. This defines a physically visible light instance.
     */
    class light
    {
        public:
            /**
             * Basic constructor.
             *
             * @param atten_func Attenuation function.
             */
            light(const char *atten_func);
            /// Basic deconstructor.
            ~light(void);

            /// Light position.
            macs::types::named<macs::types::vec4> position;
            /// Light direction.
            macs::types::named<macs::types::vec3> direction;
            /// Light color.
            macs::types::named<macs::types::vec3> color;
            /// Distribution exponent.
            macs::types::named<float> distr_exp;
            /// Cosine of the cutoff angle.
            macs::types::named<float> limit_angle_cos;
            /// Attenuation parameter.
            macs::types::named<float> atten_par;


            friend class scene;

        private:
            /// Shading render object.
            macs::render *shade;

            /// Attenuation function source code.
            const char *atten_func;
    };
}

#endif
