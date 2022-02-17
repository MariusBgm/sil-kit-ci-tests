// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/mw/logging/ILogger.hpp"

#include "RpcClient.hpp"
#include "IServiceDiscovery.hpp"
#include "IComAdapterInternal.hpp"
#include "RpcDatatypeUtils.hpp"
#include "UuidRandom.hpp"

namespace ib {
namespace sim {
namespace rpc {

RpcClient::RpcClient(mw::IComAdapterInternal* comAdapter, cfg::RpcPort config, mw::sync::ITimeProvider* timeProvider, CallReturnHandler handler)
    : _comAdapter{ comAdapter }, _timeProvider{ timeProvider }, _handler{ std::move(handler) }, _logger{ comAdapter->GetLogger() }
{
    _config = std::move(config);
}


void RpcClient::RegisterServiceDiscovery()
{
    // The RpcClient discovers RpcServersInternal and is ready to detach calls afterwards
    _comAdapter->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
        [this](ib::mw::service::ServiceDiscoveryEvent::Type discoveryType,
               const ib::mw::ServiceDescriptor& serviceDescriptor) {

            std::string controllerType;
            if (!(serviceDescriptor.GetSupplementalDataItem(mw::service::controllerType, controllerType)
                  && controllerType == mw::service::controllerTypeRpcServerInternal))
            {
                return;
            }

            auto getVal = [serviceDescriptor](std::string key) {
                std::string tmp;
                if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                {
                    throw std::runtime_error{"Unknown key in supplementalData"};
                }
                return tmp;
            };

            auto clientUUID = getVal(mw::service::supplKeyRpcServerInternalClientUUID);

            if (clientUUID == _config.clientUUID)
            {
                if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated)
                {
                    _numCounterparts++;
                }
                else if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved)
                {
                    _numCounterparts--;
                }
            }
        });
}

auto RpcClient::Config() const -> const cfg::RpcPort&
{
    return _config;
}

IRpcCallHandle* RpcClient::Call(std::vector<uint8_t> data)
{
    if (_numCounterparts == 0)
    {
        if (_handler)
            _handler(this, nullptr, CallStatus::ServerNotReachable, {});
        return nullptr;
    }
    else
    {
        util::uuid::UUID uuid = util::uuid::generate();
        auto callUUID = CallUUID{uuid.ab, uuid.cd};
        auto callHandle = std::make_unique<CallHandleImpl>(callUUID);
        auto* callHandlePtr = callHandle.get();
        _detachedCallHandles[to_string(callUUID)] = std::make_pair(_numCounterparts, std::move(callHandle));
        FunctionCall msg{std::move(callUUID), std::move(data)};
        _comAdapter->SendIbMessage(this, std::move(msg));

        return callHandlePtr;
    }
}

IRpcCallHandle* RpcClient::Call(const uint8_t* data, std::size_t size)
{
    return Call({data, data + size});
}

void RpcClient::SetCallReturnHandler(CallReturnHandler handler)
{
    _handler = std::move(handler);
}

void RpcClient::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const FunctionCallResponse& msg)
{
    ReceiveMessage(msg);
}

void RpcClient::ReceiveMessage(const FunctionCallResponse& msg)
{
    auto it = _detachedCallHandles.find(to_string(msg.callUUID));
    if (it != _detachedCallHandles.end())
    {
        _handler(this, (*it).second.second.get(), CallStatus::Success, msg.data);
        // NB: Possibly multiple responses are received (e.g. 1 client, 2 servers). 
        // Decrease the count of responses and erase the call handle if all arrived.
        auto* numReceived = &(*it).second.first;
        (*numReceived)--;
        if (*numReceived <= 0)
            _detachedCallHandles.erase(it);
    }
    else
    {
        std::string errorMsg{"RpcClient: Received unknown function call response."};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
}

void RpcClient::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace rpc
} // namespace sim
} // namespace ib