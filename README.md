# udp2tcp
Quick and dirty tool to tunnel UDP traffic over an arbitrary TCP port on Windows

### Downloads

x86, x64, and ARM64 binaries for Windows 7+ are available [on the releases page](https://github.com/cgutman/udp2tcp/releases).

### Usage

`u2t-client.exe <local address> <local UDP port> <remote address> <remote TCP port>`
- The local address will probably be 127.0.0.1, but you can also send traffic off-host by specifying another IP address
- Tunneled traffic received from the server will be sent to `<local address>:<local UDP port>` from an ephemeral port
- UDP traffic sent back to the ephemerally-bound UDP socket will be tunneled to the server

`u2t-server.exe <local TCP port> <local UDP port> <remote address> <remote UDP port>`
- The server needs to be running before starting the client
- Tunneled traffic received from the client will be sent to `<remote address>:<remote UDP port>` from `<local UDP port>`
- UDP traffic sent to `<local UDP port>` will be tunneled to the the client