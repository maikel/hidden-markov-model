#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <array>
#include <vector>
#include <functional>

#include "hidden_markov_model.h"

#include <boost/iterator/function_input_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

std::ostream& operator<<(std::ostream& out, std::array<float, 3> const& alpha)
{
  copy(alpha.begin(), alpha.end(), std::ostream_iterator<float>(out, " "));
  return out;
}

int main(int argc, char *argv[])
{
  mnb::hmm::array::matrix<float,3,3> A;
  mnb::hmm::array::matrix<float,3,2> B;
  std::array<float,3> pi { 0.4, 0.4, 0.2 };
  std::ifstream A_in("A.dat");
  A_in >> A;
  std::ifstream B_in("B.dat");
  B_in >> B;
  mnb::hmm::array::hidden_markov_model<float,3,2> hmm(A, B, pi);

  const std::size_t obslen = 10;

  std::function<std::size_t()> generator = mnb::hmm::make_generator(hmm);
  std::vector<std::size_t> obs;
  std::copy(boost::make_function_input_iterator(generator, std::size_t{0}),
            boost::make_function_input_iterator(generator, obslen),
            std::back_inserter(obs));

  std::cout << obs.size() << "\n";
  std::copy(obs.begin(), obs.end(), std::ostream_iterator<std::size_t>(std::cout, " "));
  std::cout << std::endl;

  using array_type = decltype(hmm)::array_type;
  using float_type = decltype(hmm)::float_type;

  std::vector<std::pair<float_type, array_type>> alphas;
  hmm.forward(obs.begin(), obs.end(), std::back_inserter(alphas));



  ////  for (auto& p : alphas) {
////    std::cout << p.first << ": " << p.second << "\n";
////  }
//
//  auto logfirst = [](auto const& p) {
//    return -std::log(p.first);
//  };
//  auto scaling_begin =
//      boost::make_transform_iterator(alphas.begin(), logfirst);
//  auto scaling_end =
//      boost::make_transform_iterator(alphas.end(), logfirst);
//  float P = std::accumulate(scaling_begin, scaling_end, 0.0f);
//  std::cout << "log P(O | lambda) = " << P << std::endl;
//
//  std::cout << std::flush;
  return 0;
}

