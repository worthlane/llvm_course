#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <vector>

// task from my algorithms contest about dynamic programming

static const int64_t INF = 1e10;
static int64_t MOD = 1e9;

class BigInt {
 public:
  explicit BigInt(const std::string& s) {
    for (size_t i = 0; i < s.size(); i++) {
      digits_.push_back(s[i] - '0');
    }

    std::reverse(digits_.begin(), digits_.end());
  }

  explicit BigInt(int n) {
    while (n > 0) {
      digits_.push_back(n % 10);
      n /= 10;
    }
  }

  explicit BigInt() : BigInt(0) {}

  BigInt operator+(const BigInt& other) const {
    BigInt res;

    size_t max_len = std::max(digits_.size(), other.digits_.size());
    int remain = 0;

    for (size_t i = 0; (i < max_len) || remain; i++) {
      int diff = remain;

      if (i < digits_.size()) diff += digits_[i];

      if (i < other.digits_.size()) diff += other.digits_[i];

      res.digits_.push_back(diff % 10);

      remain = diff / 10;
    }

    return res;
  }

  BigInt operator-(const BigInt& other) const {
    BigInt res;

    assert(digits_.size() >= other.digits_.size());

    size_t max_len = std::max(digits_.size(), other.digits_.size());
    int remain = 0;

    for (size_t i = 0; i < max_len || remain; i++) {
      int diff = remain;

      if (i < digits_.size()) diff += digits_[i];

      if (i < other.digits_.size()) diff -= other.digits_[i];

      if (diff < 0) {
        diff += 10;
        remain = -1;
      } else {
        remain = 0;
      }

      res.digits_.push_back(diff);
    }

    while (res.digits_.size() > 1 && res.digits_.back() == 0)
      res.digits_.pop_back();

    return res;
  }

  BigInt operator*(const BigInt& other) const {
    BigInt res;

    res.digits_.resize(digits_.size() + other.digits_.size());

    for (size_t i = 0; i < digits_.size(); i++) {
      for (size_t j = 0; j < other.digits_.size(); j++) {
        res.digits_[i + j] += digits_[i] * other.digits_[j];
        res.digits_[i + j + 1] += res.digits_[i + j] / 10;
        res.digits_[i + j] %= 10;
      }
    }

    while (res.digits_.size() > 0 && res.digits_.back() == 0)
      res.digits_.pop_back();

    return res;
  }

  BigInt operator/(const int64_t other) const {
    BigInt res;

    int64_t remain = 0;

    for (int i = digits_.size() - 1; i >= 0; i--) {
      remain = remain * 10 + digits_[i];
      res.digits_.push_back(remain / other);
      remain %= other;
    }

    std::reverse(res.digits_.begin(), res.digits_.end());

    if (res.digits_.size() > 0 && res.digits_.back() == 0) res.digits_.pop_back();

    return res;
  }

  BigInt operator%(const int64_t other) {
    BigInt quotient(*this / other);

    return *this - (quotient * static_cast<BigInt>(other));
  }

  bool operator==(const BigInt& other) {
    if (digits_.size() != other.digits_.size()) return false;

    for (size_t i = 0; i < digits_.size(); i++) {
      if (digits_[i] != other.digits_[i]) return false;
    }

    return true;
  }

  bool operator!=(const BigInt& other) { return !(*this == other); }

  const std::vector<int>& get_digits() const { return digits_; }

private:
  std::vector<int> digits_;
};

std::ostream& operator<<(std::ostream& out, const BigInt& number) {

  const std::vector<int>& digits = number.get_digits();

  if (digits.size() == 0) {
    return out << '0';
  }

  for (int i = digits.size() - 1; i >= 0; i--) {
    out << static_cast<char>(digits[i] + '0');
  }

  return out;
}

std::istream& operator>>(std::istream& in, BigInt& number) {
  std::string s;
  in >> s;
  number = BigInt(s);
  return in;
}

