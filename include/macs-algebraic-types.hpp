/**
 * @file macs-algebraic-types.hpp
 *
 * Type include file. Contains basic algebraic data types which may be used in
 * computations and features basic calculations functions.
 */

#ifndef MACS_ALGEBRAIC_TYPES_HPP
#define MACS_ALGEBRAIC_TYPES_HPP

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>

#include "macs-internals.hpp"
#include "macs-root.hpp"


namespace macs
{
    /**
     * Namespace containing basic algebraic types. This namespace contains basic
     * types (i.e., vectors and matrices) for use in render passes. They also
     * feature a minimum set of functions to do various calculations for your
     * convenience.
     */
    namespace types
    {
        class mat3;
        class mat4;


        /// 4-element vector consisting of floats
        class vec4
        {
            public:
                /**
                 * Basic constructor. All coordinates will be set to zero.
                 */
                vec4(void)
                { memset(d, 0, sizeof(d)); }

                /**
                 * Basic constructor.
                 *
                 * @param dat Array containing the initial values.
                 */
                vec4(const float *dat)
                { memcpy(d, dat, sizeof(d)); }

                /// @overload vec4::vec4(const float *dat)
                vec4(float xv, float yv, float zv, float wv):
                    x(xv), y(yv), z(zv), w(wv) {}


                /**
                 * Overloads indexing. With the [] operator you are able to
                 * access every coordinate according to its index.
                 *
                 * @param index Index to be addressed.
                 *
                 * @return Reference to the coordinate.
                 *
                 * @note This function does not perform bounds checking.
                 */
                float &operator[](int index)
                { return d[index]; }

                /// @overload float &vec4::operator[](int index)
                float operator[](int index) const
                { return d[index]; }


                /**
                 * Vector addition. Adds two vectors component-by-component.
                 */
                vec4 operator+(const vec4 &ov) const
                { return vec4(x + ov.x, y + ov.y, z + ov.z, w + ov.w); }

                /**
                 * Vector addition. Adds the given vector to this one.
                 */
                const vec4 &operator+=(const vec4 &ov)
                { x += ov.x; y += ov.y; z += ov.z; w += ov.w; return *this; }

                /**
                 * Vector subtraction. Subtracts two vectors component-by-component.
                 */
                vec4 operator-(const vec4 &ov) const
                { return vec4(x - ov.x, y - ov.y, z - ov.z, w - ov.w); }

                /**
                 * Vector subtraction. Subtracts the given vector from this one.
                 */
                const vec4 &operator-=(const vec4 &ov)
                { x -= ov.x; y -= ov.y; z -= ov.z; w -= ov.w; return *this; }

                /**
                 * Scalar multiplication. Multiplies a vector with a scalar
                 * value.
                 */
                vec4 operator*(float sv) const
                { return vec4(sv * x, sv * y, sv * z, sv * w); }

                /**
                 * Scalar multiplication. Multiplies every component of this
                 * vector with the given scalar value.
                 */
                const vec4 &operator*=(float sv)
                { x *= sv; y *= sv; z *= sv; w *= sv; return *this; }

                /**
                 * Matrix multiplication. Applies the given matrix to this
                 * vector, i.e., multiplies the matrix with the vector and
                 * stores the result in here.
                 */
                const vec4 &operator*=(const mat4 &m);

                /**
                 * Dot product. Executes the dot product on two vectors.
                 */
                float operator*(const vec4 &ov) const
                { return x * ov.x + y * ov.y + z * ov.z + w * ov.w; }

                /// Length (norm) of this vector.
                float length(void) const
                { return sqrtf(x * x + y * y + z * z + w * w); }

                /**
                 * Squares this vector. The result is the squared length (dot
                 * product of this vector with itself).
                 */
                float sqr(void) const
                { return x * x + y * y + z * z + w * w; }

                /**
                 * Norms this vector and returns the result.
                 *
                 * @return Normed vector (length is 1) which points to the same
                 *         direction as this one does.
                 */
                vec4 normed(void) const
                {
                    float rlen = 1.f / length();
                    return vec4(rlen * x, rlen * y, rlen * z, rlen * w);
                }

                /// Norms this vector, storing the result in here.
                void norm(void)
                {
                    float rlen = 1.f / length();
                    x *= rlen; y *= rlen; z *= rlen; w *= rlen;
                }


