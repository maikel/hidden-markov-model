/*
 * Copyright 2016 Maikel Nadolski
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hidden-markov-models.t.h"

#include <vector>
#include <Eigen/Dense>
#include "hmm/hidden_markov_model.h"
#include "hmm/algorithm.h"

namespace {

CASE ( "Test forward algorithm for test case in Rabiners Paper" ) {
  Eigen::Matrix3f A;
  A << 0.4, 0.3, 0.3,
       0.2, 0.6, 0.2,
       0.1, 0.1, 0.8;
  Eigen::Matrix3f B;
  B << 1.0, 0.0, 0.0,
       0.0, 1.0, 0.0,
       0.0, 0.0, 1.0;
  Eigen::Vector3f pi;
  pi << 0.0, 0.0, 1.0;
  std::vector<int> sequence { 2, 2, 2, 0, 0, 2, 1, 2 };

  std::vector<float> scaling;
  std::vector<Eigen::VectorXf> alphas;
  maikel::hmm::hidden_markov_model<float> hmm(A, B, pi);
  forward(hmm, sequence, std::back_inserter(alphas), std::back_inserter(scaling));
  float probability = std::accumulate(scaling.begin(), scaling.end(), 1.0, std::multiplies<float>());
  probability = 1/probability;
  EXPECT(maikel::almost_equal<float>(probability,(1.536/10000),1));
}



CASE ( "Test forward and backward algorithms for test case in Rabiners Paper" ) {
  Eigen::Matrix3f A;
  A << 0.4, 0.3, 0.3,
       0.2, 0.6, 0.2,
       0.1, 0.1, 0.8;
  Eigen::Matrix3f B;
  B << 1.0, 0.0, 0.0,
       0.0, 1.0, 0.0,
       0.0, 0.0, 1.0;
  Eigen::Vector3f pi;
  pi << 0.0, 0.0, 1.0;
  std::vector<int> sequence { 2, 2, 2, 0, 0, 2, 1, 2 };

  maikel::hmm::hidden_markov_model<float> hmm(A, B, pi);

  using vector_type = decltype(hmm)::vector_type;
  std::vector<float> scaling;
  std::vector<vector_type> alphas;
  maikel::hmm::forward(hmm, sequence.begin(), sequence.end(), std::back_inserter(alphas), std::back_inserter(scaling));
  float probability = std::accumulate(scaling.begin(), scaling.end(), 1.0, std::multiplies<float>());
  probability = 1/probability;
  EXPECT(maikel::almost_equal<float>(probability,(1.536/10000),1));

  std::vector<vector_type> betas_ranges;
  maikel::hmm::backward(hmm,
      sequence | ranges::view::reverse,
       scaling | ranges::view::reverse,
      std::back_inserter(betas_ranges));
  std::vector<vector_type> betas_not_ranges;
  maikel::hmm::backward(hmm,
      sequence.rbegin(), sequence.rend(),
      scaling.rbegin(), scaling.rend(),
      std::back_inserter(betas_not_ranges));
}

CASE ( "baum-welch algorithm for test case in Rabiners Paper" ) {
  Eigen::Matrix3f A;
  A << 0.4, 0.3, 0.3,
       0.2, 0.6, 0.2,
       0.1, 0.1, 0.8;
  Eigen::Matrix3f B;
  B << 1.0, 0.0, 0.0,
       0.0, 1.0, 0.0,
       0.0, 0.0, 1.0;
  Eigen::Vector3f pi;
  pi << 0.0, 0.0, 1.0;
  std::vector<int> sequence { 2, 2, 2, 0, 0, 2, 1, 2 };

  maikel::hmm::hidden_markov_model<float> initial_hmm(A, B, pi);
  EXPECT_NO_THROW (
      auto new_hmm = maikel::hmm::naive::baum_welch(initial_hmm, sequence.begin(), sequence.end());
//      std::cerr << "A\n" << new_hmm.A << "\nB\n" << new_hmm.B << "\npi\n" << new_hmm.pi << std::endl;
  );
}


}


