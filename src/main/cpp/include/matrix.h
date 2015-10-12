#ifndef MATRIX_H_
#define MATRIX_H_

#include <array>
#include <cmath>
#include <vector>

namespace cx3d {

class Matrix {
 public:
  /**
   * Returns the vector resulting from the addition of two vectors.
   *
   * <p>
   * There is no dimension check. It <code>a.length is smaller than b.length</code>, the subtraction will occur with
   * the first elements of vectB. In the opposite case, a IndexOutOfBoundsException will be thrown.
   *
   * @param a the first vector
   * @param b the second vector
   * @return vectA + vectB.
   *
   */
  template<std::size_t N>
  static std::array<double, N> add(const std::array<double, N>& a, const std::array<double, N>& b) {
    std::array<double, N> result;
    for (size_t i = 0; i < a.size(); i++) {
      result[i] = a[i] + b[i];
    }
    return result;
  }

  /**
   * Returns the cross product of two vectors.
   * <p>
   * There is no dimension check. If one of the vectors has more than 3 elements, only the 3 first
   * will be taken into account. If one vector is smaller, an <code>IndexOutOfBoundsException</code>
   * will be thrown.
   *
   * @param a
   * @param b
   * @return result the cross product of a and b (a x b)
   */
  template<std::size_t N>
  static std::array<double, N> crossProduct(const std::array<double, N>& a,
                                            const std::array<double, N>& b) {
    std::array<double, N> result;
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
    return result;
  }

  /**
   * Returns the determinant of a matrix.
   * <p>
   * For 3-by-3 or smaller matrices, the method performs an explicit computation based on the definition of the
   * determinant. For matrices of size bigger or equal to 4, a LR-decomposition with pivot is performed (the method
   * <code>detLR()</code> is called.
   * <p>
   * The determinant is only defined on a square matrix. But here is no dimension check. If the number of columns is bigger than the
   * number of lines of the second, the methods returns the determinant of a truncated matrix.
   * In the opposite case, an <code>IndexOutOfBoundsException</code> will be thrown.
   *
   * @param A the matrix
   * @return its determinant.
   *
   */
  template<std::size_t C, std::size_t R>
  static double det(const std::array<std::array<double, C>, R>& A) {
    int n = A.size();

    if (n == 1) {
      return A[0][0];
    }

    if (n == 2) {
      return A[0][0] * A[1][1] - A[1][0] * A[0][1];
    }

    if (n == 3) {
      return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
          - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
          + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
    } else {
      return Matrix::detLR(A);
    }
  }

  /**
   * Returns the determinant of a matrix, computed by a LR-decomposition with pivot.
   * For matrices 3-by-3 or smaller, the method <code>det()</code> is faster.
   * <p>
   * The determinant is only defined on a square matrix. But here is no dimension check.
   * If the number of columns is bigger than the number of lines of the second,
   * the methods returns the determinant of a truncated matrix.
   * In the opposite case, an <code>IndexOutOfBoundsException</code> will be thrown.
   *
   * @param A the matrix
   * @return its determinant.
   *
   */
  template<std::size_t Column, std::size_t Row>
  static double detLR(const std::array<std::array<double, Column>, Row>& A) {
    // based on the chapter IV of the course "Analyse numerique" taught by Pr Ernst Hairer at the University of Geneva
    size_t n = A.size();
    std::array<std::array<double, Column>, Row> R;  // array is a bit too large should be n*n (R*R)
    int nb_of_swaps = 1;
    // although we perform a LU decomposition with pivot, we don't need to store L and P

    // R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
    for (size_t i = 0; i < n; i++) {
      std::vector<double> row;
      for (int j = 0; j < n; j++) {
        R[i][j] = A[i][j];
      }
    }
    // Triangulation of R
    for (size_t i = 0; i < n - 1; i++) {
      // Find pivot and swap lines
      double a = std::abs(R[i][i]);
      for (size_t j = i + 1; j < n; j++) {
        if (std::abs(R[j][i]) > a) {
          a = std::abs(R[j][i]);
          auto temp = R[j];
          R[j] = R[i];
          R[i] = temp;
          nb_of_swaps = -nb_of_swaps;
        }
      }
      // Elimination of (i+i)th element of each line >i
      for (size_t j = i + 1; j < n; j++) {
        double l = R[j][i] / R[i][i];
        for (size_t k = i; k < n; k++) {
          R[j][k] = R[j][k] - l * R[i][k];
        }
      }
    }
    // Compute the det of the triangular Matrix R
    double determinant = nb_of_swaps;   // to keep the correct signe (inverted at each swap)
    for (size_t i = 0; i < n; i++) {
      determinant *= R[i][i];
    }
    return determinant;
  }

  /**
   * Computes the inner product (also called dot product) of two vectors.
   * @param a
   * @param b
   * @return a.b
   */
  template<std::size_t N>
  static double dot(const std::array<double, N>& a, const std::array<double, N>& b) {
    double product = 0;
    for (size_t i = 0; i < a.size(); i++) {
      product += a[i] * b[i];
    }
    return product;
  }

  /**
   * Returns the euclidean norm of a vector.
   * @param a vector
   * @param it's norm
   */
  template<std::size_t N>
  static double norm(const std::array<double, N>& a) {
    double norme = 0;
    for (size_t i = 0; i < a.size(); i++) {
      norme += a[i] * a[i];
    }
    norme = std::sqrt(norme);
    return norme;
  }

  /**
   * Normalizes a vector.
   * @param a a vector
   * @return the vector divided by its norm
   */
  template<std::size_t N>
  static std::array<double, N> normalize(const std::array<double, N>& a) {
    std::array<double, N> result;
    double norme = Matrix::norm(a);
    if (norme == 0.0) {
      norme = 1.0;
    }
    for (size_t i = 0; i < a.size(); i++) {
      result[i] = a[i] / norme;
    }
    return result;
  }

  /**
   * Multiplication of (all the elements of) a vector by a scalar value.
   *
   * @param  k a scalar
   * @param  a the vector we want to multiply
   * @return k*vectA
   */
  template<std::size_t N>
  static std::array<double, N> scalarMult(double k, const std::array<double, N>& a) {
    std::array<double, N> result;
    for (size_t i = 0; i < a.size(); i++) {
      result[i] = a[i] * k;
    }
    return result;
  }

  /**
   * Multiplication of (all the elements of) a matrix by a scalar value.
   *
   * @param  k a scalar
   * @param  A the matrix we want to multiply
   * @return k*matA
   */
  template<std::size_t C, std::size_t R>
  static std::array<std::array<double, C>, R> scalarMult(
      double k, const std::array<std::array<double, C>, R>& A) {
    std::array<std::array<double, C>, R> result;
    for (size_t i = 0; i < A.size(); i++) {
      for (size_t j = 0; j < A[i].size(); j++) {
        result[i][j] = k * A[i][j];
      }
    }
    return result;
  }

  /**
   * Returns the matrix resulting from the subtraction of two vectors. Without dimension check.
   * @param a
   * @param b
   * @return a-b
   */
  template<std::size_t N>
  static std::array<double, N> subtract(const std::array<double, N>& a,
                                        const std::array<double, N>& b) {
    std::array<double, N> result;
    int length = a.size();
    for (size_t i = 0; i < length; i++) {
      result[i] = a[i] - b[i];
    }
    return result;
  }
};

}  // namespace cx3d

#endif  // MATRIX_H_
