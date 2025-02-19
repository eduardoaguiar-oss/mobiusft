#!/bin/bash

for i in *.py
do
  echo -e "\033[1;34m>> $i\033[0;39m"
  grep -- "$1" "$i"
done

for i in `find src/python -name '*.py'`
do
  echo -e "\033[1;34m>> $i\033[0;39m"
  grep -- "$1" "$i"
done

for i in `find src/extensions -name '*.py'`
do
  echo -e "\033[1;34m>> $i\033[0;39m"
  grep -- "$1" "$i"
done

for i in `find src -name '*.h' -o -name '*.cc'`
do
  echo -e "\033[1;34m>> ${i}\033[0;39m"
  grep -- "$1" "$i"
done

for i in `find . -name '*.am'`
do
  echo -e "\033[1;34m>> ${i:2}\033[0;39m"
  grep -- "$1" "$i"
done

for i in `find $HOME/src/yfox -name '*.py'`
do
  echo -e "\033[1;34m>> $i\033[0;39m"
  grep --  "$1" "$i"
done

