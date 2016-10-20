#include <stdlib.h>
#include <string.h>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <armadillo>

enum matrix_majorness {
  row_major = 0,
  column_major
};

template <typename matrix_type>
struct matrix_traits {
  static const enum matrix_majorness majorness = row_major;

  static typename matrix_type::reference at(matrix_type &mat, size_t i, size_t j) {
    return mat(i, j);
  }

  static typename matrix_type::const_reference at(const matrix_type &mat, size_t i, size_t j) {
    return mat(i, j);
  }

  static typename matrix_type::pointer data(matrix_type &mat) {
    return mat.data();
  }

  static size_t size(const matrix_type &mat) {
    return mat.size();
  }

  static size_t m_dimension(const matrix_type &mat) {
    return mat.m();
  }

  static size_t n_dimension(const matrix_type &mat) {
    return mat.n();
  }
};


template <>
struct matrix_traits<arma::mat> {
private:
  typedef arma::mat matrix_type;

public:
  static const enum matrix_majorness majorness = column_major;

  static double &at(matrix_type &mat, size_t i, size_t j) {
    return mat(i, j);
  }

  static const double &at(const matrix_type &mat, size_t i, size_t j) {
    return mat(i, j);
  }

  static double *data(matrix_type &mat) {
    return mat.memptr();
  }

  static size_t size(const matrix_type &mat) {
    return mat.size();
  }

  static size_t m_dimension(const matrix_type &mat) {
    return mat.n_rows;
  }

  static size_t n_dimension(const matrix_type &mat) {
    return mat.n_cols;
  }
};


class Matrix {
  double *data_;
  size_t m_, n_;

public:
  typedef std::shared_ptr<Matrix> Ptr;

  inline Matrix(size_t _m, size_t _n): data_(new double[_m*_n]), m_(_m), n_(_n) {
    memset(data_, 0, m_*n_*sizeof(double));
  }
  Matrix(size_t _m, size_t _n, std::initializer_list<double> _data):
    Matrix(_m, _n) {
    if(_data.size() != m_*n_) {
      throw std::logic_error("imcompatible size");
    }
    size_t i = 0;
    for(auto &d: _data) {
      data_[i++] = d;
    }
  }

  ~Matrix() {
    delete[] data_;
  }

  inline double &operator() (size_t i, size_t j) {
    return data_[i*n_ + j];
  }

  inline const double &operator() (size_t i, size_t j) const {
    return data_[i*n_ + j];
  }

  inline double *data() {
    return data_;
  }

  inline size_t size() const {
    return m_ * n_;
  }

  inline size_t m() const {
    return m_;
  }

  inline size_t n() const {
    return n_;
  }

  inline double max() const {
    double max_ = -9999999999999;
    for(size_t i = 0; i < size(); ++i) {
      if(data_[i] > max_) {
        max_ = data_[i];
      }
    }
    return max_;
  }
};

template <typename type>
struct num_traits {
  const static bool is_num_type = false;
};

template <>
struct num_traits<double> {
  const static bool is_num_type = true;
};

template <typename num_type>
size_t base10_numstrlen(const num_type &val) {
  static_assert(num_traits<num_type>::is_num_type, "invalid number type for base 10");
  std::ostringstream os;
  os << val;
  return os.str().size();
}

int main() {

  Matrix::Ptr mat(new Matrix(3, 4, {
        1, 2, 3, 4,
          5, 6, 7, 8,
          9, 10, 11, 1200
      }));

  size_t max_numstrlen = base10_numstrlen(mat->max());
  for(int i = 0; i < 3; ++i) {
    for(int j = 0; j < 4; ++j) {
      std::cout << std::setw(max_numstrlen + 1) << (*mat)(i, j) << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}
