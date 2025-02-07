#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

void put_sub() {
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    auto subscriber = session2.declare_subscriber("zenoh/test", channels::RingChannel(1));

    std::this_thread::sleep_for(1s);

    Session::PutOptions options;
    options.attachment = std::vector<int32_t>{-1, 10, 15000};
    session1.put("zenoh/test", "data", std::move(options));

    std::this_thread::sleep_for(1s);

    auto res = subscriber.handler().recv();
    assert(std::holds_alternative<Sample>(res));
    const Sample& msg = std::get<Sample>(res);
    assert(msg.get_keyexpr() == "zenoh/test");
    assert(msg.get_payload().deserialize<std::string>() == "data");
    assert(msg.get_attachment().has_value());
    assert((msg.get_attachment()->get().deserialize<std::vector<int32_t>>() == std::vector<int32_t>{-1, 10, 15000}));
}

int main(int argc, char** argv) { put_sub(); }
