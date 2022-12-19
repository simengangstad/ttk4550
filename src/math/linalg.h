#ifndef LINALG_H
#define LINALG_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <stdlib.h>

#include "arm_math.h"

namespace linalg {

    // ----------------------------- Matrix ----------------------------------

    template <uint16_t N> struct Mat;

    /**
     * @brief Add @p first and @p second and places the result in @p
     * destination.
     */
    template <uint16_t N>
    void add(const Mat<N>& first, const Mat<N>& second, Mat<N>& destination);

    /**
     * @brief Multiplies @p first with @p second and places the result in @p
     * destination.
     */
    template <uint16_t I, uint16_t J, uint16_t K>
    void
    multiply(const Mat<I>& first, const Mat<J>& second, Mat<K>& destination);

    /**
     * @brief Transposes the @p source inplace.
     */
    template <uint16_t N> void transpose(Mat<N>& source);

    /**
     * @brief Inverts the @p source matrix and places it in @p destination
     * matrix.
     */
    template <uint16_t N>
    void inverse(const Mat<N>& source, Mat<N>& destination);

    /**
     * @return The Frobenius norm of the @p source matrix.
     */
    template <uint16_t N> float norm(const Mat<N>& source);

    /**
     * @return The determinant of a 2x2 matrix.
     */
    float determinant(const Mat<4>& source);

    /**
     * @brief Matrix class with N data elements.
     */
    template <uint16_t N> struct Mat {

      protected:
        /**
         * @brief The matrix data.
         */
        float data[N];

        /**
         * @brief Variable used to utilize the ARM DSP library with this matrix.
         */
        arm_matrix_instance_f32 instance;

      public:
        /**
         * @brief Initialises the matrix. Note that @p rows * @p columns has to
         * be equal to the template parameter N.
         *
         * @param rows The number of rows in the matrix.
         * @param columns The number of columns in the matrix.
         */
        Mat(const uint16_t rows, const uint16_t columns) {
            arm_mat_init_f32(&instance, rows, columns, data);
            memset(data, 0, rows * columns * sizeof(float));
        }

        /**
         * @brief Initialises a matrix with only one column and amount of rows
         * equal to the template parameter.
         */
        Mat() : Mat(N, 1) {}

        /**
         * @brief Copy constructor for the matrix.
         */
        Mat(const Mat& matrix) {
            arm_mat_init_f32(&instance,
                             matrix.instance.numRows,
                             matrix.instance.numCols,
                             data);
            memcpy(data,
                   matrix.data,
                   matrix.instance.numRows * matrix.instance.numCols *
                       sizeof(float));
        }

        /**
         * @return Number of columns in the matrix.
         */
        uint16_t cols() const { return instance.numCols; }

        /**
         * @return Number of rows in the matrix.
         */
        uint16_t rows() const { return instance.numRows; }

        /**
         * @return The a reference to the [row, column] element of the matrix,
         * which thus can be assigned or modified.
         */
        float& operator()(const uint16_t row, const uint16_t column) {
            return data[row * instance.numCols + column];
        }

        /**
         * @return The value of the [row, column] element in the matrix.
         */
        float operator()(const uint16_t row, const uint16_t column) const {
            return data[row * instance.numCols + column];
        }

        friend void
        add<N>(const Mat<N>& first, const Mat<N>& second, Mat<N>& destination);

        template <uint16_t K, uint16_t J, uint16_t I>
        friend void multiply(const Mat<K>& first,
                             const Mat<J>& second,
                             Mat<I>& destination);

        friend void transpose<N>(Mat<N>& source);

        friend void inverse<N>(const Mat<N>& source, Mat<N>& destination);

        friend float norm<N>(const Mat<N>& source);

        void print() const {

            printf("(%d, %d)\r\n", instance.numRows, instance.numCols);

            for (uint16_t row = 0; row < instance.numRows; row++) {
                for (uint16_t column = 0; column < instance.numCols; column++) {
                    printf("%f ", data[row * instance.numCols + column]);
                }
                printf("\r\n");
            }
        }
    };

    template <uint16_t N>
    void add(const Mat<N>& first, const Mat<N>& second, Mat<N>& destination) {
#if DEBUG
        assert(arm_mat_add_f32(&first.instance,
                               &second.instance,
                               &destination.instance) == ARM_MATH_SUCCESS);
#else
        arm_mat_add_f32(&first.instance,
                        &second.instance,
                        &destination.instance);
#endif
    }

    template <uint16_t K, uint16_t J, uint16_t I>
    void
    multiply(const Mat<K>& first, const Mat<J>& second, Mat<I>& destination) {
#if DEBUG
        assert(arm_mat_mult_f32(&(first.instance),
                                &(second.instance),
                                &(destination.instance)) == ARM_MATH_SUCCESS);
#else
        arm_mat_mult_f32(&first.instance,
                         &second.instance,
                         &destination.instance);
#endif
    }

    template <uint16_t K, uint16_t J, uint16_t I>
    Mat<K> operator*(const Mat<J>& a, const Mat<I>& b) {

        Mat<K> result(a.instance.numRows, b.instance.numCols);

#if DEBUG
        assert(arm_mat_mult_f32(&a.instance, &b.instance, &result.instance) ==
               ARM_MATH_SUCCESS);
#else
        arm_mat_mult_f32(&a.instance, &b.instance, &result.instance);
#endif
        return result;
    }

    template <uint16_t N> void transpose(Mat<N>& source) {

        Mat<N> matrix(source.rows(), source.cols());

        memcpy(matrix.instance.pData,
               source.instance.pData,
               source.rows() * source.cols() * sizeof(float));

        source.instance.numCols = matrix.rows();
        source.instance.numRows = matrix.cols();

#if DEBUG
        assert(arm_mat_trans_f32(&matrix.instance, &source.instance) ==
               ARM_MATH_SUCCESS);
#else
        arm_mat_trans_f32(&matrix.instance, &source.instance);
#endif
    }

    template <uint16_t N>
    void inverse(const Mat<N>& source, Mat<N>& destination) {

        if (source.cols() == 2 && source.rows() == 2) {

            const float det = determinant(source);

            destination.data[0] = source.data[3] / det;
            destination.data[3] = source.data[0] / det;

            destination.data[1] = -source.data[1] / det;
            destination.data[2] = -source.data[2] / det;

            return;
        }

#if DEBUG

        assert(arm_mat_inverse_f32(&source.instance, &destination.instance) ==
               ARM_MATH_SUCCESS);

#else
        arm_mat_inverse_f32(&source.instance, &destination.instance);
#endif
    }
    template <uint16_t N> float norm(const Mat<N>& source) {
        float value = 0;

        const uint16_t rows = source.rows();
        const uint16_t cols = source.cols();

        for (uint16_t row = 0; row < rows; row++) {
            for (uint16_t col = 0; col < cols; col++) {
                const float element = fabs(source.data[row * cols + col]);
                value += element * element;
            }
        }

        return sqrt(value);
    }

    // ----------------------------- Vector ----------------------------------

    struct Vec2 {

        float x, y;

        Vec2();

        Vec2(float xs, float ys);

        float norm();

        Vec2 operator+(const Vec2& vector) const;

        Vec2 operator-(const Vec2& vector) const;
    };

    Vec2 operator*(const float alpha, const Vec2& vector);

}

#endif
