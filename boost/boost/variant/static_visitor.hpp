//-----------------------------------------------------------------------------
// boost variant/static_visitor.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
// Eric Friedman
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee, 
// provided that the above copyright notice appears in all copies and 
// that both the copyright notice and this permission notice appear in 
// supporting documentation. No representations are made about the 
// suitability of this software for any purpose. It is provided "as is" 
// without express or implied warranty.

#ifndef BOOST_VARIANT_STATIC_VISITOR_HPP
#define BOOST_VARIANT_STATIC_VISITOR_HPP

#include "boost/config.hpp"
#include "boost/detail/workaround.hpp"

#include "boost/mpl/if.hpp"
#include "boost/type_traits/is_base_and_derived.hpp"

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
#   include "boost/type_traits/is_same.hpp"
#endif

// should be the last #include
#include "boost/type_traits/detail/bool_trait_def.hpp"

namespace boost {

//////////////////////////////////////////////////////////////////////////
// class template static_visitor
//
// An empty base class that typedefs the return type of a deriving static
// visitor. The class is analogous to std::unary_function in this role.
//

namespace detail {

    struct is_static_visitor_tag { };

#if !BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
    typedef void static_visitor_default_return;
#else
    typedef struct { } static_visitor_default_return;
#endif

} // namespace detail

template <typename R = ::boost::detail::static_visitor_default_return>
class static_visitor
    : public detail::is_static_visitor_tag
{
public: // typedefs

#if !BOOST_WORKAROUND(BOOST_MSVC, <= 1200)

    typedef R result_type;

#else // MSVC6

    typedef typename mpl::if_<
          is_same<R, detail::static_visitor_default_return>
        , void
        , R
        >::type result_type;

#endif // MSVC6 workaround

protected: // for use as base class only

    static_visitor() { }
    ~static_visitor() { }

};

//////////////////////////////////////////////////////////////////////////
// metafunction is_static_visitor
//
// Value metafunction indicates whether the specified type derives from
// static_visitor<...>.
//
// NOTE #1: This metafunction does NOT check whether the specified type
//  fulfills the requirements of the StaticVisitor concept.
//
// NOTE #2: This template never needs to be specialized!
//

namespace detail {

template <typename T>
struct is_static_visitor_impl
{
    BOOST_STATIC_CONSTANT(bool, value = 
        (::boost::is_base_and_derived< 
            detail::is_static_visitor_tag,
            T
        >::value));
};

} // namespace detail

BOOST_TT_AUX_BOOL_TRAIT_DEF1(
      is_static_visitor
    , T
    , (::boost::detail::is_static_visitor_impl<T>::value)
    )

} // namespace boost

#include "boost/type_traits/detail/bool_trait_undef.hpp"

#endif // BOOST_VARIANT_STATIC_VISITOR_HPP
