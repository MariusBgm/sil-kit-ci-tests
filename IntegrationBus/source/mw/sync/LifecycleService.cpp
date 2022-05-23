// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LifecycleService.hpp"
#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "LifecycleFSA.hpp"

#include <cassert>
#include <future>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {
LifecycleService::LifecycleService(IParticipantInternal* participant,
                                   const cfg::HealthCheck& healthCheckConfig)
    : _participant{participant}
    , _logger{participant->GetLogger()}
{
    _timeSyncService = _participant->CreateTimeSyncService(this);

    _status.participantName = _participant->GetParticipantName();
    _lifecycleManagement = std::make_shared<LifecycleManagement>(participant->GetLogger(), this);

    // TODO healthCheckConfig needed?
    (void)healthCheckConfig;
}


void LifecycleService::SetReinitializeHandler(ReinitializeHandlerT handler)
{
    _reinitializeHandler = std::move(handler);
}

void LifecycleService::SetStopHandler(StopHandlerT handler)
{
    _stopHandler = std::move(handler);
}

void LifecycleService::SetShutdownHandler(ShutdownHandlerT handler)
{
    _shutdownHandler = std::move(handler);
}

auto LifecycleService::ExecuteLifecycle(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                                        bool isRequiredParticipant) -> std::future<ParticipantState>
{
    _hasCoordinatedSimulationStart = hasCoordinatedSimulationStart;
    _hasCoordinatedSimulationStop = hasCoordinatedSimulationStop;
    _isRequiredParticipant = isRequiredParticipant;

    // Publish services
    auto serviceDiscovery = _participant->GetServiceDiscovery();
    serviceDiscovery->NotifyServiceCreated(GetServiceDescriptor());
    serviceDiscovery->NotifyServiceCreated(_timeSyncService->GetServiceDescriptor());

    _isRunning = true;
    _lifecycleManagement->InitLifecycleManagement("LifecycleService::ExecuteLifecycle() was called");
    //ChangeState(ParticipantState::Initialized, "LifecycleService::ExecuteLifecycle() was called");
    if (!hasCoordinatedSimulationStart)
    {
        _lifecycleManagement->Run("LifecycleService::ExecuteLifecycle() was called without start coordination");
        //ChangeState(ParticipantState::Running, "LifecycleService::ExecuteLifecycle() was called without start coordination");
    }
    return _finalStatePromise.get_future();
}

auto LifecycleService::ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                                bool isRequiredParticipant) -> std::future<ParticipantState>
{
    _timeSyncService->InitializeTimeSyncPolicy(false);

    return ExecuteLifecycle(hasCoordinatedSimulationStart, hasCoordinatedSimulationStop, isRequiredParticipant);
}

auto LifecycleService::ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
    -> std::future<ParticipantState>
{
    return ExecuteLifecycleNoSyncTime(hasCoordinatedSimulationStart, hasCoordinatedSimulationStop, false);
}

auto LifecycleService::ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService,
                                                    bool hasCoordinatedSimulationStart,
                                                    bool hasCoordinatedSimulationStop, bool isRequiredParticipant)
    -> std::future<ParticipantState>
{
    if (_timeSyncService != timeSyncService)
    {
        throw std::runtime_error("Failed to validate the provided timeSyncService pointer.");
    }

    _timeSyncService->InitializeTimeSyncPolicy(true);

    return ExecuteLifecycle(hasCoordinatedSimulationStart, hasCoordinatedSimulationStop, isRequiredParticipant);
}

auto LifecycleService::ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService,
                                                    bool hasCoordinatedSimulationStart,
                                                    bool hasCoordinatedSimulationStop) -> std::future<ParticipantState>
{
    return ExecuteLifecycleWithSyncTime(timeSyncService, hasCoordinatedSimulationStart, hasCoordinatedSimulationStop,
                                        false);
}

