#!/bin/sh
mypwd="$PWD"

echo $mypwd

if true; then
for dir_name in `find . -maxdepth 1 -type d`
  do
    echo "enter $dir_name"
    cd $dir_name
    for filename in `find . -maxdepth 1 -type f`
      do
        echo Deal with $filename ...
        sed s/cpm/$1/g $filename > $filename'.tmp'
        rm -f $filename
        mv $filename'.tmp' $(echo $filename | sed s/cpm/$1/g)
        echo "                        [done!]"
      done
    cd $mypwd
  done
fi


