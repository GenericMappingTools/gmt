#!/usr/bin/env bash

# try different
# test -T with -S 
testts(){
  gmt earthtide -T2019-01-02T -S 
  gmt earthtide -T2019-01-01T/2019-01-02T/1h  -S | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/5m  -S | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/3s  -S | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/2h  -S | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/24h -S | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/1d  -S | tail -n 1
  gmt earthtide -T2018-01-01T/2019-01-02T/6d  -S | tail -n 1
} 

testts | tee ./test_earthtide_interval.dat
# check if all lines are same
[[ $(testts | uniq | wc -l ) == 1 ]] && echo ok || { echo "test failed" ;  exit 1 ; }

echo

# test -T with -L 
testtl(){
  gmt earthtide -T2019-01-02T -L0/0 
  gmt earthtide -T2019-01-01T/2019-01-02T/1h -L0/0 | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/5m -L0/0 | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/3s -L0/0 | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/2h -L0/0 | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/24h -L0/0  | tail -n 1
  gmt earthtide -T2019-01-01T/2019-01-02T/1d -L0/0 | tail -n 1
}

# see output
testtl | tee -a ./test_earthtide_interval.dat
# check if all lines are same
[[ $(testtl | uniq | wc -l ) == 1 ]] && echo ok || { echo "test failed" ;  exit 1 ; }

