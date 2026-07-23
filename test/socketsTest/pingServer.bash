#!/bin/bash

#Script to check connection

while true; do 
	ping6 $1
	sleep 1
done
