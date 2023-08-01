#pragma once

#include <library/network/ip/socket_address.h>
#include <span>
#include <cstdint>
#include <vector>
#include <algorithm>


namespace maniscalco::network
{

    // TODO: this is junk for use during early development
    class packet
    {
    public:

        packet(std::size_t capacity):value_(capacity){}
        packet(std::span<char const> input)
        {
            value_.resize(input.size());
            std::copy_n(input.begin(), input.size(), value_.begin());
        }
        packet & operator = (std::span<char const> input)
        {
            value_.resize(input.size());
            std::copy_n(input.begin(), input.size(), value_.begin());
            return *this;
        }

        auto begin(){return value_.begin();}
        auto begin() const{return value_.begin();}
        auto end(){return value_.end();}
        auto end() const{return value_.end();}   
        auto size() const{return value_.size();}
        auto data(){return value_.data();}
        auto data() const{return value_.data();}
        auto resize(auto size){return value_.resize(size);}

    private:

        std::vector<char>   value_;
    };

}