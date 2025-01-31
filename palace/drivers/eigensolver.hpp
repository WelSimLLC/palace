// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef PALACE_DRIVERS_EIGEN_SOLVER_HPP
#define PALACE_DRIVERS_EIGEN_SOLVER_HPP

#include <complex>
#include <memory>
#include <vector>
#include "drivers/basesolver.hpp"

namespace mfem
{

class ParMesh;

}  // namespace mfem

namespace palace
{

class ErrorIndicator;
class IoData;
class LumpedPortOperator;
class PostOperator;
class Timer;

//
// Driver class for eigenmode simulations.
//
class EigenSolver : public BaseSolver
{
private:
  void Postprocess(const PostOperator &postop, const LumpedPortOperator &lumped_port_op,
                   int i, std::complex<double> omega, double error_bkwd, double error_abs,
                   int num_conv, const ErrorIndicator *indicator) const;

  void PostprocessEigen(int i, std::complex<double> omega, double error_bkwd,
                        double error_abs, int num_conv) const;

  void PostprocessPorts(const PostOperator &postop,
                        const LumpedPortOperator &lumped_port_op, int i) const;

  void PostprocessEPR(const PostOperator &postop, const LumpedPortOperator &lumped_port_op,
                      int i, std::complex<double> omega, double Em) const;

  std::pair<ErrorIndicator, long long int>
  Solve(const std::vector<std::unique_ptr<mfem::ParMesh>> &mesh) const override;

public:
  using BaseSolver::BaseSolver;
};

}  // namespace palace

#endif  // PALACE_DRIVERS_EIGEN_SOLVER_HPP
