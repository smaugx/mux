# mux
Modern C++ network programming library for Linux.



# build

first, you should get the source code:

```
git clone https://github.com/smaugx/mux.git
cd mux
```

we support to build using two tools: **scons** 、**cmake**

## use scons

you just need runing:

```
scons
```

then check dir `sbuild`, all targets will be this dir.

if you can't find this command in your machine, mayby you should install this tool first:

```
pip install scons
```

Is it super easy?

if you think using scons to  build is slow, try this:

```
scons -j4
```

> similar to `make -j4`.


if you want to clean the build-objects, try this:

```
scons -c
```

Good Luck!


## use cmake

usually, run this:

```
mkdir cbuild
cd cbuild
cmake ..
make -j4
```

then check `cbuild/bin` or `cbuild/lib` dir.

# How To use

It's easy to write a server, follow me:

+ create a TcpAcceptor which will create a listen handle and manager accept event(new socket)

```
auto tcp_acceptor = new transport::TcpAcceptor(local_ip, local_port);
```
+ create a MessageHandler which will handle recv packages

```
auto msg_handle = std::make_shared<transport::MessageHandler>();
```

+ register recv callback to MessageHandler

```
    auto recv_call = [&](const transport::PacketPtr& packet) -> void {
        // write your own recv-callback here
        /*
        auto key = packet->from_ip_addr + ":" + std::to_string(packet->from_ip_port);
        auto sock = echo_tcp_acceptor->FindSession(key);
        if (!sock) {
            MUX_WARN("not found session:{0}", key);
            return;
        }
        sock->SendData(packet);
        */
        return;
    };

    msg_handle->Init();
    msg_handle->RegisterOnDispatchCallback(recv_call);
    auto dispath_call = [&](transport::PacketPtr& packet) -> void {
        return msg_handle->HandleMessage(packet);
    };


    tcp_acceptor->RegisterNewSocketRecvCallback(dispath_call);
```

+ create EventTrigger which will handle event from kernel

```
int ep_num = 1;
auto event_trigger = std::make_shared<transport::EventTrigger>(ep_num);
```

+ Start

```
    auto accept_callback = [&](int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) -> transport::BasicSocket* {
        return tcp_acceptor->OnSocketAccept(cli_fd, remote_ip, remote_port);
    };
    
    event_trigger->Start();
    tcp_acceptor->Start();

    // attention: RegisterDescriptor must after Start
    event_trigger->RegisterOnAcceptCallback(accept_callback);
    event_trigger->RegisterDescriptor((void*)tcp_acceptor);
```


OK, that's all for a tcp server, is it easy?

you can write a tcp client, similar to tcp server. 

And you may found some examples in dir demo.

# Demo
you can found echo demo and bench demo in dir:

```
demo/echo
demo/bench
```

after build, you will get binary-exe:

```
bench_client  bench_server  echo_client  echo_server
```

open a terminal and start echo\_server:

```
./echo_server
# 或者指定ip port
./echo_server 127.0.0.1 6666
```

open a new terminal and start echo\_client:

```
./echo_client
# and then input something
```


bench test for release version:

```
90000 tps in 4 cpu 8G mem.
```


