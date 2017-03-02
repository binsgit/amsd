AMSD - Avalon Management System Daemon
==============================
A versatile and extensible daemon to provide functionalities for AMS.

Features
------------
- Simple design: Unix domain socket for I/O & JSON transport
- Super fast and computational-efficient, low resource cost
- Plugin interface, new features can be added easily

Implemented Functions
------------
- Super RTAC: Execute scripts on remote machines in parallel & monitor realtime status
- Query FW version
- Data collection in parallel
- Mail reporting

Build
------------
Install packages:

    apt-get install build-essential cmake libjansson-dev libssh2-1-dev libsqlite3-dev libevent-dev libcurl4-openssl-dev libb64-dev 

Build:

    mkdir build
    cd build
    cmake ..
    make -j `nproc`
