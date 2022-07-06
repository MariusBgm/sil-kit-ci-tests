// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/can/all.hpp"
#include "MockParticipant.hpp"

namespace {
    using namespace SilKit::Services::Can; 
    using SilKit::Core::Tests::DummyParticipant;

    MATCHER_P(CanFrameMatcher, controlFrame, "") {
        *result_listener << "matches can frames of the c-api to the cpp api";
        auto frame2 = controlFrame;
        SilKit::Services::Can::CanFrame frame;
        auto frame1 = arg;
        if (frame1.canId != frame2.id || frame1.dlc != frame2.dlc || frame1.dataField.size() != frame2.data.size) 
        {
            return false;
        }
        for (size_t i = 0; i < frame1.dataField.size(); i++)
        {
            if (frame1.dataField[i] != frame2.data.data[i]) {
                return false;
            }
        }
        if((frame1.flags.ide != 0) != ((frame2.flags & SilKit_CanFrameFlag_ide) != 0))
        {
            return false;
        }
        if ((frame1.flags.fdf != 0) != ((frame2.flags & SilKit_CanFrameFlag_fdf) != 0))
        {
            return false;
        }
        if ((frame1.flags.brs != 0) != ((frame2.flags & SilKit_CanFrameFlag_brs) != 0))
        {
            return false;
        }
        if ((frame1.flags.esi != 0) != ((frame2.flags & SilKit_CanFrameFlag_esi) != 0))
        {
            return false;
        }
        if ((frame1.flags.rtr != 0) != ((frame2.flags & SilKit_CanFrameFlag_rtr) != 0))
        {
            return false;
        }
        return true;
    }

