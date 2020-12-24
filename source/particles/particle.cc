// ---------------------------------------------------------------------
//
// Copyright (C) 2017 - 2020 by the deal.II authors
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

#include <deal.II/base/signaling_nan.h>

#include <deal.II/particles/particle.h>

DEAL_II_NAMESPACE_OPEN

namespace Particles
{
  template <int dim, int spacedim>
  Particle<dim, spacedim>::Particle()
    : location(numbers::signaling_nan<Point<spacedim>>())
    , reference_location(numbers::signaling_nan<Point<dim>>())
    , id(0)
    , property_pool(nullptr)
    , property_pool_handle(PropertyPool<dim, spacedim>::invalid_handle)
  {}



  template <int dim, int spacedim>
  Particle<dim, spacedim>::Particle(const Point<spacedim> &location,
                                    const Point<dim> &     reference_location,
                                    const types::particle_index id)
    : location(location)
    , reference_location(reference_location)
    , id(id)
    , property_pool(nullptr)
    , property_pool_handle(PropertyPool<dim, spacedim>::invalid_handle)
  {}



  template <int dim, int spacedim>
  Particle<dim, spacedim>::Particle(const Particle<dim, spacedim> &particle)
    : location(particle.get_location())
    , reference_location(particle.get_reference_location())
    , id(particle.get_id())
    , property_pool(particle.property_pool)
    , property_pool_handle(PropertyPool<dim, spacedim>::invalid_handle)
  {
    if (particle.has_properties())
      {
        property_pool_handle = property_pool->register_particle();
        const ArrayView<double> my_properties =
          property_pool->get_properties(property_pool_handle);
        const ArrayView<const double> their_properties =
          particle.get_properties();

        std::copy(their_properties.begin(),
                  their_properties.end(),
                  my_properties.begin());
      }
  }



  template <int dim, int spacedim>
  Particle<dim, spacedim>::Particle(
    const void *&                      data,
    PropertyPool<dim, spacedim> *const new_property_pool)
  {
    const types::particle_index *id_data =
      static_cast<const types::particle_index *>(data);
    id                  = *id_data++;
    const double *pdata = reinterpret_cast<const double *>(id_data);

    Point<spacedim> location;
    for (unsigned int i = 0; i < spacedim; ++i)
      location(i) = *pdata++;
    set_location(location);

    Point<dim> reference_location;
    for (unsigned int i = 0; i < dim; ++i)
      reference_location(i) = *pdata++;
    set_reference_location(reference_location);

    property_pool = new_property_pool;
    if (property_pool != nullptr)
      property_pool_handle = property_pool->register_particle();
    else
      property_pool_handle = PropertyPool<dim, spacedim>::invalid_handle;

    // See if there are properties to load
    if (has_properties())
      {
        const ArrayView<double> particle_properties =
          property_pool->get_properties(property_pool_handle);
        const unsigned int size = particle_properties.size();
        for (unsigned int i = 0; i < size; ++i)
          particle_properties[i] = *pdata++;
      }

    data = static_cast<const void *>(pdata);
  }



  template <int dim, int spacedim>
  Particle<dim, spacedim>::Particle(Particle<dim, spacedim> &&particle) noexcept
    : location(std::move(particle.location))
    , reference_location(std::move(particle.reference_location))
    , id(std::move(particle.id))
    , property_pool(std::move(particle.property_pool))
    , property_pool_handle(std::move(particle.property_pool_handle))
  {
    particle.property_pool_handle = PropertyPool<dim, spacedim>::invalid_handle;
  }



  template <int dim, int spacedim>
  Particle<dim, spacedim> &
  Particle<dim, spacedim>::operator=(const Particle<dim, spacedim> &particle)
  {
    if (this != &particle)
      {
        location           = particle.location;
        reference_location = particle.reference_location;
        id                 = particle.id;
        property_pool      = particle.property_pool;

        if (particle.has_properties())
          {
            property_pool_handle = property_pool->register_particle();
            const ArrayView<const double> their_properties =
              particle.get_properties();
            const ArrayView<double> my_properties =
              property_pool->get_properties(property_pool_handle);

            std::copy(their_properties.begin(),
                      their_properties.end(),
                      my_properties.begin());
          }
        else
          property_pool_handle = PropertyPool<dim, spacedim>::invalid_handle;
      }
    return *this;
  }



  template <int dim, int spacedim>
  Particle<dim, spacedim> &
  Particle<dim, spacedim>::
  operator=(Particle<dim, spacedim> &&particle) noexcept
  {
    if (this != &particle)
      {
        location             = particle.location;
        reference_location   = particle.reference_location;
        id                   = particle.id;
        property_pool        = particle.property_pool;
        property_pool_handle = particle.property_pool_handle;

        // We stole the rhs's properties, so we need to invalidate
        // the handle the rhs holds lest it releases the memory that
        // we still reference here.
        particle.property_pool_handle =
          PropertyPool<dim, spacedim>::invalid_handle;
      }
    return *this;
  }