                /**
                 * Converts a vector into a string representation (for usage
                 * in render pass scripts).
                 */
                operator std::string(void) const
                {
                    char str[128]; // FIXME
                    sprintf(str, "vec4(%f, %f, %f, %f)", x, y, z, w);
                    return std::string(str);
                }


                /// Coordinates in different flavors.
                union
                {
                    /// An array
                    float d[4];
                    /// XYZW (coordinates)
                    struct { float x, y, z, w; };
                    /// RGBA (color channels)
                    struct { float r, g, b, a; };
                    /// STPQ (texture coordinates)
                    struct { float s, t, p, q; };
                };
        };


        /// 3-element vector consisting of floats.
        class vec3
        {
            public:
                /**
                 * Basic constructor. All coordinates will be implicitly set to
                 * zero.
                 */
                vec3(void)
                { memset(d, 0, sizeof(d)); }

                /**
                 * Basic constructor.
                 *
                 * @param dat Array containing the initial values.
                 */
                vec3(const float *dat)
                { memcpy(d, dat, sizeof(d)); }

                /// @overload vec3::vec3(const float *dat)
                vec3(float xv, float yv, float zv):
                    x(xv), y(yv), z(zv) {}


                /**
                 * Overloads indexing. With the [] operator you are able to
                 * access every coordinate according to its index.
                 *
                 * @param index Index to be addressed.
                 *
                 * @return Reference to the coordinate.
                 *
                 * @note This function does not perform bounds checking.
                 */
                float &operator[](const int &index)
                { return d[index]; }

                /// @overload float &vec3::operator[](int index)
                float operator[](int index) const
                { return d[index]; }

                /**
                 * Vector addition. Adds two vectors component-by-component.
                 */
                vec3 operator+(const vec3 &ov) const
                { return vec3(x + ov.x, y + ov.y, z + ov.z); }

                /**
                 * Vector addition. Adds the given vector to this one.
                 */
                const vec3 &operator+=(const vec3 &ov)
                { x += ov.x; y += ov.y; z += ov.z; return *this; }

                /**
                 * Vector subtraction. Subtracts two vectors component-by-component.
                 */
                vec3 operator-(const vec3 &ov) const
                { return vec3(x - ov.x, y - ov.y, z - ov.z); }

                /**
                 * Vector subtraction. Subtracts the given vector from this one.
                 */
                const vec3 &operator-=(const vec3 &ov)
                { x -= ov.x; y -= ov.y; z -= ov.z; return *this; }

                /**
                 * Scalar multiplication. Multiplies a vector with a scalar
                 * value.
                 */
                vec3 operator*(float sv) const
                { return vec3(sv * x, sv * y, sv * z); }

                /**
                 * Scalar multiplication. Multiplies every component of this
                 * vector with the given scalar value.
                 */
                const vec3 &operator*=(float sv)
                { x *= sv; y *= sv; z *= sv; return *this; }

                /**
                 * Matrix multiplication. Applies the given matrix to this
                 * vector, i.e., multiplies the matrix with the vector and
                 * stores the result in here.
                 */
                const vec3 &operator*=(const mat3 &m);

                /**
                 * Dot product. Executes the dot product on two vectors.
                 */
                float operator*(const vec3 &ov) const
                { return x * ov.x + y * ov.y + z * ov.z; }

                /**
                 * Cross product. Executes the cross product on two vectors.
                 */
                vec3 cross(const vec3 &ov) const
                {
                    return vec3(
                        y * ov.z - z * ov.y,
                        z * ov.x - x * ov.z,
                        x * ov.y - y * ov.x
                    );
                }

                /// Length (norm) of this vector.
                float length(void) const
                { return sqrtf(x * x + y * y + z * z); }

                /**
                 * Squares this vector. The result is the squared length.
                 *
                 * @return Dot product of this vector with itself.
                 */
                float sqr(void) const
                { return x * x + y * y + z * z; }

                /**
                 * Norms this vector and returns the result.
                 *
                 * @return Normed vector (length is 1) which points to the same
                 *         direction as this one does.
                 */
                vec3 normed(void) const
                {
                    float rlen = 1.f / length();
                    return vec3(rlen * x, rlen * y, rlen * z);
                }

                /// Norms this vector, storing the result in here.
                void norm(void)
                {
                    float rlen = 1.f / length();
                    x *= rlen; y *= rlen; z *= rlen;
                }


                /**
                 * Converts a vector into a string representation (for usage
                 * in render pass scripts).
                 */
                operator std::string(void) const
                {
                    char str[96]; // FIXME
                    sprintf(str, "vec3(%f, %f, %f)", x, y, z);
                    return std::string(str);
                }


