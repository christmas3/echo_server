#!/usr/bin/env bash

print_help()
{
    echo "Usage: multi_send_cli.sh ip port cli_count send_size send_count delay"
}

check_send_cli()
{
    if [ ! -e ./single_send_cli.sh ]; then
        echo "./single_send_cli.sh: command not found"
        exit 2
    fi
}

if [ $# -lt 6 ]; then
    print_help
    exit 1
fi

serv_ip="$1"
serv_port="$2"
cli_count="$3"
send_size="$4"
send_count="$5"
delay="$6"

check_send_cli

while [ $cli_count -gt 0 ]; do
    bash -c "./single_send_cli.sh $serv_ip $serv_port $send_size $send_count $delay > /dev/null" &
    cli_count=$((cli_count - 1))
    # if [ -n "$delay" ]; then
    #     sleep $delay
    # fi
done