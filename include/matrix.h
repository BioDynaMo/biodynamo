#ifndef MATRIX_H_
#define MATRIX_H_

#include <array>
#include <vector>
#include <memory>

#include "math_util.h"
#include "simulation/ecm.h" //fixme remove after porting has been finished
#include "stl_util.h"

namespace bdm {

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
  static std::array<double, N> crossProduct(const std::array<double, N>& a, const std::array<double, N>& b) {
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
      return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
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
      for (size_t j = 0; j < n; j++) {
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
    norme = MathUtil::sqrt(norme);
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
  static std::array<std::array<double, C>, R> scalarMult(double k, const std::array<std::array<double, C>, R>& A) {
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
  static std::array<double, N> subtract(const std::array<double, N>& a, const std::array<double, N>& b) {
    std::array<double, N> result;
    size_t length = a.size();
    for (size_t i = 0; i < length; i++) {
      result[i] = a[i] - b[i];
    }
    return result;
  }

  /**
   * Returns the euclidean distance between two points. Equivalent to the 2-norm of (b-a).
   * @param a
   * @param b
   * @return ||a-b||
   */
  template<std::size_t N>
  static double distance(const std::array<double, N>& a, const std::array<double, N>& b) {
    return Matrix::norm(Matrix::subtract(a, b));
  }

  /**
   * Performs a rotation of a 3D vector a around a given axis b, in the positive mathematical sens.
   * As usual : there is no dimension check (only works with vectors of length 3), the arrays
   * given as argument are left unchanged.
   * <p>
   * <B>Example: </B>
   * <p>
   * <code>
   * double[] axis = {1.0,1.0,0.0};<br>
   * double[] vectToRotate = {4,5,6};<br>
   * double[] resultOfTheRotation = rotAroundAxis(vectToRotate, Math.PI, axis);<br>
   * </code>
   * @param a the vector we want to rotate
   * @param theta the amplitude of rotation (in radian)
   * @param b the axis (also a vector)
   * @return the vector after rotation
   */
  static std::array<double, 3> rotAroundAxis(const std::array<double, 3>& a, double theta,
                                             const std::array<double, 3>& b) {
    auto axis = normalize(b);

    auto temp_1 = scalarMult(dot(a, axis), axis);
    auto temp_2 = scalarMult(MathUtil::cos(-theta), subtract(a, temp_1));
    auto temp_3 = scalarMult(MathUtil::sin(-theta), crossProduct(a, axis));

    return {
      temp_1[0] + temp_2[0] + temp_3[0],
      temp_1[1] + temp_2[1] + temp_3[1],
      temp_1[2] + temp_2[2] + temp_3[2],
    };
  }

  /**
   * Solves a linear system of equation A*x = b of n equations with m unknowns. If n = m
   * an exact solution is given. If m is bigger than n, we look for a vector x of length n
   * s.t. ||A*x - b|| is minimal. (This method only works for coefficient matrices A with maximal rank;
   * this condition is not tested).
   * @param A
   * @param b
   * @return x
   */
  template<std::size_t N, std::size_t M>
  static std::array<double, N> solve(const std::array<std::array<double, N>, M>& A, const std::array<double, N>& b) {
    auto m = A.size();
    auto n = A[0].size();
    if (n < m) {
      throw std::logic_error("not yet implemented");
//      return solveQR(A, b);
    }
    if (n < 4) {
      return solveCramer(A, b);
    }
    return solveGauss(A, b);
  }

  /**
   * Solves a systems of linear equations A*x = b with Cramer's rule. The method is only defined
   * for n = 2 or 3. (As usual, there is no check of the dimensions or the rank of the matrix)..
   * @param A
   * @param b
   * @return x
   */
  template<std::size_t N, std::size_t M>
  static std::array<double, N> solveCramer(const std::array<std::array<double, N>, M>& A,
                                           const std::array<double, N>& b) {
    auto n = A.size();
    if (n == 2) {
      double a = A[0][0];
      double bb = A[0][1];
      double c = A[1][0];
      double d = A[1][1];
      double e = b[0];
      double f = b[1];
      double det = a * d - bb * c;
      return std::array<double, N>( { (e * d - bb * f) / det, (a * f - e * c) / det });
    }
    if (n == 3) {
      double a = A[0][0];
      double bb = A[0][1];
      double c = A[0][2];
      double d = A[1][0];
      double e = A[1][1];
      double f = A[1][2];
      double g = A[2][0];
      double h = A[2][1];
      double i = A[2][2];
      double j = b[0];
      double k = b[1];
      double l = b[2];

      double eihf = (e * i - h * f);
      double bich = (bb * i - c * h);
      double bfce = (bb * f - c * e);
      double det_1 = 1.0 / (a * eihf - d * bich + g * bfce);
      double x = det_1 * (j * eihf - k * bich + l * bfce);
      double y = det_1 * (a * (k * i - l * f) - d * (j * i - c * l) + g * (j * f - c * k));
      double z = det_1 * (a * (e * l - h * k) - d * (bb * l - h * j) + g * (k * bb - j * e));
      return std::array<double, N>( { x, y, z });
    } else {
      throw std::logic_error("not implemented yet - should return null");
//      return null;//fixme proper way to return null array
    }
  }

  /**
   * Solves a systems of linear equations A*x = b with Gaussian elimination.
   * For systems with n = 2 or 3
   * it is faster to use <code>solveCramer(A,b)</code>.
   * (As usual, there is no check of the dimensions or the rank of the matrix).
   * @param A
   * @param b
   * @return x
   */
  template<std::size_t N, std::size_t M>
  static std::array<double, N> solveGauss(const std::array<std::array<double, N>, M>& A,
                                          const std::array<double, N>& b) {
    // based on the chapter IV of the course "Analyse numerique" taught by Pr Ernst Hairer at the University of Geneva

    size_t length = A.size();
    std::array<std::array<double, M>, M> R;
    std::array<double, M> c;
    std::array<double, M> x;
    // although we perform a LU decomposition with pivot, we don't need to store L and P

    // R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
    // c = copy of b (same reason)
    for (size_t i = 0; i < length; i++) {
      c[i] = b[i];
      for (size_t j = 0; j < length; j++) {
        R[i][j] = A[i][j];
      }
    }
    // Triangulation of R
    for (size_t i = 0; i < length - 1; i++) {
      // Find pivot and swap lines
      double a = std::abs(R[i][i]);
      for (size_t j = i + 1; j < length; j++) {
        if (std::abs(R[j][i]) > a) {
          a = std::abs(R[j][i]);
          auto tempLine = R[j];
          R[j] = R[i];
          R[i] = tempLine;
          double tempc = c[j];
          c[j] = c[i];
          c[i] = tempc;
        }
      }
      // Elimination of (i+i)th element of each line >i
      for (size_t j = i + 1; j < length; j++) {
        double l = R[j][i] / R[i][i];
        c[j] -= l * c[i];
        for (auto k = i; k < length; k++) {
          R[j][k] = R[j][k] - l * R[i][k];
        }
      }

    }
    // Find x
    for (int i = length - 1; i > -1; i--) {
      double sum = 0.0;
      for (size_t j = i + 1; j < length; j++)
        sum += R[i][j] * x[j];
      x[i] = (c[i] - sum) / R[i][i];
    }

    return x;
  }

  /**
   * Returns the angle (in radian) between two vectors.
   *
   * @param a the first vector
   * @param b the second vector
   * @return the angle between them.
   */
  static double angleRadian(const std::array<double, 3>& a, const std::array<double, 3>& b) {
    double angle = MathUtil::acos(dot(a, b) / (norm(a) * norm(b)));
    return angle;
  }

  /**
   * Returns the projection of the first vector onto the second one.
   * @param a
   * @param b
   * @return the projection of a onto b
   */
  static std::array<double, 3> projectionOnto(const std::array<double, 3>& a, const std::array<double, 3>& b) {
    double k = dot(a, b) / dot(b, b);
    return scalarMult(k, b);
  }

  /**
   * Returns a vector of norm 1 perpendicular to a 3D vector. As usual there is not length check.
   * @param a
   * @return a vector perpendicular
   */
  static std::array<double, 3> perp3(const std::array<double, 3>& a, double rand) {
    std::array<double, 3> vect_perp;
    if (a[0] == 0.0) {
      vect_perp[0] = 1.0;
      vect_perp[1] = 0.0;
      vect_perp[2] = 0.0;
      vect_perp = rotAroundAxis(vect_perp, 6.35 * rand, a);
    } else {
      vect_perp[0] = a[1];
      vect_perp[1] = -a[0];
      vect_perp[2] = 0.0;
      vect_perp = normalize(vect_perp);
      vect_perp = rotAroundAxis(vect_perp, 6.35 * rand, a);
    }
    return vect_perp;
  }
};

}  // namespace bdm

#endif  // MATRIX_H_