                /// Coordinates in different flavors.
                union
                {
                    /// An array
                    float d[3];
                    /// XYZW (coordinates)
                    struct { float x, y, z; };
                    /// RGBA (color channels)
                    struct { float r, g, b; };
                    /// STPQ (texture coordinates)
                    struct { float s, t, p; };
                };
        };


        /// 4x4 matrix consisting of floats.
        class mat4
        {
            public:
                /**
                 * Basic constructor. Creates an identity matrix.
                 */
                mat4(void)
                { memset(d, 0, sizeof(d)); d[0] = d[5] = d[10] = d[15] = 1.f; }

                /**
                 * Basic constructor. The contents of the matrix will be copied
                 * from the given array.
                 */
                mat4(const float *dat)
                { memcpy(d, dat, sizeof(d)); }


                /**
                 * Applies a matrix to a vector. This matrix is multiplied by
                 * the vector, returning the resulting vector.
                 */
                vec4 operator*(const vec4 &ov) const
                {
                    return vec4(
                        ov.d[0] * d[0] + ov.d[1] * d[4] + ov.d[2] * d[ 8] + ov.d[3] * d[12],
                        ov.d[0] * d[1] + ov.d[1] * d[5] + ov.d[2] * d[ 9] + ov.d[3] * d[13],
                        ov.d[0] * d[2] + ov.d[1] * d[6] + ov.d[2] * d[10] + ov.d[3] * d[14],
                        ov.d[0] * d[3] + ov.d[1] * d[7] + ov.d[2] * d[11] + ov.d[3] * d[15]
                    );
                }

                /// Multiplies two matrices.
                mat4 operator*(const mat4 &om) const;
                /// Multiplies this matrix with another one.
                const mat4 &operator*=(const mat4 &om);

                /// Tranposes this matrix and returns the result.
                mat4 transposed(void) const
                {
                    float nd[16] = {
                        d[0], d[4], d[ 8], d[12],
                        d[1], d[5], d[ 9], d[13],
                        d[2], d[6], d[10], d[14],
                        d[3], d[7], d[11], d[15]
                    };
                    return mat4(nd);
                }

                /// Transposes this matrix, storing the result in here.
                void transpose(void)
                {
                    float nd[16] = {
                        d[0], d[4], d[ 8], d[12],
                        d[1], d[5], d[ 9], d[13],
                        d[2], d[6], d[10], d[14],
                        d[3], d[7], d[11], d[15]
                    };
                    memcpy(d, nd, sizeof(nd));
                }

                /// Returns the determinant.
                float det(void) const;
                /// Returns the inverse matrix.
                mat4 inv(void) const;
                /// Inverts this matrix.
                void invert(void);


                /**
                 * Translates this matrix according to the given vector. Creates
                 * a translation matrix based on that vector and applies it to
                 * this matrix.
                 */
                void translate(const vec3 &v)
                {
                    d[12] += v.d[0] * d[0] + v.d[1] * d[4] + v.d[2] * d[ 8];
                    d[13] += v.d[0] * d[1] + v.d[1] * d[5] + v.d[2] * d[ 9];
                    d[14] += v.d[0] * d[2] + v.d[1] * d[6] + v.d[2] * d[10];
                    d[15] += v.d[0] * d[3] + v.d[1] * d[7] + v.d[2] * d[11];
                }

                /**
                 * Scales this matrix according to the given vector. Creates a
                 * scale matrix based on that vector and applies it to this
                 * matrix.
                 */
                void scale(const vec3 &v)
                {
                    d[0] *= v.d[0]; d[1] *= v.d[0]; d[ 2] *= v.d[0]; d[ 3] *= v.d[0];
                    d[4] *= v.d[1]; d[5] *= v.d[1]; d[ 6] *= v.d[1]; d[ 7] *= v.d[1];
                    d[8] *= v.d[2]; d[9] *= v.d[2]; d[10] *= v.d[2]; d[11] *= v.d[2];
                }

                /**
                 * Rotates this matrix according to the given vector. Creates a
                 * rotation matrix based on that vector and applies it to this
                 * matrix.
                 */
                void rotate(float angle, vec3 axis);


                /// Matrix contents
                float d[16];
        };


        /// 3x3 matrix consisting of floats
        class mat3
        {
            public:
                /**
                 * Basic constructor. Creates an identity matrix.
                 */
                mat3(void)
                { memset(d, 0, sizeof(d)); d[0] = d[4] = d[8] = 1.f; }

