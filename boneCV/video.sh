#!/bin/bash
complete=1
while [ $complete == 1 ] 
do
./capture -F -c 200 -o > output.raw
chmod 777 output.raw
./raw2mpg4
return=$?
  if [ $return == 0 ]; then
     complete=0
  fi
  if [ $return == 1 ]; then
     sleep 1
  fi
rm output.raw
done

