// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_ARRAY_HPP
#define JSONCONS_JSON_ARRAY_HPP

#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <utility>
#include <initializer_list>
#include <jsoncons/json_text_traits.hpp>

namespace jsoncons {

template <class Json, class Allocator>
class json_array
{
public:
    typedef Allocator allocator_type;
    typedef Json value_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<json_array> self_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<value_type> vector_allocator_type;
    typedef typename std::vector<Json, Allocator>::reference reference;
    typedef typename std::vector<Json, Allocator>::const_reference const_reference;
    typedef typename std::vector<Json, Allocator>::iterator iterator;
    typedef typename std::vector<Json, Allocator>::const_iterator const_iterator;

    json_array()
        : self_allocator_(), 
          elements_(vector_allocator_type())
    {
    }

    explicit json_array(const Allocator& allocator)
        : self_allocator_(allocator), 
          elements_(vector_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const Allocator& allocator = Allocator())
        : self_allocator_(allocator), 
          elements_(n,Json(),vector_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const Json& value, 
                        const Allocator& allocator = Allocator())
        : self_allocator_(allocator), 
          elements_(n,value,vector_allocator_type(allocator))
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end, const Allocator& allocator = Allocator())
        : self_allocator_(allocator), 
          elements_(begin,end,vector_allocator_type(allocator))
    {
    }
    json_array(const json_array& val)
        : self_allocator_(val.get_self_allocator()),
          elements_(val.elements_)
    {
    }
    json_array(const json_array& val, const Allocator& allocator)
        : self_allocator_(allocator), 
          elements_(val.elements_,vector_allocator_type(allocator))
    {
    }

    json_array(json_array&& val) JSONCONS_NOEXCEPT
        : self_allocator_(val.get_self_allocator()), 
          elements_(std::move(val.elements_))
    {
    }
    json_array(json_array&& val, const Allocator& allocator)
        : self_allocator_(allocator), 
          elements_(std::move(val.elements_),vector_allocator_type(allocator))
    {
    }

    json_array(std::initializer_list<Json> init, 
               const Allocator& allocator = Allocator())
        : self_allocator_(allocator), 
          elements_(std::move(init),vector_allocator_type(allocator))
    {
    }
    ~json_array()
    {
    }

    self_allocator_type get_self_allocator() const
    {
        return self_allocator_;
    }

    void swap(json_array<Json, Allocator>& val)
    {
        elements_.swap(val.elements_);
    }

    size_t size() const {return elements_.size();}

    size_t capacity() const {return elements_.capacity();}

    void clear() {elements_.clear();}

    void shrink_to_fit() 
    {
        for (size_t i = 0; i < elements_.size(); ++i)
        {
            elements_[i].shrink_to_fit();
        }
        elements_.shrink_to_fit();
    }

    void reserve(size_t n) {elements_.reserve(n);}

    void resize(size_t n) {elements_.resize(n);}

    void resize(size_t n, const Json& val) {elements_.resize(n,val);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.begin()+from_index,elements_.begin()+to_index);
    }

    void erase(iterator first, iterator last) 
    {
        elements_.erase(first,last);
    }

    Json& operator[](size_t i) {return elements_[i];}

    const Json& operator[](size_t i) const {return elements_[i];}

    template <class T>
    void add(T&& value)
    {
        elements_.emplace_back(Json(std::forward<T&&>(value),get_self_allocator()));
    }

    void add(size_t index, const Json& value)
    {
        auto position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    void add(size_t index, Json&& value)
    {
        auto it = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(it, std::forward<Json&&>(value));
    }

#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
    // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
    template <class T>
    iterator add(const_iterator pos, T&& value)
    {
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.emplace(it, Json(std::forward<T&&>(value),get_self_allocator()));
    }
#else
    template <class T>
    iterator add(const_iterator pos, T&& value)
    {
        return elements_.emplace(pos, Json(std::forward<T&&>(value),get_self_allocator()));
    }
#endif

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<Json, Allocator>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (size_t i = 0; i < size(); ++i)
        {
            if (elements_[i] != rhs.elements_[i])
            {
                return false;
            }
        }
        return true;
    }
private:
    self_allocator_type self_allocator_;
    std::vector<Json,vector_allocator_type> elements_;

    json_array& operator=(const json_array<Json,Allocator>&) = delete;
};

}

#endif
