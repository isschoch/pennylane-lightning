// Copyright 2021 Xanadu Quantum Technologies Inc.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
 * @file
 * Contains uncategorised utility functions.
 */
#pragma once

#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <memory>
#include <numeric>
#include <set>
#include <stdexcept>
#include <type_traits>

#include <iostream>

/// @cond DEV
#if __has_include(<cblas.h>) && defined _ENABLE_BLAS
#include <cblas.h>
#define USE_CBLAS 1
#else
#include <mutex>
#include <thread>
#define USE_CBLAS 0
#define DOTU_STD_CROSSOVER (1 << 20)
#define DOTC_STD_CROSSOVER (1 << 20)
static std::mutex barrier;
#endif
/// @endcond

namespace Pennylane {

namespace Util {

/**
 * @brief Compile-time scalar real times complex number.
 *
 * @tparam U Precision of real value `a`.
 * @tparam T Precision of complex value `b` and result.
 * @param a Real scalar value.
 * @param b Complex scalar value.
 * @return constexpr std::complex<T>
 */
template <class T, class U = T>
inline static constexpr std::complex<T> ConstMult(U a, std::complex<T> b) {
    return {a * b.real(), a * b.imag()};
}

/**
 * @brief Compile-time scalar complex times complex.
 *
 * @tparam U Precision of complex value `a`.
 * @tparam T Precision of complex value `b` and result.
 * @param a Complex scalar value.
 * @param b Complex scalar value.
 * @return constexpr std::complex<T>
 */
template <class T, class U = T>
inline static constexpr std::complex<T> ConstMult(std::complex<U> a,
                                                  std::complex<T> b) {
    return {a.real() * b.real() - a.imag() * b.imag(),
            a.real() * b.imag() + a.imag() * b.real()};
}
template <class T, class U = T>
inline static constexpr std::complex<T> ConstMultConj(std::complex<U> a,
                                                      std::complex<T> b) {
    return {a.real() * b.real() + a.imag() * b.imag(),
            -a.imag() * b.real() + a.real() * b.imag()};
}

/**
 * @brief Compile-time scalar complex summation.
 *
 * @tparam T Precision of complex value `a` and result.
 * @tparam U Precision of complex value `b`.
 * @param a Complex scalar value.
 * @param b Complex scalar value.
 * @return constexpr std::complex<T>
 */
template <class T, class U = T>
inline static constexpr std::complex<T> ConstSum(std::complex<U> a,
                                                 std::complex<T> b) {
    return a + b;
}

/**
 * @brief Return complex value 1+0i in the given precision.
 *
 * @tparam T Floating point precision type. Accepts `double` and `float`.
 * @return constexpr std::complex<T>{1,0}
 */
template <class T> inline static constexpr std::complex<T> ONE() {
    return {1, 0};
}

/**
 * @brief Return complex value 0+0i in the given precision.
 *
 * @tparam T Floating point precision type. Accepts `double` and `float`.
 * @return constexpr std::complex<T>{0,0}
 */
template <class T> inline static constexpr std::complex<T> ZERO() {
    return {0, 0};
}

/**
 * @brief Return complex value 0+1i in the given precision.
 *
 * @tparam T Floating point precision type. Accepts `double` and `float`.
 * @return constexpr std::complex<T>{0,1}
 */
template <class T> inline static constexpr std::complex<T> IMAG() {
    return {0, 1};
}

/**
 * @brief Returns sqrt(2) as a compile-time constant.
 *
 * @tparam T Precision of result. `double`, `float` are accepted values.
 * @return constexpr T sqrt(2)
 */
template <class T> inline static constexpr T SQRT2() {
    if constexpr (std::is_same_v<T, float>) {
        return {0x1.6a09e6p+0f};
    } else {
        return {0x1.6a09e667f3bcdp+0};
    }
}

/**
 * @brief Returns inverse sqrt(2) as a compile-time constant.
 *
 * @tparam T Precision of result. `double`, `float` are accepted values.
 * @return constexpr T 1/sqrt(2)
 */
template <class T> inline static constexpr T INVSQRT2() {
    return {1 / SQRT2<T>()};
}

/**
 * @brief Calculates 2^n for some integer n > 0 using bitshifts.
 *
 * @param n the exponent
 * @return value of 2^n
 */
inline size_t exp2(const size_t &n) { return static_cast<size_t>(1) << n; }

/**
 * @brief Log2 calculation.
 *
 * @param value Value to calculate for.
 * @return size_t
 */
inline size_t log2(size_t value) {
    return static_cast<size_t>(std::log2(value));
}

/**
 * @brief Calculates the decimal value for a qubit, assuming a big-endian
 * convention.
 *
 * @param qubitIndex the index of the qubit in the range [0, qubits)
 * @param qubits the number of qubits in the circuit
 * @return decimal value for the qubit at specified index
 */
inline size_t maxDecimalForQubit(size_t qubitIndex, size_t qubits) {
    assert(qubitIndex < qubits);
    return exp2(qubits - qubitIndex - 1);
}

/**
 * @brief Returns the number of wires supported by a given qubit gate.
 *
 * @tparam T Floating point precision type.
 * @param data Gate matrix data.
 * @return size_t Number of wires.
 */
template <class T> inline size_t dimSize(const std::vector<T> &data) {
    const size_t s = data.size();
    const size_t s_sqrt = static_cast<size_t>(std::floor(std::sqrt(s)));

    if (s < 4)
        throw std::invalid_argument("The dataset must be at least 2x2");
    if (((s == 0) || (s & (s - 1))))
        throw std::invalid_argument("The dataset must be a power of 2");
    if (s_sqrt * s_sqrt != s)
        throw std::invalid_argument("The dataset must be a perfect square");

    return static_cast<size_t>(log2(s_sqrt));
}

/**
 * @brief Partition [0, data_size] into n subset of size data_size/n.
 *
 * @param n Number of partitions.
 * @param data_size Size of data array.
 * @return std::vector of partitions indices.
 *
 * @note the last index sets to data_size.
 */
inline static std::vector<std::size_t> partition(std::size_t n,
                                                 std::size_t data_size) {
    std::vector<std::size_t> bnd;
    if (n == 1) {
        bnd.push_back(0);
        bnd.push_back(data_size);
        return bnd;
    }
    std::size_t d = data_size / n;
    std::size_t i, n1 = 0, n2 = 0;
    bnd.push_back(n1);
    for (i = 0; i < n; ++i) {
        n2 = n1 + d;
        if (i == n - 1)
            n2 = data_size;
        bnd.push_back(n2);
        n1 = n2;
    }
    return bnd;
}

/**
 * @brief Calculates the partial inner-product of input arrays naively.
 *
 * @tparam T Floating point precision type.
 * @param v1 Complex data array 1.
 * @param v2 Complex data array 2.
 * @param result Calculated inner-product of v1 and v2 for the range [0, r).
 * @param l lower-bound index in the for-loop.
 * @param r upper-bound index in the for-loop.
 */
template <class T>
inline static void
_innerProd(const std::complex<T> *v1, const std::complex<T> *v2,
           std::complex<T> &result, std::size_t l, std::size_t r) {
    std::complex<T> s{0, 0};
    for (std::size_t i = l; i < r; ++i)
        s += *(v1 + i) * *(v2 + i);

    std::lock_guard<std::mutex> block_threads_until_finish_this_job(barrier);
    result += s;
}

/**
 * @brief Calculates the inner-product using the best available method.
 *
 * @tparam T Floating point precision type.
 * @param v1 Complex data array 1.
 * @param v2 Complex data array 2.
 * @param data_size Size of data arrays.
 * @param nthreads Number of threads.
 * @return std::complex<T> Result of inner product operation.
 */
template <class T>
inline std::complex<T>
innerProd(const std::complex<T> *v1, const std::complex<T> *v2,
          const size_t data_size, const size_t nthreads = 2) {
    std::complex<T> result(0, 0);

    if constexpr (USE_CBLAS) {
        if constexpr (std::is_same_v<T, float>)
            cblas_cdotu_sub(data_size, v1, 1, v2, 1, &result);
        else if constexpr (std::is_same_v<T, double>)
            cblas_zdotu_sub(data_size, v1, 1, v2, 1, &result);
    } else {
        if (data_size > DOTU_STD_CROSSOVER) {
            result = std::inner_product(
                v1, v1 + data_size, v2, std::complex<T>(), ConstSum<T>,
                static_cast<std::complex<T> (*)(
                    std::complex<T>, std::complex<T>)>(&ConstMult<T>));
        } else {
            const std::vector<std::size_t> bnd = partition(nthreads, data_size);
            std::vector<std::thread> threads;
            for (std::size_t i = 0; i < nthreads; ++i)
                threads.push_back(std::thread(_innerProd<T>, std::ref(v1),
                                              std::ref(v2), std::ref(result),
                                              bnd[i], bnd[i + 1]));
            for (auto &h : threads)
                h.join();
        }
    }
    return result;
}

/**
 * @brief Calculates the partial inner-product of input arrays naively
 * with the the first dataset conjugated.
 *
 * @tparam T Floating point precision type.
 * @param v1 Complex data array 1.
 * @param v2 Complex data array 2.
 * @param result Calculated inner-product of v1 and v2 for the range [0, r).
 * @param l lower-bound index in the for-loop.
 * @param r upper-bound index in the for-loop.
 */
template <class T>
inline static void
_innerProdC(const std::complex<T> *v1, const std::complex<T> *v2,
            std::complex<T> &result, std::size_t l, std::size_t r) {
    std::complex<T> s{0, 0};
    for (std::size_t i = l; i < r; ++i)
        s += std::conj(*(v1 + i)) * *(v2 + i);

    std::lock_guard<std::mutex> block_threads_until_finish_this_job(barrier);
    result += s;
}

/**
 * @brief Calculates the inner-product using the best available method
 * with the first dataset conjugated.
 *
 * @tparam T Floating point precision type.
 * @param v1 Complex data array 1; conjugated before application.
 * @param v2 Complex data array 2.
 * @param data_size Size of data arrays.
 * @param nthreads Number of threads.
 * @return std::complex<T> Result of inner product operation.
 */
template <class T>
inline std::complex<T>
innerProdC(const std::complex<T> *v1, const std::complex<T> *v2,
           const size_t data_size, const size_t nthreads = 2) {
    std::complex<T> result(0, 0);

#if defined(USE_CBLAS) && USE_CBLAS
    if constexpr (std::is_same_v<T, float>)
        cblas_cdotc_sub(data_size, v1, 1, v2, 1, &result);
    else if constexpr (std::is_same_v<T, double>)
        cblas_zdotc_sub(data_size, v1, 1, v2, 1, &result);
#else
    if (data_size > DOTC_STD_CROSSOVER) {
        result = std::inner_product(v1, v1 + data_size, v2, std::complex<T>(),
                                    ConstSum<T>, ConstMultConj<T>);
    } else {
        const std::vector<std::size_t> bnd = partition(nthreads, data_size);
        std::vector<std::thread> threads;
        for (std::size_t i = 0; i < nthreads; ++i)
            threads.push_back(std::thread(_innerProdC<T>, std::ref(v1),
                                          std::ref(v2), std::ref(result),
                                          bnd[i], bnd[i + 1]));
        for (auto &h : threads)
            h.join();
    }
#endif
    return result;
}

/**
 * @brief Calculates the inner-product using the best available method.
 *
 * @see innerProd(const std::complex<T> *v1, const std::complex<T> *v2,
 * const size_t data_size)
 */
template <class T>
inline std::complex<T> innerProd(const std::vector<std::complex<T>> &v1,
                                 const std::vector<std::complex<T>> &v2) {
    return innerProd(v1.data(), v2.data(), v1.size());
}

/**
 * @brief Calculates the inner-product using the best available method with the
 * first dataset conjugated.
 *
 * @see innerProdC(const std::complex<T> *v1, const std::complex<T> *v2,
 * const size_t data_size)
 */
template <class T>
inline std::complex<T> innerProdC(const std::vector<std::complex<T>> &v1,
                                  const std::vector<std::complex<T>> &v2) {
    return innerProdC(v1.data(), v2.data(), v1.size());
}

/**
 * @brief Calculates the matrix-vector product partially.
 *
 * @tparam T Floating point precision type.
 * @param mat Complex data array repr. a flatten (row-wise) matrix m * n.
 * @param v_in Complex data array repr. a vector of shape n * 1.
 * @param v_out Pre-allocated complex data array to store the result of shape m
 * * 1.
 * @param m Number of rows of mat.
 * @param n Number of columns of mat.
 * @param left Lower-bound of the for-loop.
 * @param right Upper-bound of the for-loop.
 * @param transpose If `true`, considers transposed version of `mat`.
 */
template <class T>
inline static void
_matrixVecProd(const std::complex<T> *mat, const std::complex<T> *v_in,
               std::complex<T> *v_out, std::size_t m, std::size_t n,
               std::size_t left, std::size_t right, bool transpose = false) {
    std::size_t r, c;
    if (transpose) {
        for (r = left; r < right; ++r)
            for (c = 0; c < n; ++c)
                v_out[r] += mat[c * m + r] * v_in[c];
    } else {
        for (r = left; r < right; ++r)
            for (c = 0; c < n; ++c)
                v_out[r] += mat[r * n + c] * v_in[c];
    }
}

/**
 * @brief Calculates the matrix-vector product using the best available method.
 *
 * @tparam T Floating point precision type.
 * @param mat Complex data array repr. a flatten (row-wise) matrix m * n.
 * @param v_in Complex data array repr. a vector of shape n * 1.
 * @param v_out Pre-allocated complex data array to store the result.
 * @param m Number of rows of `mat`.
 * @param n Number of columns of `mat`.
 * @param nthreads Number of threads.
 * @param transpose If `true`, considers transposed version of `mat`.
 */
template <class T>
inline void matrixVecProd(const std::complex<T> *mat,
                          const std::complex<T> *v_in, std::complex<T> *v_out,
                          std::size_t m, std::size_t n,
                          std::size_t nthreads = 1, bool transpose = false) {
    if (!v_out)
        return;
#if defined(USE_CBLAS) && USE_CBLAS
    constexpr std::complex<T> co{1, 0};
    constexpr std::complex<T> cz{0, 0};
    const auto tr = (transpose) ? CblasTrans : CblasNoTrans;
    if constexpr (std::is_same_v<T, float>)
        cblas_cgemv(CblasRowMajor, tr, m, n, &co, mat, m, v_in, 1, &cz, v_out,
                    1);
    else if constexpr (std::is_same_v<T, double>)
        cblas_zgemv(CblasRowMajor, tr, m, n, &co, mat, m, v_in, 1, &cz, v_out,
                    1);
#else
    const std::vector<std::size_t> bnd = partition(nthreads, m);
    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < nthreads; ++i)
        threads.push_back(std::thread(_matrixVecProd<T>, std::ref(mat),
                                      std::ref(v_in), std::ref(v_out), m, n,
                                      bnd[i], bnd[i + 1], transpose));
    for (auto &h : threads)
        h.join();
#endif
}

/**
 * @brief Calculates transpose of a matrix recursively and Cache-Friendly
 * using blacking and Cache-optimized techniques.
 */
template <class T>
inline static void CFTranspose(const std::complex<T> *mat,
                               std::complex<T> *mat_t, std::size_t m,
                               std::size_t n, std::size_t m1, std::size_t m2,
                               std::size_t n1, std::size_t n2) {
    constexpr std::size_t BLOCK_THRESHOLD = 16;
    std::size_t r, s, r1, s1, r2, s2;
tail:
    r1 = m2 - m1;
    s1 = n2 - n1;
    if (r1 >= s1 && r1 > BLOCK_THRESHOLD) {
        r2 = (m1 + m2) / 2;
        CFTranspose(mat, mat_t, m, n, m1, r2, n1, n2);
        m1 = r2;
        goto tail;
    } else if (s1 > BLOCK_THRESHOLD) {
        s2 = (n1 + n2) / 2;
        CFTranspose(mat, mat_t, m, n, m1, m2, n1, s2);
        n1 = s2;
        goto tail;
    } else {
        for (r = m1; r < m2; ++r)
            for (s = n1; s < n2; ++s)
                mat_t[s * m + r] = mat[r * n + s];
    }
}

/**
 * @brief Transpose a matrix of size m * n to n * m using the
 * best available method.
 *
 *
 * @tparam T Floating point precision type.
 * @param mat Row-wise flatten matrix of size m * n.
 * @param mat_t Pre-allocated row-wise flatten matrix of size n * m.
 * @param m Number of rows of `mat`.
 * @param n Number of columns of `mat`.
 */
template <class T>
inline void Transpose(const std::complex<T> *mat, std::complex<T> *mat_t,
                      std::size_t m, std::size_t n) {
    CFTranspose(mat, mat_t, m, n, 0, m, 0, n);
}

template <class T>
inline static void
_matrixMatProd_tp(const std::complex<T> *m_left, const std::complex<T> *m_right,
                  std::complex<T> *m_out, std::size_t m, std::size_t n,
                  std::size_t k, std::size_t left, std::size_t right) {
    std::size_t r, c, b;
    for (r = left; r < right; ++r)
        for (c = 0; c < n; ++c)
            for (b = 0; b < k; ++b)
                m_out[r * n + c] += m_left[r * k + b] * m_right[c * n + b];
}

template <class T>
inline static void
_matrixMatProd(const std::complex<T> *m_left, const std::complex<T> *m_right,
               std::complex<T> *m_out, std::size_t m, std::size_t n,
               std::size_t k, std::size_t left, std::size_t right) {
    std::size_t r, c, b;
    for (r = left; r < right; ++r)
        for (c = 0; c < n; ++c)
            for (b = 0; b < k; ++b)
                m_out[r * n + c] += m_left[r * k + b] * m_right[b * n + c];
}

/**
 * @brief Calculates matrix-matrix product using the best avaiable method.
 *
 * @tparam T Floating point precision type.
 * @param m_left Row-wise flatten matrix of size m * k.
 * @param m_right Row-wise flatten matrix of size k * n.
 * @param m_out Pre-allocated row-wise flatten matrix of size m * n.
 * @param m Number of rows of `m_left`.
 * @param n Number of columns of `m_right`.
 * @param k Number of rows of `m_right`.
 * @param nthreads Number of threads.
 * @param transpose If `true`, considers transposed version of `mat`.
 */
template <class T>
inline void
matrixMatProd(const std::complex<T> *m_left, const std::complex<T> *m_right,
              std::complex<T> *m_out, std::size_t m, std::size_t n,
              std::size_t k, std::size_t nthreads = 1, bool transpose = false) {
    if (!m_out)
        return;
#if defined(USE_CBLAS) && USE_CBLAS
    constexpr std::complex<T> co{1, 0};
    constexpr std::complex<T> cz{0, 0};
    const auto tr = (transpose) ? CblasTrans : CblasNoTrans;
    if constexpr (std::is_same_v<T, float>)
        cblas_cgemm(CblasRowMajor, tr, CblasNoTrans, m, n, k, &co, m_left, k,
                    m_right, n, &cz, m_out, n);
    else if constexpr (std::is_same_v<T, double>)
        cblas_zgemm(CblasRowMajor, tr, CblasNoTrans, m, n, k, &co, m_left, k,
                    m_right, n, &cz, m_out, n);
#else
    std::vector<std::size_t> bnd = partition(nthreads, m);
    std::vector<std::thread> threads;
    if (transpose) {
        for (std::size_t i = 0; i < nthreads; ++i)
            threads.push_back(std::thread(_matrixMatProd<T>, std::ref(m_left),
                                          std::ref(m_right), std::ref(m_out), m,
                                          n, k, bnd[i], bnd[i + 1]));
    } else {
        for (std::size_t i = 0; i < nthreads; ++i)
            threads.push_back(std::thread(
                _matrixMatProd_tp<T>, std::ref(m_left), std::ref(m_right),
                std::ref(m_out), m, n, k, bnd[i], bnd[i + 1]));
    }
    for (auto &h : threads)
        h.join();
#endif
}

/**
 * @brief Streaming operator for vector data.
 *
 * @tparam T Vector data type.
 * @param os Output stream.
 * @param vec Vector data.
 * @return std::ostream&
 */
template <class T>
inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << '[';
    for (size_t i = 0; i < vec.size(); i++) {
        os << vec[i] << ",";
    }
    os << ']';
    return os;
}

