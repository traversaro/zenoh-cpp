//
// Copyright (c) 2024 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>

#pragma once

#include "base.hxx"

namespace zenoh {

class Session;
namespace detail {
class QueryableBase : public Owned<::z_owned_queryable_t> {
   protected:
    QueryableBase() : Owned(nullptr){};

    friend class zenoh::Session;
};
}  // namespace detail

/// A Zenoh queryable. Constructed by ``Session::declare_queryable`` method.
template <class Handler>
class Queryable : public detail::QueryableBase {
    Handler _handler;
    Queryable(detail::QueryableBase queryable, Handler handler)
        : QueryableBase(std::move(queryable)), _handler(std::move(handler)) {}

   public:
    /// @name Methods

    /// @brief Return handler to queryable data stream.
    const Handler& handler() const { return _handler; };

    friend class Session;
};

template <>
class Queryable<void> : public detail::QueryableBase {
    Queryable() : QueryableBase(){};

   public:
    using QueryableBase::QueryableBase;

    friend class Session;
};

}  // namespace zenoh