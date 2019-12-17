#!/bin/bash
ifconfig eth0 down
ifconfig eth0 up 
ifconfig eth0 172.16.21.1/24
route add 172.16.20.1 gw 172.16.21.253
cp resolv.conf /etc/resolv.conf

