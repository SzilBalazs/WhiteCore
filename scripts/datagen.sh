#!/bin/bash

git clone https://github.com/SzilBalazs/WhiteCore
make EXE=Binary -C WhiteCore
cp WhiteCore/Binary Binary
rm -rf WhiteCore

while true
do
  echo $1 | ./Binary
  sleep 1
done