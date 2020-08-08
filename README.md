# echo_server
simple single-thread tcp echo server

# Dependencies
* libev
* libgflags
* libgoogle-glog

# Compiling
``` shell
mkdir build
cd build
cmake ..
cmake --build .
```

## Running
For all running options you need:

```shell 
./echo_server --help
```


Only for necessary running options:

```shell 
./echo_server --helpshort
```


For writing debug into stderr use -logtostderr flag, otherwise log will be writing in /tmp

# Testing
```shell
cd tests
./multi_send_cli.sh <ip> <port> <cli_count> <send_size> <send_count> <delay>
```


run <cli_count> numbers of nc commands in background which connect to the server by <ip> and <port>
and send random alphabet sequence <send_size> length <send_count> times with <delay> between every send

# Docker
```shell
docker build -t echo_server .
docker run -p 55001:55001 --name echo_server echo_server
```