                /**
                 * Basic constructor. The contents of the matrix will be copied
                 * from the given array.
                 */
                mat3(const float *dat)
                { memcpy(d, dat, sizeof(d)); }

                /**
                 * Copies the left upper 3x3 values from the given matrix into
                 * the one to be created.
                 */
                mat3(const mat4 &m4)
                { memcpy(&d[0], &m4.d[0], sizeof(d[0]) * 3); memcpy(&d[3], &m4.d[4], sizeof(d[0]) * 3); memcpy(&d[6], &m4.d[8], sizeof(d[0]) * 3); }


                /**
                 * Applies a matrix to a vector. This matrix is multiplied by
                 * the vector, returning the resulting vector.
                 */
                vec3 operator*(const vec3 &ov) const
                {
                    return vec3(
                        ov.d[0] * d[0] + ov.d[1] * d[3] + ov.d[2] * d[6],
                        ov.d[0] * d[1] + ov.d[1] * d[4] + ov.d[2] * d[7],
                        ov.d[0] * d[2] + ov.d[1] * d[5] + ov.d[2] * d[8]
                    );
                }

                /// Multiplies two matrices.
                mat3 operator*(const mat3 &om) const;
                /// Multiplies this matrix with another one.
                const mat3 &operator*=(const mat3 &om);

                /// Tranposes this matrix and returns the result.
                mat3 transposed(void) const
                {
                    float nd[9] = {
                        d[0], d[3], d[6],
                        d[1], d[4], d[7],
                        d[2], d[5], d[8]
                    };
                    return mat3(nd);
                }

                /// Transposes this matrix, storing the result in here.
                void transpose(void)
                {
                    float nd[9] = {
                        d[0], d[3], d[6],
                        d[1], d[4], d[7],
                        d[2], d[5], d[8]
                    };
                    memcpy(d, nd, sizeof(nd));
                }

                /// Returns the determinant.
                float det(void) const
                {
                    return d[0] * (d[4] * d[8] - d[5] * d[7]) -
                           d[3] * (d[1] * d[8] - d[2] * d[7]) +
                           d[6] * (d[1] * d[5] - d[2] * d[4]);
                }

                /// Returns the inverse matrix.
                mat3 inv(void) const;
                /// Inverts this matrix.
                void invert(void);


                /// Matrix contents
                float d[9];
        };


        /**
         * This namespace contains named variants of the macs::types classes.
         * These are not usable in any way than to join an identifier and such
         * a type for use in constructors of render pass objects.
         */
        namespace rps
        {
            /// vec4 wrapper
            class vec4: public vec
            {
                public:
                    /// Basic constructor
                    vec4(const char *name, const types::vec4 &vector)
                    { i_type = in::t_vec4; i_name = strdup(name); v = vector; }

                    /// Basic deconstructor
                    ~vec4(void)
                    { free(const_cast<char *>(i_name)); }


                    friend class internals::prg_uniform;

                private:
                    /// Vector
                    types::vec4 v;
            };

            /// vec3 wrapper
            class vec3: public vec
            {
                public:
                    /// Basic constructor
                    vec3(const char *name, const types::vec3 &vector)
                    { i_type = in::t_vec3; i_name = strdup(name); v = vector; }

                    /// Basic deconstructor
                    ~vec3(void)
                    { free(const_cast<char *>(i_name)); }


                    friend class internals::prg_uniform;

                private:
                    /// Vector
                    types::vec3 v;
            };

            /// mat4 wrapper
            class mat4: public mat
            {
                public:
                    /// Basic constructor
                    mat4(const char *name, const types::mat4 &matrix)
                    { i_type = in::t_mat4; i_name = strdup(name); m = matrix; }

                    /// Basic deconstructor
                    ~mat4(void)
                    { free(const_cast<char *>(i_name)); }


                    friend class internals::prg_uniform;

                private:
                    /// Matrix
                    types::mat4 m;
            };

            /// mat3 wrapper
            class mat3: public mat
            {
                public:
                    /// Basic constructor
                    mat3(const char *name, const types::mat3 &matrix)
                    { i_type = in::t_mat3; i_name = strdup(name); m = matrix; }

                    /// Basic deconstructor
                    ~mat3(void)
                    { free(const_cast<char *>(i_name)); }


                    friend class internals::prg_uniform;

                private:
                    /// Matrix
                    types::mat3 m;
            };
        }
    }
}

#endif
