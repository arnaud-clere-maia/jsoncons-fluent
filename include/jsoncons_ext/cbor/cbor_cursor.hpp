// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_CURSOR_HPP
#define JSONCONS_CBOR_CBOR_CURSOR_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <istream> // std::basic_istream
#include <jsoncons/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>

namespace jsoncons { 
namespace cbor {

template<class Src=jsoncons::bin_stream_source,class Allocator=std::allocator<char>>
class basic_cbor_cursor : public basic_staj_reader<char>, private virtual ser_context
{
public:
    typedef Src source_type;
    typedef char char_type;
    typedef Allocator allocator_type;
private:
    basic_cbor_parser<Src,Allocator> parser_;
    basic_staj_event_handler<char_type> event_handler_;
    bool eof_;

    // Noncopyable and nonmoveable
    basic_cbor_cursor(const basic_cbor_cursor&) = delete;
    basic_cbor_cursor& operator=(const basic_cbor_cursor&) = delete;

public:
    typedef string_view string_view_type;

    template <class Source>
    basic_cbor_cursor(Source&& source,
                      const allocator_type& alloc = allocator_type())
       : parser_(std::forward<Source>(source), alloc), 
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    template <class Source>
    basic_cbor_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter,
                      const allocator_type& alloc = allocator_type())
       : parser_(std::forward<Source>(source), alloc), 
         event_handler_(filter), 
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    // Constructors that set parse error codes

    template <class Source>
    basic_cbor_cursor(Source&& source, std::error_code& ec)
        : basic_cbor_cursor(std::allocator_arg, allocator_type(),
                            std::forward<Source>(source), accept_all, ec)
    {
    }

    template <class Source>
    basic_cbor_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter,
                      std::error_code& ec)
        : basic_cbor_cursor(std::allocator_arg, allocator_type(),
                            std::forward<Source>(source), filter, ec)
    {
    }

    template <class Source>
    basic_cbor_cursor(std::allocator_arg_t, const allocator_type& alloc, 
                      Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter,
                      std::error_code& ec)
       : parser_(std::forward<Source>(source), alloc), 
         event_handler_(filter),
         eof_(false)
    {
        if (!done())
        {
            next(ec);
        }
    }

    bool done() const override
    {
        return parser_.done();
    }

    bool is_typed_array() const
    {
        return event_handler_.is_typed_array();
    }

    const basic_staj_event<char_type>& current() const override
    {
        return event_handler_.event();
    }

    void read(basic_json_content_handler<char_type>& handler) override
    {
        std::error_code ec;
        read(handler, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read(basic_json_content_handler<char_type>& handler,
              std::error_code& ec) override
    {
        if (!event_handler_.dump(handler, *this, ec))
        {
            return;
        }
        read_next(handler, ec);
    }

    void next() override
    {
        std::error_code ec;
        next(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void next(std::error_code& ec) override
    {
        read_next(ec);
    }

    void read_next(std::error_code& ec)
    {
        if (event_handler_.in_available())
        {
            event_handler_.send_available(ec);
        }
        else
        {
            parser_.restart();
            while (!parser_.stopped())
            {
                parser_.parse(event_handler_, ec);
                if (ec) return;
            }
        }
    }

    void read_next(basic_json_content_handler<char_type>& handler, std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.stopped())
        {
            parser_.parse(handler, ec);
            if (ec) return;
        }
    }
    const ser_context& context() const override
    {
        return *this;
    }

    bool eof() const
    {
        return eof_;
    }

    std::size_t line() const override
    {
        return parser_.line();
    }

    std::size_t column() const override
    {
        return parser_.column();
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use read(basic_json_content_handler<char_type>&)")
    void read_to(basic_json_content_handler<char_type>& handler)
    {
        read(handler);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use read(basic_json_content_handler<char_type>&, std::error_code&)")
    void read_to(basic_json_content_handler<char_type>& handler,
                 std::error_code& ec) 
    {
        read(handler, ec);
    }
#endif
private:
    static bool accept_all(const basic_staj_event<char_type>&, const ser_context&) 
    {
        return true;
    }
};

typedef basic_cbor_cursor<jsoncons::bin_stream_source> cbor_stream_cursor;
typedef basic_cbor_cursor<jsoncons::bytes_source> cbor_bytes_cursor;

} // namespace cbor
} // namespace jsoncons

#endif

