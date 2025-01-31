// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef PALACE_MODELS_CURL_CURL_OPERATOR_HPP
#define PALACE_MODELS_CURL_CURL_OPERATOR_HPP

#include <memory>
#include <vector>
#include <mfem.hpp>
#include "fem/fespace.hpp"
#include "linalg/operator.hpp"
#include "linalg/vector.hpp"
#include "models/materialoperator.hpp"
#include "models/surfacecurrentoperator.hpp"

namespace palace
{

class IoData;

//
// A class handling discretization of curl-curl problems for magnetostatics.
//
class CurlCurlOperator
{
private:
  const int pa_order_threshold;  // Order above which to use partial assembly vs. full
  const bool skip_zeros;         // Skip zeros during full assembly of matrices

  // Helper variable for log file printing.
  bool print_hdr;

  // Essential boundary condition markers.
  mfem::Array<int> dbc_marker;
  std::vector<mfem::Array<int>> dbc_tdof_lists;
  void CheckBoundaryProperties();

  // Objects defining the finite element spaces for the magnetic vector potential
  // (Nedelec) and magnetic flux density (Raviart-Thomas) on the given mesh. The H1 spaces
  // are used for various purposes throughout the code including postprocessing.
  std::vector<std::unique_ptr<mfem::ND_FECollection>> nd_fecs;
  std::vector<std::unique_ptr<mfem::H1_FECollection>> h1_fecs;
  std::unique_ptr<mfem::RT_FECollection> rt_fec;
  FiniteElementSpaceHierarchy nd_fespaces;
  AuxiliaryFiniteElementSpaceHierarchy h1_fespaces;
  AuxiliaryFiniteElementSpace rt_fespace;

  // Operator for domain material properties.
  MaterialOperator mat_op;

  // Operator for source current excitation.
  SurfaceCurrentOperator surf_j_op;

public:
  CurlCurlOperator(const IoData &iodata,
                   const std::vector<std::unique_ptr<mfem::ParMesh>> &mesh);

  // Return material operator for postprocessing.
  const MaterialOperator &GetMaterialOp() const { return mat_op; }

  // Access to underlying BC operator objects for postprocessing.
  const auto &GetSurfaceCurrentOp() const { return surf_j_op; }

  // Return the parallel finite element space objects.
  auto &GetNDSpaces() { return nd_fespaces; }
  const auto &GetNDSpaces() const { return nd_fespaces; }
  auto &GetNDSpace() { return nd_fespaces.GetFinestFESpace(); }
  const auto &GetNDSpace() const { return nd_fespaces.GetFinestFESpace(); }
  auto &GetH1Spaces() { return h1_fespaces; }
  const auto &GetH1Spaces() const { return h1_fespaces; }
  auto &GetH1Space() { return h1_fespaces.GetFinestFESpace(); }
  const auto &GetH1Space() const { return h1_fespaces.GetFinestFESpace(); }
  auto &GetRTSpace() { return rt_fespace; }
  const auto &GetRTSpace() const { return rt_fespace; }

  // Return the number of true (conforming) dofs on the finest ND space.
  auto GlobalTrueVSize() { return GetNDSpace().GlobalTrueVSize(); }

  // Construct and return system matrix representing discretized curl-curl operator for
  // Ampere's law.
  std::unique_ptr<Operator> GetStiffnessMatrix();

  // Construct and return the discrete curl matrix.
  const Operator &GetCurlMatrix() const { return GetRTSpace().GetDiscreteInterpolator(); }

  // Assemble the right-hand side source term vector for a current source applied on
  // specified excited boundaries.
  void GetExcitationVector(int idx, Vector &RHS);

  // Get the associated MPI communicator.
  MPI_Comm GetComm() const { return GetNDSpace().GetComm(); }
};

}  // namespace palace

#endif  // PALACE_MODELS_CURL_CURL_OPERATOR_HPP
