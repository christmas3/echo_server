FROM gcc:latest 

RUN apt-get -y update && \
    apt-get install -y && \
    apt-get install -y cmake libev-dev libgflags-dev libgoogle-glog-dev

COPY . /usr/src/echo_server

WORKDIR /usr/src/echo_server/build

RUN cmake .. && \
    cmake --build .

EXPOSE 55001

CMD [ "./echo_server", "-port", "55001", "-logtostderr" ]