// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------



// test internal typetraits used in matrix_free.h

#include <deal.II/lac/la_parallel_block_vector.h>
#include <deal.II/lac/la_parallel_vector.h>
#include <deal.II/lac/trilinos_vector.h>

#include <deal.II/matrix_free/matrix_free.h>

#include "../tests.h"

int
main()
{
  initlog();

  deallog << "has_update_ghost_values_start:" << std::endl
          << "LinearAlgebra::distributed::Vector = "
          << internal::has_update_ghost_values_start<
               LinearAlgebra::distributed::Vector<double>>::value
          << std::endl
          << "TrilinosWrappers::MPI::Vector = "
          << internal::has_update_ghost_values_start<
               TrilinosWrappers::MPI::Vector>::value
          << std::endl
          << "Vector = "
          << internal::has_update_ghost_values_start<Vector<double>>::value
          << std::endl;

  deallog << "has_compress_start:" << std::endl
          << "LinearAlgebra::distributed::Vector = "
          << internal::has_compress_start<
               LinearAlgebra::distributed::Vector<double>>::value
          << std::endl
          << "TrilinosWrappers::MPI::Vector = "
          << internal::has_compress_start<TrilinosWrappers::MPI::Vector>::value
          << std::endl
          << "Vector = " << internal::has_compress_start<Vector<double>>::value
          << std::endl;

  deallog
    << "has_exchange_on_subset:" << std::endl
    << "LinearAlgebra::distributed::Vector = "
    << internal::has_exchange_on_subset<
         LinearAlgebra::distributed::Vector<double>>::value
    << std::endl
    << "TrilinosWrappers::MPI::Vector = "
    << internal::has_exchange_on_subset<TrilinosWrappers::MPI::Vector>::value
    << std::endl
    << "Vector = " << internal::has_exchange_on_subset<Vector<double>>::value
    << std::endl;


  deallog << "has_communication_block_size:" << std::endl
          << "LinearAlgebra::distributed::Vector = "
          << internal::has_communication_block_size<
               LinearAlgebra::distributed::Vector<double>>::value
          << std::endl
          << "LinearAlgebra::distributed::BlockVector = "
          << internal::has_communication_block_size<
               LinearAlgebra::distributed::BlockVector<double>>::value
          << std::endl;

  deallog << "is_serial_or_dummy:" << std::endl
          << "LinearAlgebra::distributed::Vector = "
          << internal::is_serial_or_dummy<
               LinearAlgebra::distributed::Vector<double>>::value
          << std::endl
          << "TrilinosWrappers::MPI::Vector = "
          << internal::is_serial_or_dummy<TrilinosWrappers::MPI::Vector>::value
          << std::endl
          << "Vector = " << internal::is_serial_or_dummy<Vector<double>>::value
          << std::endl
          << "unsigned int = "
          << internal::is_serial_or_dummy<unsigned int>::value << std::endl;

  // check that MatrixFree::cell_loop can run for non-vector types
  MatrixFree<2> matrix_free;
  int           dummy = 0;
  matrix_free.cell_loop(
    std::function<void(const MatrixFree<2> &,
                       int &,
                       const int &,
                       const std::pair<unsigned int, unsigned int> &)>(),
    dummy,
    dummy);

  deallog << "OK" << std::endl;
}
