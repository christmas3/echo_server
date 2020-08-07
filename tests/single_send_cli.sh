#!/usr/bin/env bash

print_help()
{
    echo "Usage: single_send_cli.sh ip port client_count send_size [delay]"
}

check_nc()
{
    if ! which nc > /dev/null; then
        echo "nc: command not found"
        exit 2
    fi
}

send_text()
{
    local send_size_="$1"
    cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $send_size_ | head -n 1
}

serv_ip="$1"
serv_port="$2"
client_count="$3"
send_size="$4"
delay="$5"

if [ $# -lt 4 ]; then
    print_help
    exit 1
fi

check_nc

while [ $client_count -gt 0 ]; do
    for_send="$(send_text $send_size)"
    echo -e "\n$client_count : $for_send"
    echo "$for_send" | nc -W 1 $serv_ip $serv_port
    client_count=$((client_count - 1))
    # if [ -n "$delay" ]; then
    #     sleep $delay
    # fi
done
