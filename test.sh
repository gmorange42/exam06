#!/bin/bash
for i in {1..1500}
do
    nc localhost 6666 < /dev/null &
    nc_pid=$!
    sleep 0.01
    kill $nc_pid
done

