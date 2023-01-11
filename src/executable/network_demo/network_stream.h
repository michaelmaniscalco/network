#pragma once

#include <queue>
#include <vector>
#include <span>
#include <mutex>
#include <library/system.h>


namespace maniscalco
{

    class default_buffer_type
    {
    public:
        default_buffer_type() = default;
        default_buffer_type(std::span<char const> data):data_(data.begin(), data.end()){}
        auto begin() const{return data_.begin();}
        auto end() const{return data_.end();}
        auto size() const{return data_.size();}
        auto empty() const{return data_.empty();}
        operator std::span<char const>()const{return data_;}
    private:

        std::vector<char> data_;
    };


    template <network::socket_concept S, typename B = default_buffer_type>
    class network_stream
    {
    public:

        using socket_type = S;
        using buffer_type = B;

        network_stream
        (
            socket_type socket,
            system::work_contract_group & workContractGroup
        ):
            socket_(std::move(socket)),
            workContract_(workContractGroup.create_contract(
                    {
                        .contractHandler_ = [this](){this->process_packets();}
                    }))
        {
        }

        void send
        (
            std::span<char const> data
        )
        {
            std::lock_guard lockGuard(mutex_);
            packets_.emplace_back(data);
            workContract_.invoke();
        }

    private:

        void process_packets
        (
        )
        {
            std::lock_guard lockGuard(mutex_);
            if (!packets_.empty())
            {
                auto const & packet = packets_.front();
                socket_.send(packet);
                packets_.pop_front();
                if (!packets_.empty())
                    workContract_.invoke();
            }
        }

        socket_type socket_;

        system::work_contract workContract_;

        std::deque<buffer_type> packets_;

        std::mutex mutable      mutex_;
        
    }; // class network_stream

} // maniscalco