  template <int dim, int spacedim>
  Particle<dim, spacedim>::~Particle()
  {
    if (property_pool != nullptr &&
        property_pool_handle != PropertyPool<dim, spacedim>::invalid_handle)
      property_pool->deregister_particle(property_pool_handle);
  }



  template <int dim, int spacedim>
  void
  Particle<dim, spacedim>::free_properties()
  {
    if (property_pool != nullptr &&
        property_pool_handle != PropertyPool<dim, spacedim>::invalid_handle)
      property_pool->deregister_particle(property_pool_handle);
  }



  template <int dim, int spacedim>
  void
  Particle<dim, spacedim>::write_data(void *&data) const
  {
    types::particle_index *id_data = static_cast<types::particle_index *>(data);
    *id_data                       = id;
    ++id_data;
    double *pdata = reinterpret_cast<double *>(id_data);

    // Write location data
    for (unsigned int i = 0; i < spacedim; ++i, ++pdata)
      *pdata = get_location()[i];

    // Write reference location data
    for (unsigned int i = 0; i < dim; ++i, ++pdata)
      *pdata = get_reference_location()[i];

    // Write property data
    if (has_properties())
      {
        const ArrayView<double> particle_properties =
          property_pool->get_properties(property_pool_handle);
        for (unsigned int i = 0; i < particle_properties.size(); ++i, ++pdata)
          *pdata = particle_properties[i];
      }

    data = static_cast<void *>(pdata);
  }



  template <int dim, int spacedim>
  void
  Particle<dim, spacedim>::update_particle_data(const void *&data)
  {
    const types::particle_index *id_data =
      static_cast<const types::particle_index *>(data);
    id                  = *id_data++;
    const double *pdata = reinterpret_cast<const double *>(id_data);

    Point<spacedim> location;
    for (unsigned int i = 0; i < spacedim; ++i)
      location(i) = *pdata++;
    set_location(location);

    Point<dim> reference_location;
    for (unsigned int i = 0; i < dim; ++i)
      reference_location(i) = *pdata++;
    set_reference_location(reference_location);

    // See if there are properties to load
    if (has_properties())
      {
        const ArrayView<double> particle_properties =
          property_pool->get_properties(property_pool_handle);
        const unsigned int size = particle_properties.size();
        for (unsigned int i = 0; i < size; ++i)
          particle_properties[i] = *pdata++;
      }

    data = static_cast<const void *>(pdata);
  }



  template <int dim, int spacedim>
  std::size_t
  Particle<dim, spacedim>::serialized_size_in_bytes() const
  {
    std::size_t size = sizeof(types::particle_index) + sizeof(location) +
                       sizeof(reference_location);

    if (has_properties())
      {
        const ArrayView<double> particle_properties =
          property_pool->get_properties(property_pool_handle);
        size += sizeof(double) * particle_properties.size();
      }
    return size;
  }



  template <int dim, int spacedim>
  void
  Particle<dim, spacedim>::set_properties(
    const ArrayView<const double> &new_properties)
  {
    Assert(property_pool != nullptr, ExcInternalError());

    // If we haven't allocated memory yet, do so now
    if (property_pool_handle == PropertyPool<dim, spacedim>::invalid_handle)
      property_pool_handle = property_pool->register_particle();

    const ArrayView<double> property_values =
      property_pool->get_properties(property_pool_handle);

    Assert(new_properties.size() == property_values.size(),
           ExcMessage(
             "You are trying to assign properties with an incompatible length. "
             "The particle has space to store " +
             std::to_string(property_values.size()) +
             " properties, but you are trying to assign " +
             std::to_string(new_properties.size()) +
             " properties. This is not allowed."));

    if (property_values.size() > 0)
      std::copy(new_properties.begin(),
                new_properties.end(),
                property_values.begin());
  }



  template <int dim, int spacedim>
  const ArrayView<double>
  Particle<dim, spacedim>::get_properties()
  {
    Assert(property_pool != nullptr, ExcInternalError());

    // If this particle has no properties yet, allocate and initialize them.
    if (property_pool_handle == PropertyPool<dim, spacedim>::invalid_handle)
      {
        property_pool_handle = property_pool->register_particle();

        ArrayView<double> my_properties =
          property_pool->get_properties(property_pool_handle);

        std::fill(my_properties.begin(), my_properties.end(), 0.0);
      }

    return property_pool->get_properties(property_pool_handle);
  }
} // namespace Particles

DEAL_II_NAMESPACE_CLOSE

DEAL_II_NAMESPACE_OPEN

#include "particle.inst"

DEAL_II_NAMESPACE_CLOSE
