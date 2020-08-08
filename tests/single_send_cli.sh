#!/usr/bin/env bash

print_help()
{
    echo "Usage: single_send_cli.sh ip port send_size send_count delay"
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
    local send_count_="$2"
    local delay_="$3"
    cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $send_size_ | head -n $send_count_ | while read -r line ; do echo "$line"; sleep $delay_; done
}

if [ $# -lt 5 ]; then
    print_help
    exit 1
fi

serv_ip="$1"
serv_port="$2"
send_size="$3"
send_count="$4"
delay="$5"

check_nc

send_text $send_size $send_count $delay | nc -N $serv_ip $serv_port


