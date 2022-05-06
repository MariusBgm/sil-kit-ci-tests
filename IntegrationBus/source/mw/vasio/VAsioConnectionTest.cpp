// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IVAsioPeer.hpp"
#include "IIbMessageReceiver.hpp"
#include "VAsioConnection.hpp"
#include "MockParticipant.hpp" // for DummyLogger

#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;
using namespace ib::mw;

using testing::Return;
using testing::ReturnRef;
using testing::_;

//Printing helpers, required for implicit template usage in VAsioConnection
namespace ib {
namespace mw {
namespace test {
namespace  version1 {
std::ostream& operator<<(std::ostream& out , const TestMessage& msg)
{
    out <<"version1:TestMessage{" << msg.integer <<",\"" << msg.str << "\"}";
    return out;
}
} // version1
inline namespace version2 {
std::ostream& operator<<(std::ostream& out , const TestMessage& msg)
{
    out <<"version2:TestMessage{" << msg.integer <<",\"" << msg.str << "\"}";
    return out;
}
}
std::ostream& operator<<(std::ostream& out , const TestFrameEvent& msg)
{
    out <<"TestFrameEvent{" << msg.integer <<",\"" << msg.str << "\"}";
    return out;
}

} //test
} //mw
} //ib

namespace {
struct MockIbMessageReceiver
    : public IIbMessageReceiver<test::version1::TestMessage>
    , public IIbMessageReceiver<test::version2::TestMessage>
    , public IIbMessageReceiver<test::TestFrameEvent>
    , public IIbServiceEndpoint
{
    ServiceDescriptor _serviceDescriptor;

    MockIbMessageReceiver()
    {
        _serviceDescriptor._serviceId = 1;
        _serviceDescriptor._participantId = 1;

        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }
    //IIbMessageReceiver<T>
    MOCK_METHOD(void, ReceiveIbMessage, (const ib::mw::IIbServiceEndpoint*, const test::version1::TestMessage&), (override));
    MOCK_METHOD(void, ReceiveIbMessage, (const ib::mw::IIbServiceEndpoint*, const test::version2::TestMessage&), (override));
    MOCK_METHOD(void, ReceiveIbMessage, (const ib::mw::IIbServiceEndpoint*, const test::TestFrameEvent&), (override));

    //IIbServiceEndpoint
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (override, const));
};


struct MockVAsioPeer
    : public IVAsioPeer
    , public IIbServiceEndpoint
    , public IVasioProtocolPeer
{
    VAsioPeerUri _peerUri;
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    MockVAsioPeer()
    {
        _peerUri.participantId = 1234;
        _peerUri.participantName = "MockVAsioPeer";
        _peerUri.acceptorUris.push_back("tcp://localhost:1234");
        _peerInfo.acceptorHost = "localhost";
        _peerInfo.acceptorPort = 1234;

        _serviceDescriptor._serviceId = 1;
        _serviceDescriptor._participantId = 1;

        ON_CALL(*this, GetLocalAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetRemoteAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetUri()).WillByDefault(ReturnRef(_peerUri));
        ON_CALL(*this, GetInfo()).WillByDefault(ReturnRef(_peerInfo));
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }

    //IVasioPeer
    MOCK_METHOD(void, SendIbMsg, (MessageBuffer), (override));
    MOCK_METHOD(void, Subscribe, (VAsioMsgSubscriber), (override));
    MOCK_METHOD(const VAsioPeerInfo&, GetInfo, (), (const, override));
    MOCK_METHOD(void, SetInfo, (VAsioPeerInfo), (override));
    MOCK_METHOD(void, SetUri, (VAsioPeerUri), (override));
    MOCK_METHOD(const VAsioPeerUri&, GetUri, (), (override, const));
    MOCK_METHOD(std::string, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(std::string, GetLocalAddress, (), (const, override));

    //IIbServiceEndpoint
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (override, const));

    //IVasioProtocolPeer
    MOCK_METHOD(void, VersionNotSupported, (), (override));
};

// Wire protocol, using the Serdes* methods
auto Serialize(const ParticipantAnnouncement& announcement) -> MessageBuffer
{
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncement
           << announcement;
    return buffer;
}

auto Serialize(const VAsioMsgSubscriber& subscriber) -> MessageBuffer
{
    ib::mw::MessageBuffer buffer;
    uint32_t rawMsgSize{0};
    buffer
        << rawMsgSize
        << VAsioMsgKind::SubscriptionAnnouncement
        << subscriber;

    return buffer;
}

auto Serialize(EndpointId remoteIndex, EndpointAddress address,
    const test::version1::TestMessage& msg) -> MessageBuffer
{
    ib::mw::MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};
    buffer
        << msgSizePlaceholder
        << VAsioMsgKind::IbSimMsg
        << remoteIndex
        << address
        << msg;
    return buffer;
}

template<typename IbMessageT>
auto Deserialize(MessageBuffer) -> IbMessageT;

template<>
auto Deserialize(MessageBuffer buffer) -> ParticipantAnnouncementReply
{
    VAsioMsgKind msgkind;
    RegistryMessageKind kind;
    ParticipantAnnouncementReply reply;
    buffer
        >> msgkind
        >> kind
        >> reply
        ;
    return reply;
}