template<typename T>
class Modular {
  public:
    explicit Modular(const T& value, const int64_t mod) : value_(value), mod_(mod) {
      value_ = value_ % mod;
    }

    Modular(const T& value) : value_(value) {}

    Modular operator+(const Modular& other) {
      return Modular((value_ + other.value_) % mod_, mod_);
    }

    Modular operator*(const Modular& other) {
      return Modular((value_ * other.value_) % mod_, mod_);
    }

    Modular operator/(const int64_t other) {
      return Modular((value_ / other) % mod_, mod_);
    }

    Modular operator-(const Modular& other) {
      return Modular((value_ - other.value_) % mod_, mod_);
    }

    T get_value() const { return value_; }

    T operator=(const T& other) { return value_ = other; }

  private:
    T value_;
    int64_t mod_ = MOD;

};

std::ostream& operator<<(std::ostream& out, const Modular<int64_t>& value) {
  return out << value.get_value();
}

template <typename T>
class Matrix {
 public:
  Matrix(const size_t rows, const size_t columns)
      : rows(rows), columns(columns), data(rows, std::vector<T>(columns, 0)) {}

  std::vector<T>& operator[](size_t i) { return data[i]; }

  Matrix operator*(const Matrix& other) {
    assert(columns == other.rows);

    Matrix res(rows, other.columns);

    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < other.columns; j++) {
        for (size_t k = 0; k < columns; k++) {
          res.data[i][j] = (res.data[i][j] + (data[i][k] * other.data[k][j]));
        }
      }
    }

    return res;
  }

  Matrix operator^(BigInt& power) {
    assert(rows == columns);

    Matrix res(rows, rows);
    for (size_t i = 0; i < rows; i++) res[i][i] = 1;

    Matrix a = *this;

    while (power != static_cast<BigInt>(0)) {
      if (power % 2 == static_cast<BigInt>(1)) res = res * a;

      a = a * a;
      power = power / 2;
    }

    return res;
  }

  size_t rows, columns;
  std::vector<std::vector<T>> data;
};

bool get_bit(size_t mask, size_t pos) { return (mask & (1 << pos)) != 0; }

bool has_transistion(size_t mask_1, size_t mask_2, size_t mask_len) {
  bool prev_bit_1 = get_bit(mask_1, 0);
  bool prev_bit_2 = get_bit(mask_2, 0);

  for (size_t i = 1; i < mask_len; i++) {
    bool cur_bit_1 = get_bit(mask_1, i);
    bool cur_bit_2 = get_bit(mask_2, i);

    if (cur_bit_1 == cur_bit_2 && prev_bit_1 == prev_bit_2 &&
        cur_bit_1 == prev_bit_1) {
      return false;
    }

    prev_bit_1 = cur_bit_1;
    prev_bit_2 = cur_bit_2;
  }
  return true;
}

Matrix<Modular<int64_t>> make_transistion_matrix(const size_t mask_len) {
  size_t max_profile = (1 << mask_len) - 1;

  Matrix<Modular<int64_t>> profile(max_profile + 1, max_profile + 1);

  for (size_t i = 0; i <= max_profile; i++) {
    for (size_t j = 0; j <= max_profile; j++) {
      if (has_transistion(i, j, mask_len))
        profile[i][j] = 1;
      else
        profile[i][j] = 0;
    }
  }

  return profile;
}

int main() {
  int64_t city_length = 0, mod = 0;
  BigInt  city_width;

  std::cin >> city_width >> city_length >> mod;
  MOD = mod;

  size_t max_profile = (1 << city_length) - 1;

  Matrix<Modular<int64_t>> profile = make_transistion_matrix(city_length);

  city_width = city_width - BigInt(1);

  Matrix<Modular<int64_t>> res = profile ^ city_width;

  Modular<int64_t> ans(0, mod);

  for (size_t i = 0; i <= max_profile; i++) {
    for (size_t j = 0; j <= max_profile; j++) {
      ans = ans + res[i][j];
    }
  }

  std::cout << ans << std::endl;
}
