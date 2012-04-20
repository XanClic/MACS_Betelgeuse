/**
 * @file scene.hpp
 *
 * Scene class include file.
 */

#ifndef BETELGEUSE_SCENE_HPP
#define BETELGEUSE_SCENE_HPP

namespace betelgeuse
{
    class object;
    class light;


    /**
     * The scene class is used for rendering. It combines all object types and
     * all instances and renders them onto the screen.
     */
    class scene
    {
        public:
            /// Basic constructor.
            scene(void);

            /**
             * Sets the horizontal field of view.
             *
             * @param fov FOV in radians.
             */
            void set_fov(float fov);
            /// Sets the display aspect (X/Y).
            void set_aspect(float aspect);

            /// Adds an object type.
            void new_object_type(object *obj);
            /// Adds a light instance.
            void add_light(light *lgt);


            /**
             * Renders everything into the <tt>output</tt> texture.
             *
             * @sa scene::output
             */
            void render(void);
            /// Displays the <tt>output</tt> texture onto the screen.
            void display(void);


            /**
             * Output texture. This is the place where everything is rendered
             * to. If you need to, you may access this directly as allowed by
             * the MACS framework. If you just want to display it one the
             * screen, use the <tt>display()</tt> method.
             *
             * @sa void scene::display(void)
             */
            macs::texture output;


        private:
            /// Initializes the view rays.
            void render_view(void);
            /// Renders object intersection points.
            void render_intersection(void);
            /// Does the light shading.
            void render_shading(void);


            /// Display aspect.
            float aspect;
            /// Vertical FOV.
            macs::types::named<float> yfov;
            /// Horizontal FOV.
            macs::types::named<float> xfov;
            /// Anticipated most far Z value.
            macs::types::named<float> zfar;

            /// Camera position.
            macs::types::named<macs::types::vec4> cam_pos;
            /// Camera's forward pointing vector.
            macs::types::named<macs::types::vec3> cam_fwd;
            /// Camera's right pointing vector.
            macs::types::named<macs::types::vec3> cam_rgt;
            /// Camera's up pointing vector.
            macs::types::named<macs::types::vec3> cam_up;

            /// List of object classes attached.
            std::list<object *> objs;
            /// List of lights attached.
            std::list<light *> lgts;

            /// Stencil/depth buffer used for intersection calculcation.
            macs::stencildepth sd;

            /// Ray starting points.
            macs::texture ray_stt;
            /// Ray directions.
            macs::texture ray_dir;
            /// Global intersection point map.
            macs::texture glob_isct;
            /// Surface normal map.
            macs::texture norm_map;
            /// Surface tangent map.
            macs::texture tang_map;
            /// Material color map.
            macs::texture color_map;
            /// Material ambient map.
            macs::texture ambient_map;
            /// Material roughness/isotropy and texture UV map.
            macs::texture rpuv_map;

            /// "Artificial" stencil buffer (for early-out in fragment shaders).
            macs::texture asten;

            /// Initial view rendering object.
            macs::render *rnd_view;
    };
}

#endif
