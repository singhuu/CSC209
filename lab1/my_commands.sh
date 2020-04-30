#!/usr/bin/env bash
echo_arg csc209 > echo_out.txt
echo_stdin.c < echo_stdin
count 210 | wc -c
ls -S | echo_stdin
