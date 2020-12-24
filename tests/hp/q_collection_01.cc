// ---------------------------------------------------------------------
//
// Copyright (C) 2005 - 2018 by the deal.II authors
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



// test that QCollection objects are copyable without running into
// troubles when the copy is destroyed earlier than the original
// object


#include <deal.II/base/quadrature_lib.h>

#include <deal.II/hp/q_collection.h>

#include "../tests.h"



template <int dim>
void
test()
{
  hp::QCollection<dim> q_collection;
  q_collection.push_back(QGauss<dim>(2));
  q_collection.push_back(QGauss<dim>(3));

  // now create a copy and make sure
  // it goes out of scope before the
  // original
  {
    hp::QCollection<dim> copy(q_collection);
  }
}



int
main()
{
  initlog();
  deallog.get_file_stream().precision(2);

  test<1>();
  test<2>();
  test<3>();

  deallog << "OK" << std::endl;
}