template<>
auto Deserialize(MessageBuffer buffer) -> SubscriptionAcknowledge
{
    VAsioMsgKind msgkind;
    SubscriptionAcknowledge ack;
    buffer
        >> msgkind
        >> ack
        ;
    return ack;
}
void DropMessageSize(MessageBuffer& buffer)
{
    //After receiving we have to strip the message size, modifying the read position in the buffer
    uint32_t messageSize{0u};
    buffer >> messageSize;
}

//////////////////////////////////////////////////////////////////////
// Matchers
//////////////////////////////////////////////////////////////////////
MATCHER_P(AnnouncementReplyMatcher, announcement,
    "Deserialize the MessageBuffer and check the announcement's reply")
{
    auto buffer = arg;
    DropMessageSize(buffer);
    auto reply  = Deserialize<ParticipantAnnouncementReply>(buffer);
    //TODO sensible verification check
    return true;
}

MATCHER_P(SubscriptionAcknowledgeMatcher, subscriber,
    "Deserialize the MessageBuffer and check the subscriptions's ack")
{
    auto buffer = arg;
    DropMessageSize(buffer);
    auto reply  = Deserialize<SubscriptionAcknowledge>(buffer);
    return reply.status == SubscriptionAcknowledge::Status::Success
        && reply.subscriber == subscriber
        ;
}

} //namespace 

//////////////////////////////////////////////////////////////////////
// Test Fixture
//////////////////////////////////////////////////////////////////////

namespace ib {
namespace mw {
class VAsioConnectionTest : public testing::Test
{
protected:
    VAsioConnectionTest()
        : _connection({}, "VAsioConnectionTest", 1)
    {
        _connection.SetLogger(&_dummyLogger);
    }
    VAsioConnection _connection;
    MockVAsioPeer _from;
    test::DummyLogger _dummyLogger;

    //we are a friend class
    // - allow selected access to private member
    template<typename MessageT, typename ServiceT>
    void RegisterIbMsgReceiver(const std::string& networkName, ib::mw::IIbMessageReceiver<MessageT>* receiver)
    {
        _connection.RegisterIbMsgReceiver<MessageT, ServiceT>(networkName, receiver);
    }
};
} //mw
} //ib

//////////////////////////////////////////////////////////////////////
// Versioned initial handshake
//////////////////////////////////////////////////////////////////////

TEST_F(VAsioConnectionTest, unsupported_version_connect)
{
    ParticipantAnnouncement announcement{};
    announcement.peerUri = _from.GetUri();
    announcement.messageHeader.versionHigh = 1;

    auto buffer = Serialize(announcement);
    DropMessageSize(buffer);

    EXPECT_CALL(_from, VersionNotSupported()).Times(1);
    _connection.OnSocketData(&_from, std::move(buffer));

}

TEST_F(VAsioConnectionTest, current_version_connect)
{
    ParticipantAnnouncement announcement{}; //sets correct version in header
    announcement.peerUri = _from.GetUri();

    auto buffer = Serialize(announcement);
    DropMessageSize(buffer);

    EXPECT_CALL(_from, SendIbMsg(AnnouncementReplyMatcher(announcement))).Times(1);

    EXPECT_CALL(_from, VersionNotSupported()).Times(0);
    _connection.OnSocketData(&_from, std::move(buffer));
}

//////////////////////////////////////////////////////////////////////
// Versioned subscriptions: test backward compatibility
//////////////////////////////////////////////////////////////////////

//Disabled because we do not have a versioned Ser/Des tha detects
// the different version used for transmission.
TEST_F(VAsioConnectionTest, DISABLED_versioned_send_testmessage)
{
    // We send a 'version1', but expect to receive a 'version2' 
    test::version1::TestMessage message;
    message.integer = 1234;
    message.str ="1234";

    //Setup subscriptions for transmisison
    using MessageTrait = ib::mw::IbMsgTraits<decltype(message)>;
    VAsioMsgSubscriber subscriber;
    subscriber.msgTypeName = MessageTrait::SerdesName();
    subscriber.networkName = "unittest";
    subscriber.version = MessageTrait::Version();
    subscriber.receiverIdx = 0; //the first receiver

    EXPECT_EQ(subscriber.version, 1);

    // ReceiveSubscriptionAnnouncement -> sets internal structures up
    auto subscriberBuffer = Serialize(subscriber);
    DropMessageSize(subscriberBuffer);
    EXPECT_CALL(_from, SendIbMsg(SubscriptionAcknowledgeMatcher(subscriber))).Times(1);
    _connection.OnSocketData(&_from, std::move(subscriberBuffer));

    // Create a receiver with index 0 and a different TestMessage _version_
    MockIbMessageReceiver mockReceiver;
    RegisterIbMsgReceiver<test::version2::TestMessage,MockIbMessageReceiver>(subscriber.networkName, &mockReceiver);
    //the actual message

    auto buffer = Serialize(subscriber.receiverIdx,
        _from.GetServiceDescriptor().to_endpointAddress(), message);
    DropMessageSize(buffer);

    _connection.OnSocketData(&_from, std::move(buffer));
}