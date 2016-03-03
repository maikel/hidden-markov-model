#ifndef ARRAY_HMM_H_
#define ARRAY_HMM_H_

#include <array>
#include <algorithm>
#include <exception>
#include <sstream>
#include <istream>

#include "gsl_util.h"

namespace nmb {

template<typename _Tp, std::size_t N, std::size_t M>
using matrix_type = std::array<std::array<_Tp, M>, N>;

struct matrix_read_error: public std::runtime_error {
    matrix_read_error(std::string arg) :
        std::runtime_error(arg)
    {
    }
};

struct matrix_write_error: public std::runtime_error {
    matrix_write_error(std::string arg) :
        std::runtime_error(arg)
    {
    }
};

}

template<typename _Tp, std::size_t N, std::size_t M>
std::istream& operator>>(std::istream& in, nmb::matrix_type<_Tp, N, M>& matrix)
{
  std::istringstream linestream;
  std::string line;
  if (!in)
    throw nmb::matrix_read_error("Can not read from an invalid stream.");
  for (std::size_t n = N; n > 0 && std::getline(in, line); --n) {
    linestream.str(line);
    linestream.clear();
    for (std::size_t k = 0; k < M; ++k)
      if (!(linestream >> matrix[N - n][k]))
        throw nmb::matrix_read_error(
            "Error while parsing the matrix line: " + line);
  }
  if (!in)
    throw nmb::matrix_read_error("Some error happened while reading a line.");

  return in;
}

template<typename _Tp, std::size_t N, std::size_t M>
std::ostream& operator<<(std::ostream& out,
    nmb::matrix_type<_Tp, N, M> const& matrix)
{
  if (!out)
    throw nmb::matrix_write_error("Can not wirte to an invalid output stream.");
  for (std::size_t n = 0; n < N; ++n) {
    for (std::size_t m = 0; m < M; ++m)
      out << matrix[n][m] << " ";
    out << "\n";
  }
  return out;
}

namespace nmb {

/**
 * @brief Checks if a given floating point value is nonnegative.
 */
template<typename _Tp, typename = typename std::enable_if<
    std::is_floating_point<_Tp>::value>::type>
inline bool is_nonnegative(_Tp value) noexcept
{
  return !(value < 0.0);
}

/**
 * @brief Checks if a given array is nonnegative and normed to one.
 */
template<typename _floatT, std::size_t N, std::size_t accuracy = 15,
    typename =
        typename std::enable_if<std::is_floating_point<_floatT>::value>::type>
inline bool is_probability_array(std::array<_floatT, N> const& p) noexcept
{
  bool entries_are_nonnegative = std::all_of(begin(p), end(p),
      is_nonnegative<_floatT>);
  _floatT total_sum = std::accumulate(begin(p), end(p), 0.0);
  bool array_is_normed =
      (std::abs(total_sum - 1.0) < pow10(-gsl::narrow<int>(accuracy)));
  return entries_are_nonnegative && array_is_normed;
}

/**
 * @brief Checks if every entry nonnegative and every row is normed to one.
 */
template<typename _floatT, std::size_t N, std::size_t M,
    std::size_t accuracy = 15, typename = typename std::enable_if<
        std::is_floating_point<_floatT>::value>::type>
inline bool is_right_stochastic_matrix(
    matrix_type<_floatT, N, M> const &matrix) noexcept
{
  return std::all_of(begin(matrix), end(matrix),
      is_probability_array<_floatT, M, accuracy>);
}

template<typename _tp>
class number_iterator: public std::iterator<std::input_iterator_tag, _tp> {
  public:
    number_iterator(_tp n) :
        current_number(n)
    {
    }
    _tp operator*() const noexcept
    {
      return current_number;
    }
    void operator++() noexcept
    {
      ++current_number;
    }
    void operator--() noexcept
    {
      --current_number;
    }
    bool operator==(const number_iterator& it) const noexcept
    {
      return it.current_number == current_number;
    }
  private:
    _tp current_number;
};

template<size_t N, size_t M, typename _Probtp = float>
class array_hmm {
  public:
    static constexpr size_t num_hidden_states = N;
    static constexpr size_t num_symbols = M;

    using size_type = std::size_t;
    using state_type = size_type;
    using symbol_type = size_type;
    using probability_type = _Probtp;
    using transition_matrix_type = matrix_type<probability_type, N, N>;
    using symbols_matrix_type = matrix_type<probability_type, N, M>;
    using state_distribution_type = std::array<probability_type, N>;

    struct probability_error: public std::runtime_error {
        probability_error(std::string what) :
            std::runtime_error(what)
        {
        }
    };

    array_hmm(
        transition_matrix_type const& A,
        symbols_matrix_type const& B,
        state_distribution_type const& initial) :
        m_A(A), m_B(B), m_pi(initial)
    {
      // check rows of transition matrix
      if (!is_right_stochastic_matrix(m_A) ||
      // check rows of symbol probability
          !is_right_stochastic_matrix(m_B) ||
          // check initial distribution
          !is_probability_array(m_pi))
        throw probability_error("Error while constructing a HMM.");
    }

    virtual ~array_hmm() = default;

    /**
     * Return the current probability to transition to hidden state `j` if
     * being in state `i`.
     */
    inline probability_type transition_probability(state_type i,
        state_type j) const noexcept
    {
      return m_A[i][j];
    }

    /**
     * Return the observation probability of symbol `k` under the restriction
     * of being in the hidden state `i`.
     */
    inline probability_type symbol_probability(state_type i,
        symbol_type k) const noexcept
    {
      return m_B[i][k];
    }

    /**
     * Return the probability to start in the hidden state `i`.
     */
    inline probability_type initial_probability(state_type i) const noexcept
    {
      return m_pi[i];
    }

    struct state_iterator: public number_iterator<state_type> {
        state_iterator(state_type n) :
            number_iterator<state_type>(n)
        {
        }
    };
    struct symbol_iterator: public number_iterator<symbol_type> {
        symbol_iterator(symbol_type m) :
            number_iterator<state_type>(m)
        {
        }
    };

    constexpr std::size_t num_states() const noexcept { return N; }
    constexpr std::size_t num_symbol() const noexcept { return M; }

    state_iterator begin_states() const noexcept
    {
      return state_iterator(0);
    }
    state_iterator end_states() const noexcept
    {
      return state_iterator(N);
    }
    symbol_iterator begin_symbols() const noexcept
    {
      return symbol_iterator(0);
    }
    symbol_iterator end_symbols() const noexcept
    {
      return symbol_iterator(M);
    }

  private:
    // Hidden Markov Model variables
    transition_matrix_type m_A;
    symbols_matrix_type m_B;
    state_distribution_type m_pi;
};

}

#endif /* ARRAY_HMM_H_ */
