// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "integrator.hpp"

#include "fem/libceed/integrator.hpp"

namespace palace
{

void DiscreteInterpolator::Assemble(const mfem::ParFiniteElementSpace &trial_fespace,
                                    const mfem::ParFiniteElementSpace &test_fespace,
                                    const mfem::IntegrationRule &ir,
                                    const std::vector<int> &indices, Ceed ceed,
                                    CeedOperator *op, CeedOperator *op_t)
{
  // Interpolators do not use an integration rule to map between the test and trial spaces.
  ceed::AssembleCeedInterpolator(trial_fespace, test_fespace, indices, ceed, op, op_t);
}

void VectorFEBoundaryLFIntegrator::AssembleRHSElementVect(const mfem::FiniteElement &fe,
                                                          mfem::ElementTransformation &T,
                                                          mfem::Vector &elvect)
{
  const int dof = fe.GetDof();
  const int dim = fe.GetDim();
  const int q_order = fem::GetDefaultIntegrationOrder(fe, fe, T, q_extra);
  const mfem::IntegrationRule &ir = mfem::IntRules.Get(fe.GetGeomType(), q_order);
  f_hat.SetSize(dim);
  vshape.SetSize(dof, dim);
  elvect.SetSize(dof);
  elvect = 0.0;

  for (int i = 0; i < ir.GetNPoints(); i++)
  {
    const mfem::IntegrationPoint &ip = ir.IntPoint(i);
    T.SetIntPoint(&ip);
    fe.CalcVShape(ip, vshape);

    Q.Eval(f_loc, T, ip);
    T.InverseJacobian().Mult(f_loc, f_hat);
    f_hat *= ip.weight * T.Weight();
    vshape.AddMult(f_hat, elvect);
  }
}

void BoundaryLFIntegrator::AssembleRHSElementVect(const mfem::FiniteElement &fe,
                                                  mfem::ElementTransformation &T,
                                                  mfem::Vector &elvect)
{
  const int dof = fe.GetDof();
  const int q_order = fem::GetDefaultIntegrationOrder(fe, fe, T, q_extra);
  const mfem::IntegrationRule &ir = mfem::IntRules.Get(fe.GetGeomType(), q_order);
  shape.SetSize(dof);
  elvect.SetSize(dof);
  elvect = 0.0;

  for (int i = 0; i < ir.GetNPoints(); i++)
  {
    const mfem::IntegrationPoint &ip = ir.IntPoint(i);
    T.SetIntPoint(&ip);
    fe.CalcShape(ip, shape);

    double val = ip.weight * T.Weight() * Q.Eval(T, ip);
    elvect.Add(val, shape);
  }
}

}  // namespace palace
