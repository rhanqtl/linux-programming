#!/bin/sh

if read n; then
    max=$n
    min=$n
    sum=$n
else
    exit -1
fi

i=1
while [ "$i" -lt 10 ]; do
    read n
    if [ "$n" -lt $min ]; then
        min=$n
    fi
    if [ "$n" -gt $max ]; then
        max=$n
    fi
    sum=$(($sum+$n))
    i=$(($i+1))
done

echo "max: ${max}, min: ${min}, sum: ${sum}"
