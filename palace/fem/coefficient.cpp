// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "coefficient.hpp"

namespace palace
{

void BdrGridFunctionCoefficient::GetElementTransformations(mfem::ElementTransformation &T,
                                                           const mfem::IntegrationPoint &ip,
                                                           mfem::ElementTransformation *&T1,
                                                           mfem::ElementTransformation *&T2,
                                                           mfem::Vector *C1)
{
  // Return transformations for elements attached to boundary element T. T1 always exists
  // but T2 may not if the element is truly a single-sided boundary.
  MFEM_ASSERT(T.ElementType == mfem::ElementTransformation::BDR_ELEMENT,
              "Unexpected element type in BdrGridFunctionCoefficient!");
  MFEM_ASSERT(&mesh == T.mesh, "Invalid mesh for BdrGridFunctionCoefficient!");
  int i, o;
  int iel1, iel2, info1, info2;
  mesh.GetBdrElementFace(T.ElementNo, &i, &o);
  mesh.GetFaceElements(i, &iel1, &iel2);
  mesh.GetFaceInfos(i, &info1, &info2);

  // Master faces can never be boundary elements, thus only need to check for the state of
  // info2 and el2, and do not need to access the ncface numbering. See mfem::Mesh::FaceInfo
  // for details.
  mfem::FaceElementTransformations *FET;
  if (info2 >= 0 && iel2 < 0)
  {
    // Face is shared with another subdomain.
    const int &ishared = local_to_shared.at(i);
    FET = mesh.GetSharedFaceTransformations(ishared);
  }
  else
  {
    // Face is either internal to the subdomain, or a true one-sided boundary.
    FET = mesh.GetFaceElementTransformations(i);
  }

  // Boundary elements and boundary faces may have different orientations so adjust the
  // integration point if necessary. See mfem::GridFunction::GetValue and GetVectorValue.
  mfem::IntegrationPoint fip =
      mfem::Mesh::TransformBdrElementToFace(FET->GetGeometryType(), o, ip);
  FET->SetAllIntPoints(&fip);
  T1 = &FET->GetElement1Transformation();
  T2 = (info2 >= 0) ? &FET->GetElement2Transformation() : nullptr;

  // If desired, get vector pointing from center of boundary element into element 1 for
  // orientations.
  if (C1)
  {
    mfem::Vector CF(T.GetSpaceDim());
    mfem::ElementTransformation &TF = *mesh.GetFaceTransformation(i);
    TF.Transform(mfem::Geometries.GetCenter(mesh.GetFaceGeometry(i)), CF);

    C1->SetSize(T.GetSpaceDim());
    T1->Transform(mfem::Geometries.GetCenter(T1->GetGeometryType()), *C1);
    *C1 -= CF;  // Points into element 1 from the face
  }
}

}  // namespace palace
