
//  (C) Copyright Edward Diener 2011
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#include "test_vm_mf_has_template_cp.hpp"
#include <boost/mpl/assert.hpp>

int main()
  {
  
#if !defined(BOOST_NO_VARIADIC_MACROS)

  using namespace boost::mpl::placeholders;
  
  // Wrong template types
  
  BOOST_MPL_ASSERT((boost::tti::mf_has_template_check_params
                      <
                      BOOST_TTI_TRAIT_GEN(WrongParametersForMP)<_>,
                      boost::mpl::identity<AnotherType>
                      >
                  ));
  
#else
  
  BOOST_MPL_ASSERT((boost::mpl::false_));
  
#endif //!defined(BOOST_NO_VARIADIC_MACROS)

  return 0;
  
  }
