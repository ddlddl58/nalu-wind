// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS), National Renewable Energy Laboratory, University of Texas Austin,
// Northwest Research Associates. Under the terms of Contract DE-NA0003525
// with NTESS, the U.S. Government retains certain rights in this software.
//
// This software is released under the BSD 3-clause license. See LICENSE file
// for more details.
//


#ifndef HYPREUVWLINEARSYSTEM_H
#define HYPREUVWLINEARSYSTEM_H

#include "HypreLinearSystem.h"
#include "stk_mesh/base/NgpMesh.hpp"

#include <vector>
#include <array>

namespace sierra {
namespace nalu {

class HypreUVWLinearSystem : public HypreLinearSystem
{
public:
  HypreUVWLinearSystem(
    Realm&,
    const unsigned numDof,
    EquationSystem*,
    LinearSolver*);

  virtual ~HypreUVWLinearSystem();

  virtual void zeroSystem();

  /** Update coefficients of a particular row(s) in the linear system
   *
   *  The core method of this class, it updates the matrix and RHS based on the
   *  inputs from the various algorithms. Note that, unlike TpetraLinearSystem,
   *  this method skips over the fringe points of Overset mesh and the Dirichlet
   *  nodes rather than resetting them afterward.
   *
   *  This overloaded method deals with Kernels designed with Kokkos::View arrays.
   *
   *  @param[in] numEntities The total number of nodes where data is to be updated
   *  @param[in] entities A list of STK node entities
   *
   *  @param[in] rhs Array containing RHS entries to be summed into
   *      [numEntities * numDof]
   *
   *  @param[in] lhs Array containing LHS entries to be summed into.
   *      [numEntities * numDof, numEntities * numDof]
   *
   *  @param[in] localIds Work array for storing local row IDs
   *  @param[in] sortPermutation Work array for sorting row IDs
   *  @param[in] trace_tag Debugging message
   */
  virtual void sumInto(
    unsigned numEntities,
    const stk::mesh::NgpMesh::ConnectedNodes& entities,
    const SharedMemView<const double*, DeviceShmem> & rhs,
    const SharedMemView<const double**, DeviceShmem> & lhs,
    const SharedMemView<int*, DeviceShmem> & localIds,
    const SharedMemView<int*, DeviceShmem> & sortPermutation,
    const char * trace_tag
  );

  /** Update coefficients of a particular row(s) in the linear system
   *
   *  The core method of this class, it updates the matrix and RHS based on the
   *  inputs from the various algorithms. Note that, unlike TpetraLinearSystem,
   *  this method skips over the fringe points of Overset mesh and the Dirichlet
   *  nodes rather than resetting them afterward.
   *
   *  This overloaded method deals with classic SupplementalAlgorithms
   *
   *  @param[in] sym_meshobj A list of STK node entities
   *  @param[in] scratchIds Work array for row IDs
   *  @param[in] scratchVals Work array for row entries
   *
   *  @param[in] rhs Array containing RHS entries to be summed into
   *      [numEntities * numDof]
   *
   *  @param[in] lhs Array containing LHS entries to be summed into.
   *      [numEntities * numDof * numEntities * numDof]
   *
   *  @param[in] trace_tag Debugging message
   */
  virtual void sumInto(
    const std::vector<stk::mesh::Entity> & sym_meshobj,
    std::vector<int> &scratchIds,
    std::vector<double> &scratchVals,
    const std::vector<double> & rhs,
    const std::vector<double> & lhs,
    const char *trace_tag
  );

  /** Populate the LHS and RHS for the Dirichlet rows in linear system
   */
  virtual void applyDirichletBCs(
    stk::mesh::FieldBase * solutionField,
    stk::mesh::FieldBase * bcValuesField,
    const stk::mesh::PartVector & parts,
    const unsigned beginPos,
    const unsigned endPos);

  virtual int solve(stk::mesh::FieldBase*);

  virtual unsigned numDof() const { return nDim_; }

protected:
  virtual void finalizeSolver();

  virtual void loadCompleteSolver();

  void copy_hypre_to_stk(stk::mesh::FieldBase*, std::vector<double>&);

private:
  std::vector<std::string> vecNames_{"X", "Y", "Z"};

  std::vector<double> scratchRowVals_;

  std::array<double, 3> firstNLR_;

  mutable std::vector<HYPRE_IJVector> rhs_;
  mutable std::vector<HYPRE_IJVector> sln_;

  const int nDim_{3};
};

}  // nalu
}  // sierra


#endif /* HYPREUVWLINEARSYSTEM_H */