    class MockCanController : public SilKit::Services::Can::ICanController
    {
    public:
        MOCK_METHOD(void, SetBaudRate, (uint32_t rate, uint32_t fdRate));
        MOCK_METHOD(void, Reset, ());
        MOCK_METHOD(void, Start, ());
        MOCK_METHOD(void, Stop, ());
        MOCK_METHOD(void, Sleep, ());
        MOCK_METHOD(CanTxId,SendFrame, (const CanFrame&, void*));
        MOCK_METHOD(SilKit::Services::HandlerId, AddFrameHandler, (FrameHandler, SilKit::Services::DirectionMask));
        MOCK_METHOD(void, RemoveFrameHandler, (SilKit::Services::HandlerId));
        MOCK_METHOD(SilKit::Services::HandlerId, AddStateChangeHandler, (StateChangeHandler));
        MOCK_METHOD(void, RemoveStateChangeHandler, (SilKit::Services::HandlerId));
        MOCK_METHOD(SilKit::Services::HandlerId, AddErrorStateChangeHandler, (ErrorStateChangeHandler));
        MOCK_METHOD(void, RemoveErrorStateChangeHandler, (SilKit::Services::HandlerId));
        MOCK_METHOD(SilKit::Services::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler, CanTransmitStatusMask));
        MOCK_METHOD(void, RemoveFrameTransmitHandler, (SilKit::Services::HandlerId));
    };

    void FrameTransmitHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanFrameTransmitEvent* /*ack*/)
    {
    }

    void FrameHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanFrameEvent* /*metaData*/)
    {
    }

    void StateChangeHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanStateChangeEvent* /*state*/)
    {
    }

    void ErrorStateChangeHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanErrorStateChangeEvent* /*state*/)
    {
    }

    class MockParticipant : public DummyParticipant
    {
    public:
        MOCK_METHOD2(CreateCanController, SilKit::Services::Can::ICanController*(const std::string& /*canonicalName*/,
                                               const std::string& /*networkName*/));
    };

    class CapiCanTest : public testing::Test
    {
    public:
        MockCanController mockController;
        CapiCanTest()
        {
        }
    };

    TEST_F(CapiCanTest, can_controller_function_mapping)
    {
        using SilKit::Util::HandlerId;

        SilKit_ReturnCode returnCode;
        MockParticipant mockParticipant;
        SilKit_HandlerId handlerId;

        EXPECT_CALL(mockParticipant, CreateCanController("ControllerName", "NetworkName")).Times(testing::Exactly(1));
        SilKit_CanController* testParam;
        returnCode = SilKit_CanController_Create(&testParam, (SilKit_Participant*)&mockParticipant, "ControllerName",
                                              "NetworkName");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SetBaudRate(123,456)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SetBaudRate((SilKit_CanController*)&mockController, 123, 456);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Start()).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_Start((SilKit_CanController*)&mockController);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Stop()).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_Stop((SilKit_CanController*)&mockController);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Sleep()).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_Sleep((SilKit_CanController*)&mockController);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrame(testing::_, testing::_)).Times(testing::Exactly(1));
        SilKit_CanFrame cf{ SilKit_InterfaceIdentifier_CanFrame, 0,0,0,{0,0} };
        cf.id = 1;
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_AddFrameHandler((SilKit_CanController*)&mockController, NULL, &FrameHandler,
                                                       SilKit_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveFrameHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveFrameHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddStateChangeHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_AddStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                             &StateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveStateChangeHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveStateChangeHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddErrorStateChangeHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_AddErrorStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                                  &ErrorStateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveErrorStateChangeHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveErrorStateChangeHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode =
            SilKit_CanController_AddFrameTransmitHandler((SilKit_CanController*)&mockController, NULL, &FrameTransmitHandler,
                                                      SilKit_CanTransmitStatus_Transmitted, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveFrameTransmitHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveFrameTransmitHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    }

    TEST_F(CapiCanTest, can_controller_send_frame_no_flags)
    {

        SilKit_ReturnCode returnCode;

        SilKit_CanFrame cf1{};
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = 0;
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_flags1)
    {

        SilKit_ReturnCode returnCode;

        SilKit_CanFrame cf1{};
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = SilKit_CanFrameFlag_ide | SilKit_CanFrameFlag_rtr | SilKit_CanFrameFlag_esi;
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_flags2)
    {

        SilKit_ReturnCode returnCode;

        SilKit_CanFrame cf1{};
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = SilKit_CanFrameFlag_fdf | SilKit_CanFrameFlag_brs;
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }


    TEST_F(CapiCanTest, can_controller_nullpointer_params)
    {
        SilKit_ReturnCode returnCode;
        SilKit_HandlerId handlerId;

        returnCode = SilKit_CanController_SetBaudRate(nullptr, 123, 456);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_CanController_Start(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_CanController_Stop(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_CanController_Sleep(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        SilKit_CanFrame cf{ SilKit_InterfaceIdentifier_CanFrame, 1,0,0,{0,0} };
        cf.id = 1;
        returnCode = SilKit_CanController_SendFrame(nullptr, &cf, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, nullptr, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode =
            SilKit_CanController_AddFrameHandler(nullptr, NULL, &FrameHandler, SilKit_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameHandler((SilKit_CanController*)&mockController, NULL, nullptr,
                                                       SilKit_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameHandler((SilKit_CanController*)&mockController, NULL, &FrameHandler,
                                                       SilKit_Direction_SendReceive, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_AddStateChangeHandler(nullptr, NULL, &StateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode =
            SilKit_CanController_AddStateChangeHandler((SilKit_CanController*)&mockController, NULL, nullptr, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                             &StateChangeHandler, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_AddErrorStateChangeHandler(nullptr, NULL, &ErrorStateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddErrorStateChangeHandler((SilKit_CanController*)&mockController, NULL, nullptr,
                                                                  &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddErrorStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                                  &ErrorStateChangeHandler, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_AddFrameTransmitHandler(nullptr, NULL, &FrameTransmitHandler,
                                                               SilKit_CanTransmitStatus_Canceled, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameTransmitHandler((SilKit_CanController*)&mockController, NULL, nullptr,
                                                               SilKit_CanTransmitStatus_Canceled, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameTransmitHandler(
            (SilKit_CanController*)&mockController, NULL, &FrameTransmitHandler, SilKit_CanTransmitStatus_Canceled, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_RemoveFrameHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_RemoveFrameTransmitHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_RemoveStateChangeHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_RemoveErrorStateChangeHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    }

}