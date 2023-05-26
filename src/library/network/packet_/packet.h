#pragma once

#include <span>
#include <cstdint>
#include <vector>
#include <algorithm>


namespace maniscalco::network
{

    class packet
    {
    public:

        packet(std::span<char const>):value_(value){}
/*
        packet & operator = (std::span<char const> input)
        {
            value_.resize(input.size());
            std::copy_n(input.begin(), input.size(), value_.begin());
            return *this;
        }
*/
        auto begin(){return value_.begin();}
        auto begin() const{return value_.begin();}
        auto end(){return value_.end();}
        auto end() const{return value_.end();}   
        auto size() const{return value_.size();}
        auto data(){return value_.data();}
        auto data() const{return value_.data();}

    private:

        std::span<char const>   value_;
    };

}