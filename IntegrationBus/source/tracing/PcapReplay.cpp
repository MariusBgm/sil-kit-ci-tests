#include "PcapReplay.hpp"

#include <memory>

#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/mw/EndpointAddress.hpp"

#include "PcapReader.hpp"


namespace {
using namespace ib::extensions;
using namespace ib::mw::logging;
using namespace ib::tracing;
//////////////////////////////////////////////////////////////////////
// IReplay: Boilerplate to satisfy interfaces follows.
//          Actual implementation is in PcapReader.
//////////////////////////////////////////////////////////////////////

class ReplayPcapChannel
    : public IReplayChannel
{
public:
    ReplayPcapChannel(const std::string& filePath, ILogger* logger)
        : _reader{filePath, logger}
    {
    }

    // Interface IReplayChannel
    auto Type() const -> ib::extensions::TraceMessageType override
    {
        //our version supports only ethernet
        return TraceMessageType::EthFrame;
    }

    auto StartTime() const -> std::chrono::nanoseconds override
    {
        return _reader.StartTime();
    }
    auto EndTime() const -> std::chrono::nanoseconds override
    {
        return _reader.EndTime();
    }
    auto NumberOfMessages() const -> uint64_t override
    {
        return _reader.NumberOfMessages();
    }
    auto Name() const -> const std::string&
    {
        return "PcapChannel0";
    }
    auto GetMetaInfos() const -> const std::map<std::string, std::string>&
    {
        return _reader.GetMetaInfos();
    }
    auto GetReader() -> std::shared_ptr<IReplayChannelReader>
    {
        // return a copy, which allows caching the internal data structures
        // for seeking. It is reset to start reading at the beginning.
        return std::make_shared<PcapReader>(_reader);
    }
private:
    PcapReader _reader;
};

class ReplayPcapFile :
    public IReplayFile
{
public:
    ReplayPcapFile(std::string filePath, ib::mw::logging::ILogger* logger)
        : _filePath{std::move(filePath)}
    {
        auto channel = std::make_shared<ReplayPcapChannel>(_filePath, logger);
        _channels.emplace_back(std::move(channel));
    }
    auto FilePath() const -> const std::string & override
    {
        return _filePath;
    }
    auto IntegrationBusConfig() const -> std::string override
    {
        return {};
    }

    FileType Type() const override
    {
        return IReplayFile::FileType::PcapFile;
    }

    std::vector<std::shared_ptr<IReplayChannel>>::iterator begin() override
    {
        return _channels.begin();
    }
    std::vector<std::shared_ptr<IReplayChannel>>::iterator end() override
    {
        return _channels.end();
    }

private:
    std::string _filePath;
    std::vector<std::shared_ptr<IReplayChannel>> _channels;
};

} //end anonymous namespace


namespace ib {
namespace tracing {

auto PcapReplay::OpenFile(std::string filePath, ib::mw::logging::ILogger* ibLogger)
    -> std::shared_ptr<extensions::IReplayFile>
{
    return std::make_shared<ReplayPcapFile>(std::move(filePath), ibLogger);
}

} // namespace tracing
} // namespace ib
