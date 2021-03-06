// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#define BOOST_TEST_IGNORE_SIGCHLD
#include <boost/test/included/unit_test.hpp>

#include <boost/system/error_code.hpp>

#include <iostream>

#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <boost/process/error.hpp>
#include <boost/process/io.hpp>
#include <boost/process/args.hpp>
#include <boost/process/system.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/process/async.hpp>
#include <system_error>

#include <boost/filesystem.hpp>

#include <string>
#include <chrono>
#include <istream>
#include <cstdlib>
#if defined(BOOST_WINDOWS_API)
#   include <Windows.h>
typedef boost::asio::windows::stream_handle pipe_end;
#elif defined(BOOST_POSIX_API)
#   include <sys/wait.h>
#   include <unistd.h>
typedef boost::asio::posix::stream_descriptor pipe_end;
#endif

namespace fs = boost::filesystem;
namespace bp = boost::process;

BOOST_AUTO_TEST_CASE(explicit_async_io, *boost::unit_test::timeout(2))
{
    using boost::unit_test::framework::master_test_suite;

    boost::asio::io_service ios;

    std::future<std::string> fut;

    std::error_code ec;
    bp::system(
        master_test_suite().argv[1],
        "test", "--echo-stdout", "abc",
        bp::std_out > fut,
        ios,
        ec
    );
    BOOST_REQUIRE(!ec);

    BOOST_REQUIRE(fut.valid());
    BOOST_REQUIRE(boost::starts_with(fut.get(), "abc"));
}

BOOST_AUTO_TEST_CASE(explicit_async_io_running, *boost::unit_test::timeout(10))
{
    using boost::unit_test::framework::master_test_suite;

    boost::asio::io_service ios;

    boost::asio::deadline_timer timer{ios, boost::posix_time::seconds(5)};

    bool called = false;
    timer.async_wait(
            [&](const boost::system::error_code &ec)
            {
                called=true;
            });

    std::thread thr([&]{ios.run();});

    std::future<std::string> fut;
    std::error_code ec;

    bp::system(
        master_test_suite().argv[1],
        "test", "--echo-stdout", "abc",
        bp::std_out > fut,
        ios,
        ec
    );
    BOOST_REQUIRE(!ec);

    BOOST_REQUIRE(fut.valid());
    BOOST_REQUIRE(boost::starts_with(
            fut.get(), "abc"));

    BOOST_CHECK(!called);

    thr.join();

    BOOST_CHECK(called);
}