/**
 * @brief Streaming operator for set data.
 *
 * @tparam T Vector data type.
 * @param os Output stream.
 * @param s Set data.
 * @return std::ostream&
 */
template <class T>
inline std::ostream &operator<<(std::ostream &os, const std::set<T> &s) {
    os << '{';
    for (const auto &e : s) {
        os << e << ",";
    }
    os << '}';
    return os;
}

/**
 * @brief Define linearly spaced data [start, end]
 *
 * @tparam T Data type.
 * @param start Start position.
 * @param end End position.
 * @param num_points Number of data-points in range.
 * @return std::vector<T>
 */
template <class T> std::vector<T> linspace(T start, T end, size_t num_points) {
    std::vector<T> data(num_points);
    T step = (end - start) / (num_points - 1);
    for (size_t i = 0; i < num_points; i++) {
        data[i] = start + (step * i);
    }
    return data;
}

/**
 * @brief Exception for functions that are not yet implemented.
 *
 */
class NotImplementedException : public std::logic_error {
  public:
    /**
     * @brief Construct a NotImplementedException exception object.
     *
     * @param fname Function name to indicate not imeplemented.
     */
    explicit NotImplementedException(const std::string &fname)
        : std::logic_error(std::string("Function is not implemented. ") +
                           fname){};
};

} // namespace Util
} // namespace Pennylane
