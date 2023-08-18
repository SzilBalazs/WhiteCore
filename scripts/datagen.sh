#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "Example usage: ./datagen.sh \"gen nodes 1000 threads 4 games 1000\""
    exit 1
fi

git clone https://github.com/SzilBalazs/WhiteCore
make EXE=Binary -C WhiteCore
cp WhiteCore/Binary Binary
./Binary test || exit 1
./Binary bench
rm -rf WhiteCore

while true
do
  echo $1 | ./Binary
  sleep 1
done