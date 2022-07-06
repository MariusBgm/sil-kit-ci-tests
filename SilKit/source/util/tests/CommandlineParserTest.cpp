// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <functional>
#include <numeric>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CommandlineParser.hpp"

namespace SilKit {
namespace Util {
namespace tests {

using ::testing::Return;
using ::testing::A;
using ::testing::An;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Throw;

auto BuildArguments(const std::vector<const char*>& arguments) -> std::vector<char*>
{
    std::vector<char*> argv;
    for (const auto& arg : arguments)
    {
        argv.push_back(const_cast<char*>(arg));
    }
    argv.push_back(nullptr);

    return argv;
}

TEST(SilKitCommandlineParserTest, test_mixed_arguments)
{
    auto arguments = { "main", "-d=1234", "XYZ", "--name=Xxxx", "--version" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "NetworkSimulator", "[--name=<participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'NetworkSimulator'");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("domain", "d", "42", "[--domain=<domainId>]",
        "-d, --domain <domainId>: The domain ID which is used by the SilKit. Defaults to 42");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "<configuration>",
        "<configuration>: Path and filename of the network simulator configuration YAML or JSON file. Note that the format was changed in v3.6.11");

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").Value(), "42");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "NetworkSimulator");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").Value(), "1234");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "XYZ");
}

TEST(SilKitCommandlineParserTest, test_spaced_option_values)
{
    auto arguments = { "main", "-d", "1234", "XYZ", "--name", "Xxxx", "--version" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("domain", "d", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").Value(), "1234");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "XYZ");
}

TEST(SilKitCommandlineParserTest, test_bad_argument)
{
    auto arguments = { "main", "-domain=1234", "XYZ", "--name=Xxxx", "--version" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("domain", "d", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    EXPECT_THROW(commandlineParser.ParseArguments(argc, argv), std::runtime_error);
}

TEST(SilKitCommandlineParserTest, test_no_arguments)
{
    auto arguments = { "main" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "NetworkSimulator", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("domain", "d", "42", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("domain").Value(), "42");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "NetworkSimulator");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "");
}

TEST(SilKitCommandlineParserTest, test_missing_option_value)
{
    auto arguments = { "main", "--domain" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("domain", "d", "", "", "");

    EXPECT_THROW(commandlineParser.ParseArguments(argc, argv), std::runtime_error);
}


} // namespace tests
} // namespace Util
} // namespace SilKit