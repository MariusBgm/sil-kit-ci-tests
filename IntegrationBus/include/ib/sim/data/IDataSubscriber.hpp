// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

class IDataSubscriber
{

public:
    virtual ~IDataSubscriber() = default;

    /*! \brief Set the default handler for data reception
     *
     * The handler is executed when data is received from a matching publisher.
     * The default handler will not be invoked if a specific is available.
     */
    virtual void SetDefaultDataMessageHandler(DataMessageHandlerT callback) = 0;

    /*! \brief Register a handler for data received by DataPublishers matching specific criteria
     * 
     * Specific data handlers can be used to distinguish publications on a single topic. If all of the labels provided
     * here appear in the labels of a DataPublisher, publications are redirected to the specific handler. A label key 
     * must match, an empty string in the value of a label given here is a wildcard. Likewise, the 
     * mediaType must match the one given by a DataPublisher. An empty string in the 
     * mediaType provided here also is a wildcard.
     */
    virtual void AddExplicitDataMessageHandler(DataMessageHandlerT callback,
                                               const std::string& mediaType,
                                               const std::map<std::string, std::string>& labels) = 0;
    
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace data
} // namespace sim
} // namespace ib
