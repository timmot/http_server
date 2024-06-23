#include "utility/Clock.hpp"
#include "utility/Convert.hpp"
#include "utility/Ipv4Address.hpp"
#include "utility/log.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <netdb.h>
#include <string_view>
#include <unistd.h>

void test_ip()
{
    auto ip = Ipv4Address::from_string("192.0.2.33");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("255.254.253.252");
    if (ip.has_value()) {
        sockaddr_in sa;
        sa.sin_addr.s_addr = ip->to_in_addr_t();

        assert(ip == Ipv4Address(255, 254, 253, 252));
        assert(ip != Ipv4Address(1, 2, 3, 4));
        static_assert(Ipv4Address(255, 254, 253, 252) == Ipv4Address(255, 254, 253, 252));
        static_assert(Ipv4Address(1, 2, 3, 4) != Ipv4Address(1, 2, 3, 5));

        logln(ip->to_string());
    }

    ip = Ipv4Address::from_string("0.0.0.0");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("localhost");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("test");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("a.b.c.d");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("256.254.253.252");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("254.300.253.252");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("254.100.353.152");
    if (ip.has_value())
        logln(ip->to_string());

    ip = Ipv4Address::from_string("254.100.253.352");
    if (ip.has_value())
        logln(ip->to_string());

    Clock clock = {};
    // Remote resolution can take >500ms on first load, or ~10ms on cached load
    auto maybe_value
        = Ipv4Address::resolve_hostname("google.com");
    assert(maybe_value.has_value());
    if (maybe_value.has_value())
        logln("ip: {}, took {}ms", maybe_value->to_string(), clock.ms_and_reset());

    maybe_value = Ipv4Address::resolve_hostname("localhost");
    assert(maybe_value.value() == Ipv4Address(127, 0, 0, 1));
    if (maybe_value.has_value())
        logln("ip: {}, took {}ms", maybe_value->to_string(), clock.ms_and_reset());

    maybe_value = Ipv4Address::resolve_hostname("nsdflk.com");
    assert(!maybe_value.has_value());
    if (maybe_value.has_value())
        logln("ip: {}, took {}ms", maybe_value->to_string(), clock.ms_and_reset());

    maybe_value = Ipv4Address::resolve_hostname("0.0.0.0");
    if (maybe_value.has_value())
        logln("ip: {}", maybe_value->to_string());

    maybe_value = Ipv4Address::resolve_hostname("192.168.2.1");
    if (maybe_value.has_value())
        logln("ip: {}", maybe_value->to_string());
}

void test_conversion(std::string const& s)
{
    auto x = Convert::to<int>(s);
    if (!x.has_value())
        logln("int fail: \"{}\"", s);
    else
        logln("int success: {}", *x);

    auto y = Convert::to<double>(s);
    if (!y.has_value())
        logln("double fail: \"{}\"", s);
    else
        logln("double success: {}", *y);
}

int main(int argc, char* argv[])
{
    logln("testing Ipv4Address");
    test_ip();

    logln("testing conversions");
    test_conversion("123");
    test_conversion("123.456");
    test_conversion("9999999999999999");
    test_conversion("Banana");
    test_conversion("INF");
    test_conversion("NaN");
    auto x = Convert::to<Convert>("fake conversion");
    assert(!x.has_value());

    return 0;
}
