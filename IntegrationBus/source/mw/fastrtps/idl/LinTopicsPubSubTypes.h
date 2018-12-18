// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*! 
 * @file LinTopicsPubSubTypes.h
 * This header file contains the declaration of the serialization functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#ifndef _IB_SIM_LIN_IDL_LINTOPICS_PUBSUBTYPES_H_
#define _IB_SIM_LIN_IDL_LINTOPICS_PUBSUBTYPES_H_

#include <fastrtps/config.h>
#include <fastrtps/TopicDataType.h>

#include "LinTopics.h"

#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error Generated LinTopics is not compatible with current installed Fast-RTPS. Please, regenerate it with fastrtpsgen.
#endif

namespace ib
{
    namespace sim
    {
        namespace lin
        {
            namespace idl
            {
                typedef uint8_t LinId;



                /*!
                 * @brief This class represents the TopicDataType of the type LinMessage defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class LinMessagePubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef LinMessage type;

                	LinMessagePubSubType();
                	virtual ~LinMessagePubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
                /*!
                 * @brief This class represents the TopicDataType of the type RxRequest defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class RxRequestPubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef RxRequest type;

                	RxRequestPubSubType();
                	virtual ~RxRequestPubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
                /*!
                 * @brief This class represents the TopicDataType of the type TxAcknowledge defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class TxAcknowledgePubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef TxAcknowledge type;

                	TxAcknowledgePubSubType();
                	virtual ~TxAcknowledgePubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
                /*!
                 * @brief This class represents the TopicDataType of the type WakeupRequest defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class WakeupRequestPubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef WakeupRequest type;

                	WakeupRequestPubSubType();
                	virtual ~WakeupRequestPubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };

                /*!
                 * @brief This class represents the TopicDataType of the type ControllerConfig defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class ControllerConfigPubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef ControllerConfig type;

                	ControllerConfigPubSubType();
                	virtual ~ControllerConfigPubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
                /*!
                 * @brief This class represents the TopicDataType of the type SlaveResponseConfig defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class SlaveResponseConfigPubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef SlaveResponseConfig type;

                	SlaveResponseConfigPubSubType();
                	virtual ~SlaveResponseConfigPubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
                /*!
                 * @brief This class represents the TopicDataType of the type SlaveConfiguration defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class SlaveConfigurationPubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef SlaveConfiguration type;

                	SlaveConfigurationPubSubType();
                	virtual ~SlaveConfigurationPubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
                /*!
                 * @brief This class represents the TopicDataType of the type SlaveResponse defined by the user in the IDL file.
                 * @ingroup LINTOPICS
                 */
                class SlaveResponsePubSubType : public eprosima::fastrtps::TopicDataType {
                public:
                        typedef SlaveResponse type;

                	SlaveResponsePubSubType();
                	virtual ~SlaveResponsePubSubType();
                	virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
                	virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;
                    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
                	virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
                		bool force_md5 = false) override;
                	virtual void* createData() override;
                	virtual void deleteData(void * data) override;
                	MD5 m_md5;
                	unsigned char* m_keyBuffer;
                };
            }
        }
    }
}

#endif // _IB_SIM_LIN_IDL_LINTOPICS_PUBSUBTYPES_H_