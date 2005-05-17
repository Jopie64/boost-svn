// (C) Copyright Jonathan Turkanis 2005.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#ifndef BOOST_IOSTREAMS_FILTER_TEST_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp> // Put size_t in std.
#include <algorithm>        // min.
#include <cstddef>          // size_t.
#include <cstdlib>          // rand.
#include <iterator>
#include <string>
#include <vector>
#if defined(__CYGWIN__)
# include <boost/random/linear_congruential.hpp>
# include <boost/random/uniform_smallint.hpp>
#endif
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/detail/adapter/basic_adapter.hpp>
#include <boost/iostreams/detail/bool_trait_def.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/iostreams/device/file.hpp>
#include <boost/range/iterator_range.hpp>

#ifdef BOOST_NO_STDC_NAMESPACE
# undef rand
# undef memcpy
# undef strlen
namespace std { using ::rand; using ::memcpy; using ::strlen; }
#endif

namespace boost { namespace iostreams {

BOOST_IOSTREAMS_BOOL_TRAIT_DEF(is_string, std::basic_string, 3)

const std::streamsize default_increment = 5;

#if defined(__CYGWIN__)
    std::streamsize rand(int inc)
    {
        static rand48                random_gen;
        static uniform_smallint<int> random_dist(0, inc);
        return random_dist(random_gen);
    }
#else
    std::streamsize rand(int inc) 
    { 
        return (std::rand() * inc + 1) / RAND_MAX; 
    }
#endif

class non_blocking_source {
public:
    typedef char char_type;
    struct io_category
        : source_tag,
          peekable_tag
        { };
    explicit non_blocking_source( const std::string& data, 
                                  std::streamsize inc = default_increment ) 
        : data_(data), inc_(inc), pos_(0)
        { }
    std::streamsize read(char* s, std::streamsize n)
    {
        if (pos_ == static_cast<std::streamsize>(data_.size()))
            return -1;
        std::streamsize avail = 
            (std::min) (n, static_cast<std::streamsize>(data_.size() - pos_));
        std::streamsize amt = (std::min) (rand(inc_), avail);
        if (amt)
            std::memcpy(s, data_.c_str() + pos_, amt);
        pos_ += amt;
        return amt;
    }

    bool putback(char c)
    {
        if (pos_ > 0) {
            data_[--pos_] = c;
            return true;
        }
        return false;
    }
private:
    std::string      data_;
    std::streamsize  inc_, pos_;
};

class non_blocking_sink : public sink {
public:
    non_blocking_sink( std::string& dest,
                       std::streamsize inc = default_increment ) 
        : dest_(dest), inc_(inc) 
        { }
    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = (std::min) (rand(inc_), n);
        dest_.insert(dest_.end(), s, s + amt);
        return amt;
    }
private:
    std::string&     dest_;
    std::streamsize  inc_;
};
                
//--------------Definition of test_input_filter-------------------------------//

template<typename Filter>
bool test_input_filter( Filter filter, 
                        const std::string& input, 
                        const std::string& output, 
                        mpl::true_ )
{
    for ( int inc = default_increment; 
          inc < default_increment * 40; 
          inc += default_increment )
    {
        non_blocking_source  src(input, inc);
        std::string          dest;
        iostreams::copy(compose(filter, src), iostreams::back_inserter(dest));
        if (dest != output)
            return false;
    }
    return true;
}

template<typename Filter, typename Source1, typename Source2>
bool test_input_filter( Filter filter, 
                        const Source1& input, 
                        const Source2& output, 
                        mpl::false_ )
{
    std::string in;
    std::string out;
    iostreams::copy(input, iostreams::back_inserter(in));
    iostreams::copy(output, iostreams::back_inserter(out));
    return test_input_filter(filter, in, out);
}

template<typename Filter, typename Source1, typename Source2>
bool test_input_filter( Filter filter, 
                        const Source1& input, 
                        const Source2& output )
{
    // Use tag dispatch to compensate for bad overload resolution.
    return test_input_filter( filter, input, output,    
                              is_string<Source1>() );
}

//--------------Definition of test_output_filter------------------------------//

template<typename Filter>
bool test_output_filter( Filter filter, 
                         const std::string& input, 
                         const std::string& output, 
                         mpl::true_ )
{
    for ( int inc = default_increment; 
          inc < default_increment * 40; 
          inc += default_increment )
    {
        array_source  src(input.data(), input.data() + input.size());
        std::string   dest;
        iostreams::copy(src, compose(filter, non_blocking_sink(dest, inc)));
        if (dest != output )
            return false;
    }
    return true;
}

template<typename Filter, typename Source1, typename Source2>
bool test_output_filter( Filter filter, 
                         const Source1& input, 
                         const Source2& output, 
                         mpl::false_ )
{
    std::string in;
    std::string out;
    iostreams::copy(input, iostreams::back_inserter(in));
    iostreams::copy(output, iostreams::back_inserter(out));
    return test_output_filter(filter, in, out);
}

template<typename Filter, typename Source1, typename Source2>
bool test_output_filter( Filter filter, 
                         const Source1& input, 
                         const Source2& output )
{
    // Use tag dispatch to compensate for bad overload resolution.
    return test_output_filter( filter, input, output,    
                               is_string<Source1>() );
}

//--------------Definition of test_filter_pair--------------------------------//

template<typename OutputFilter, typename InputFilter>
bool test_filter_pair( OutputFilter out, 
                       InputFilter in, 
                       const std::string& data, 
                       mpl::true_ )
{
    for ( int inc = default_increment; 
          inc <= default_increment * 40; 
          inc += default_increment )
    {
        array_source  src(data.data(), data.data() + data.size());
        std::string   temp;
        std::string   dest;
        iostreams::copy(src, compose(out, non_blocking_sink(temp, inc)));
        iostreams::copy( 
            compose(in, non_blocking_source(temp, inc)),
            iostreams::back_inserter(dest)
        );
        if (dest != data)
            return false;
    }
    return true;
}

template<typename OutputFilter, typename InputFilter, typename Source>
bool test_filter_pair( OutputFilter out, 
                       InputFilter in, 
                       const Source& data, 
                       mpl::false_ )
{
    std::string str;
    iostreams::copy(data, iostreams::back_inserter(str));
    return test_filter_pair(out, in, str);
}

template<typename OutputFilter, typename InputFilter, typename Source>
bool test_filter_pair( OutputFilter out, 
                       InputFilter in, 
                       const Source& data )
{
    // Use tag dispatch to compensate for bad overload resolution.
    return test_filter_pair(out, in, data, is_string<Source>());
}

} } // End namespaces iostreams, boost.

#endif // #ifndef BOOST_IOSTREAMS_FILTER_TEST_HPP_INCLUDED