void LifecycleService::ReportError(std::string errorMsg)
{
    _logger->Error(errorMsg);

    if (State() == ParticipantState::Shutdown)
    {
        _logger->Warn("ParticipantController::ReportError() was called in terminal state ParticipantState::Shutdown; "
                      "transition to ParticipantState::Error is ignored.");
        return;
    }
    // TODO error message lost
    _lifecycleManagement->Error(std::move(errorMsg));
    //ChangeState(ParticipantState::Error, std::move(errorMsg));
}

void LifecycleService::Pause(std::string reason)
{
    if (State() != ParticipantState::Running)
    {
        const std::string errorMessage{"TimeSyncService::Pause() was called in state ParticipantState::"
                                       + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }
    _pauseDonePromise = decltype(_pauseDonePromise){};
    _timeSyncService->SetPaused(_pauseDonePromise.get_future());
    _lifecycleManagement->Pause(reason);
    //ChangeState(ParticipantState::Paused, std::move(reason));
}

void LifecycleService::Continue()
{
    if (State() != ParticipantState::Paused)
    {
        const std::string errorMessage{"TimeSyncService::Continue() was called in state ParticipantState::"
                                       + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }

    _lifecycleManagement->Continue("Pause finished");
    //ChangeState(ParticipantState::Running, "Pause finished");
    _pauseDonePromise.set_value();
}

void LifecycleService::Stop(std::string reason)
{
    _lifecycleManagement->Stop(reason);
    //ChangeState(ParticipantState::Stopping, reason);

    if (_stopHandler)
    {
        try
        {
            _stopHandler();
            // The handler can report an error, which overrules the default transition to ParticipantState::Stopped
            if (State() != ParticipantState::Error)
            {
                reason += " and StopHandler completed successfully";
                _lifecycleManagement->StopHandled(std::move(reason));
                //ChangeState(ParticipantState::Stopped, std::move(reason));
            }
        }
        catch (const std::exception& e)
        {
            reason += " and StopHandler threw exception: ";
            reason += e.what();
            _lifecycleManagement->StopHandled(std::move(reason));
            //ChangeState(ParticipantState::Stopped, std::move(reason));
        }
    }
    else
    {
        reason += " and no StopHandler registered";
        _lifecycleManagement->StopHandled(std::move(reason));
        //ChangeState(ParticipantState::Stopped, reason);
    }

    if (!_hasCoordinatedSimulationStop) 
    {
        Shutdown("Shutdown after LifecycleService::Stop without stop coordination.");
    }
}

void LifecycleService::Shutdown(std::string reason)
{
    _lifecycleManagement->Shutdown(reason);
    //ChangeState(ParticipantState::ShuttingDown, reason);

    if (_shutdownHandler)
    {
        try
        {
            _shutdownHandler();
            reason += " and ShutdownHandler completed";
            _lifecycleManagement->ShutdownHandled(std::move(reason));
            //ChangeState(ParticipantState::Shutdown, std::move(reason));
        }
        catch (const std::exception& e)
        {
            reason += " and ShutdownHandler threw exception: ";
            reason += e.what();
            _lifecycleManagement->ShutdownHandled(std::move(reason));
            //ChangeState(ParticipantState::Shutdown, std::move(reason));
        }
    }
    else
    {
        reason += " and no ShutdownHandler was registered";
        _lifecycleManagement->ShutdownHandled(std::move(reason));
        //ChangeState(ParticipantState::Shutdown, std::move(reason));
    }

    _finalStatePromise.set_value(State());
}

void LifecycleService::Reinitialize(std::string reason)
{
    _lifecycleManagement->Reinitialize(reason);
    //ChangeState(ParticipantState::Reinitializing, reason);

    if (_reinitializeHandler)
    {
        try
        {
            // TODO trigger reinitialize callback on another thread
            _reinitializeHandler();
            reason += " and ReinitializeHandler completed";
            _timeSyncService->ResetTime();
            _lifecycleManagement->ReinitializeHandled(std::move(reason));
            //ChangeState(ParticipantState::Initialized, std::move(reason));
        }
        catch (const std::exception& e)
        {
            reason += " and ReinitializeHandler threw exception: ";
            reason += e.what();
            _lifecycleManagement->Error(std::move(reason));
            //ChangeState(ParticipantState::Error, std::move(reason));
        }
    }
    else
    {
        reason += " and no ReinitializeHandler was registered";
        _lifecycleManagement->ReinitializeHandled(std::move(reason));
        //ChangeState(ParticipantState::Initialized, std::move(reason));
    }

    _timeSyncService->ResetTime();
}
auto LifecycleService::State() const -> ParticipantState
{
    return _status.state;
}

auto LifecycleService::Status() const -> const ParticipantStatus&
{
    return _status;
}

void LifecycleService::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const ParticipantCommand& command)
{
    // TODO FIXME VIB-551
    if (command.participant != _serviceDescriptor.GetParticipantId())
        return;
    
    if ((State() == ParticipantState::Stopped || State() == ParticipantState::Error) && _hasCoordinatedSimulationStop)
    {
        if (command.kind == ParticipantCommand::Kind::Reinitialize)
        {
            Reinitialize(std::string{"Received ParticipantCommand::"} + to_string(command.kind));
        }
        else if (command.kind == ParticipantCommand::Kind::Shutdown)
        {
            Shutdown("Received ParticipantCommand::Shutdown");
        }
    }
}

auto LifecycleService::GetTimeSyncService() const -> ITimeSyncService*
{
    return _timeSyncService;
}

void LifecycleService::ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& command)
{
    // Ignore messages if the lifecycle is not being executed yet
    if (!_isRunning)
    {
        // TODO this should be handled as a late joining scenario instead of an error...
        std::stringstream msg;
        msg << "Received SystemCommand::" << command.kind
            << " before ParticipantController::ExecuteLifecycleSyncTime(...) or ExecuteLifecycleNoSyncTime(...) was called."
            << " Origin of current command was " << from->GetServiceDescriptor();
        ReportError(msg.str());
        return;
    }

    switch (command.kind)
    {
    case SystemCommand::Kind::Invalid: break;

    case SystemCommand::Kind::Run:
        if (!_hasCoordinatedSimulationStart)
        {
            _logger->Info(
                "Received SystemCommand::Start, but ignored it because coordinatedSimulationStart was not set.");
            return;
        }
        else
        {
            _lifecycleManagement->Run("Received SystemCommand::Run");
            //ChangeState(ParticipantState::Running, "Received SystemCommand::Run");
            return;
        }
        break;

    case SystemCommand::Kind::Stop:

        if (!_hasCoordinatedSimulationStop)
        {
            _logger->Info(
                "Received SystemCommand::Stop, but ignored it because coordinatedSimulationStop was not set.");
            return;
        }
        //_logger->Warn("Ignored the received SystemCommand::Stop, because the participant state is already ParticipantState::Stopped");
        Stop("Received SystemCommand::Stop");
        return;

    case SystemCommand::Kind::Shutdown:
        Shutdown("Received SystemCommand::Shutdown");
        return;
        // TODO Remove below ASAP
    case SystemCommand::Kind::PrepareColdswap: 
    case SystemCommand::Kind::ExecuteColdswap: 
    default: break;
    }

    // We should not reach this point in normal operation.
    ReportError("Received SystemCommand::" + to_string(command.kind)
                + " while in ParticipantState::" + to_string(State()));
}

void LifecycleService::ChangeState(ParticipantState newState, std::string reason)
{
    _status.state = newState;
    _status.enterReason = std::move(reason);
    _status.enterTime = std::chrono::system_clock::now();
    _status.refreshTime = _status.enterTime;

    SendIbMessage(_status);
}

void LifecycleService::SetTimeSyncService(TimeSyncService* timeSyncService)
{
    _timeSyncService = timeSyncService;
}

} // namespace sync
} // namespace mw
} // namespace ib