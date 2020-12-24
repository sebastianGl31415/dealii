// ---------------------------------------------------------------------
//
// Copyright (C) 1998 - 2018 by the deal.II authors
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



// check that SmartPointers preserve constness etc of the objects they
// point to, through assignment of SmartPointers to each other and
// other tests.


#include <deal.II/base/smartpointer.h>
#include <deal.II/base/subscriptor.h>

#include <iostream>
#include <vector>

#include "../tests.h"

// Provide memory for objects of type T such that access to a deleted
// object does not cause a segmentation fault
std::vector<char> memory(10000);
int               next = 0;

class Test : public Subscriptor
{
  const char *name;

public:
  Test(const char *n)
    : name(n)
  {
    deallog << "Construct " << name << std::endl;
  }
  ~Test()
  {
    deallog << "Destruct " << name << std::endl;
  }
  void
  f()
  {
    deallog << "mutable" << std::endl;
  }
  void
  f() const
  {
    deallog << "const" << std::endl;
  }
};



int
main()
{
  deal_II_exceptions::disable_abort_on_exception();

  initlog();

  Test        a("A");
  const Test &b("B");

  SmartPointer<Test, Test>       r(&a, "Test R");
  SmartPointer<const Test, Test> s(&a, "const Test S");
  //  SmartPointer<Test,Test>       t=&b;    // this one should not work
  SmartPointer<Test, Test>       t(const_cast<Test *>(&b), "Test T");
  SmartPointer<const Test, Test> u(&b, "const Test");


  deallog << "a ";
  a.f(); // should print "mutable", since #a# is not const
  deallog << "b ";
  b.f(); // should print "const", since #b# is const
  deallog << "r ";
  r->f(); // should print "mutable", since it points to the non-const #a#
  deallog << "s ";
  s->f(); // should print "const", since it points to the const #b#
  // but we made it const
  deallog << "t ";
  t->f(); // should print "mutable", since #b# is const, but
  // we casted the constness away
  deallog << "u ";
  u->f(); // should print "const" since #b# is const
  // Now try if subscriptor works
  Test c("C");
  r = &c;

  // Test that a dangling pointer is correctly detected.
  try
    {
      {
        Test d("D");
        r = &d;
      }
      const auto dummy = *r;
    }
  catch (const ExceptionBase &exc)
    {
      deallog << exc.get_exc_name() << std::endl;
    }
}
