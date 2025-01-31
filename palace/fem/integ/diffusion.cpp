// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "fem/integrator.hpp"

#include <vector>
#include <mfem.hpp>
#include "fem/libceed/coefficient.hpp"
#include "fem/libceed/integrator.hpp"

#include "fem/qfunctions/diffusion_qf.h"

namespace palace
{

struct DiffusionIntegratorInfo : public ceed::IntegratorInfo
{
  DiffusionContext ctx;
};

namespace
{

DiffusionIntegratorInfo
InitializeIntegratorInfo(const mfem::ParFiniteElementSpace &fespace,
                         const mfem::IntegrationRule &ir, const std::vector<int> &indices,
                         bool use_bdr, mfem::Coefficient *Q, mfem::VectorCoefficient *VQ,
                         mfem::MatrixCoefficient *MQ,
                         std::vector<ceed::QuadratureCoefficient> &coeff)
{
  MFEM_VERIFY(fespace.GetVDim() == 1,
              "libCEED interface for DiffusionIntegrator does not support vdim > 1!");

  DiffusionIntegratorInfo info = {{0}};

  mfem::ParMesh &mesh = *fespace.GetParMesh();
  info.ctx.dim = mesh.Dimension() - use_bdr;
  info.ctx.space_dim = mesh.SpaceDimension();

  info.trial_op = ceed::EvalMode::Grad;
  info.test_op = ceed::EvalMode::Grad;
  info.qdata_size = (info.ctx.dim * (info.ctx.dim + 1)) / 2;

  mfem::ConstantCoefficient *const_coeff = dynamic_cast<mfem::ConstantCoefficient *>(Q);
  if (const_coeff || !(Q || VQ || MQ))
  {
    info.ctx.coeff = const_coeff ? const_coeff->constant : 1.0;

    info.build_qf = f_build_diff_const_scalar;
    info.build_qf_path = PalaceQFunctionRelativePath(f_build_diff_const_scalar_loc);
  }
  else if (Q)
  {
    ceed::InitCoefficient(*Q, mesh, ir, indices, use_bdr, coeff.emplace_back());

    info.build_qf = f_build_diff_quad_scalar;
    info.build_qf_path = PalaceQFunctionRelativePath(f_build_diff_quad_scalar_loc);
  }
  else if (VQ)
  {
    MFEM_VERIFY(VQ->GetVDim() == info.ctx.space_dim,
                "Invalid vector coefficient dimension for DiffusionIntegrator!");
    ceed::InitCoefficient(*VQ, mesh, ir, indices, use_bdr, coeff.emplace_back());

    info.build_qf = f_build_diff_quad_vector;
    info.build_qf_path = PalaceQFunctionRelativePath(f_build_diff_quad_vector_loc);
  }
  else if (MQ)
  {
    MFEM_VERIFY(MQ->GetVDim() == info.ctx.space_dim,
                "Invalid matrix coefficient dimension for DiffusionIntegrator!");
    ceed::InitCoefficient(*MQ, mesh, ir, indices, use_bdr, coeff.emplace_back());

    info.build_qf = f_build_diff_quad_matrix;
    info.build_qf_path = PalaceQFunctionRelativePath(f_build_diff_quad_matrix_loc);
  }

  info.apply_qf = f_apply_diff;
  info.apply_qf_path = PalaceQFunctionRelativePath(f_apply_diff_loc);

  return info;
}

}  // namespace

void DiffusionIntegrator::Assemble(const mfem::ParFiniteElementSpace &trial_fespace,
                                   const mfem::ParFiniteElementSpace &test_fespace,
                                   const mfem::IntegrationRule &ir,
                                   const std::vector<int> &indices, Ceed ceed,
                                   CeedOperator *op, CeedOperator *op_t)
{
  MFEM_VERIFY(&trial_fespace == &test_fespace,
              "DiffusionIntegrator requires the same test and trial spaces!");
  constexpr bool use_bdr = false;
  std::vector<ceed::QuadratureCoefficient> coeff;
  const auto info =
      InitializeIntegratorInfo(trial_fespace, ir, indices, use_bdr, Q, VQ, MQ, coeff);
  ceed::AssembleCeedOperator(info, trial_fespace, test_fespace, ir, indices, use_bdr, coeff,
                             ceed, op, op_t);
}

void DiffusionIntegrator::AssembleBoundary(const mfem::ParFiniteElementSpace &trial_fespace,
                                           const mfem::ParFiniteElementSpace &test_fespace,
                                           const mfem::IntegrationRule &ir,
                                           const std::vector<int> &indices, Ceed ceed,
                                           CeedOperator *op, CeedOperator *op_t)
{
  MFEM_VERIFY(&trial_fespace == &test_fespace,
              "DiffusionIntegrator requires the same test and trial spaces!");
  constexpr bool use_bdr = true;
  std::vector<ceed::QuadratureCoefficient> coeff;
  const auto info =
      InitializeIntegratorInfo(trial_fespace, ir, indices, use_bdr, Q, VQ, MQ, coeff);
  ceed::AssembleCeedOperator(info, trial_fespace, test_fespace, ir, indices, use_bdr, coeff,
                             ceed, op, op_t);
}

}  // namespace palace
