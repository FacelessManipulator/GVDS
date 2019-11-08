#!/bin/bash

prefix=/home/wcb/data #测试数据存放位置
ls $prefix/test_1a* |
while read file_name;
do
{
    echo -e -n $file_name >> all.txt
    echo -e -n '\t'>> all.txt
    # echo 'BW=' >> all.txt
    q=$(awk '/BW=/' $file_name | awk '{print $3}')
    q=${q#*=}
    q=${q%,*}
    
    if (echo $q | grep -q 'KiB');then
        q=${q%K*}
    elif (echo $q | grep -q 'MiB');then
        q=${q%MiB*}
        q=$(awk 'BEGIN{print "'$q'"*1000 }')
    elif (echo $q | grep -q 'B/s');then
        q=${q%B*}
        q=$(awk 'BEGIN{print "'$q'"/1000 }')
    fi

    
    echo -e -n $q >> all.txt
    echo -e -n '\t' >> all.txt
    # echo 'lat' >> all.txt
    q=$(awk '$1=="lat"&&$3~/min/' $file_name | awk '{print $5}')
    quint=$(awk '$1=="lat"&&$3~/min/' $file_name | awk '{print $2}')
    quint=${quint#*(}
    quint=${quint%)*}
    q=${q#*=}
    q=${q%,*}
    
    if (echo $quint | grep -q 'msec');then
        q=$(awk 'BEGIN{print "'$q'"*1000 }')
    fi

    echo -e -n $q >> all.txt
    # echo -e -n $quint >> all.txt
    echo -e -n '\t' >> all.txt
    # echo 'IOPS=' >> all.txt
    q=$(awk '/IOPS=/' $file_name | awk '{print $2}')
    q=${q#*=}
    q=${q%,*}
    echo $q >> all.txt
}
done

ls $prefix/test_1b* |
while read file_name;
do
{
    echo -e -n $file_name >> all.txt
    echo -e -n '\t'>> all.txt
    # echo 'BW=' >> all.txt
    q=$(awk '/BW=/' $file_name | awk '{print $3}')
    q=${q#*=}
    q=${q%,*}

    if (echo $q | grep -q 'KiB');then
        q=${q%K*}
    elif (echo $q | grep -q 'MiB');then
        q=${q%MiB*}
        q=$(awk 'BEGIN{print "'$q'"*1000 }')
    elif (echo $q | grep -q 'B/s');then
        q=${q%B*}
        q=$(awk 'BEGIN{print "'$q'"/1000 }')
    fi

    echo -e -n $q >> all.txt
    echo -e -n '\t' >> all.txt
    # echo 'lat' >> all.txt
    q=$(awk '$1=="lat"&&$3~/min/' $file_name | awk '{print $5}')
    quint=$(awk '$1=="lat"&&$3~/min/' $file_name | awk '{print $2}')
    quint=${quint#*(}
    quint=${quint%)*}
    q=${q#*=}
    q=${q%,*}
    
    if (echo $quint | grep -q 'msec');then
        q=$(awk 'BEGIN{print "'$q'"*1000 }')
    fi

    echo -e -n $q >> all.txt
    # echo -e -n $quint >> all.txt
    echo -e -n '\t' >> all.txt
    # echo 'IOPS=' >> all.txt
    q=$(awk '/IOPS=/' $file_name | awk '{print $2}')
    q=${q#*=}
    q=${q%,*}
    echo $q >> all.txt
}
done

ls $prefix/test_1fw* |
while read file_name;
do
{
#write
    echo -e -n $file_name >> all.txt
    echo -e -n '\t'>> all.txt
    # echo 'bw=' >> all.txt
    q=$(awk '$1~/WRITE:/' $file_name | awk '{print $2}')
    q=${q#*=}
    q=${q%,*}

    if (echo $q | grep -q 'KiB');then
        q=${q%K*}
    elif (echo $q | grep -q 'MiB');then
        q=${q%MiB*}
        q=$(awk 'BEGIN{print "'$q'"*1000 }')
    elif (echo $q | grep -q 'B/s');then
        q=${q%B*}
        q=$(awk 'BEGIN{print "'$q'"/1000 }')
    fi

    echo -e -n $q >> all.txt
    echo -e -n '\t' >> all.txt

    q=$(awk '$1~/WRITE:/' $file_name | awk '{print $8}')
    q=${q#*=}
    q=${q%-*}
    echo -e -n $q >> all.txt
    echo -e -n '\t' >> all.txt
    q=$(awk '$1~/WRITE:/' $file_name | awk '{print $8}')
    q=${q#*-}
    q=${q%m*}
    q=${q%u*}
    echo  $q >> all.txt
    # echo -e -n '\t' >> all.txt
}
done

ls $prefix/test_1fr* |
while read file_name;
do
{
#read
    echo -e -n $file_name >> all.txt
    echo -e -n '\t'>> all.txt
    # echo 'bw=' >> all.txt
    q=$(awk '$1~/READ:/' $file_name | awk '{print $2}')
    q=${q#*=}
    q=${q%,*}

    if (echo $q | grep -q 'KiB');then
        q=${q%K*}
    elif (echo $q | grep -q 'MiB');then
        q=${q%MiB*}
        q=$(awk 'BEGIN{print "'$q'"*1000 }')
    elif (echo $q | grep -q 'B/s');then
        q=${q%B*}
        q=$(awk 'BEGIN{print "'$q'"/1000 }')
    fi

    echo -e -n $q >> all.txt
    echo -e -n '\t' >> all.txt

    q=$(awk '$1~/READ:/' $file_name | awk '{print $8}')
    q=${q#*=}
    q=${q%-*}
    echo -e -n $q >> all.txt
    echo -e -n '\t' >> all.txt
    q=$(awk '$1~/READ:/' $file_name | awk '{print $8}')
    q=${q#*-}
    q=${q%m*}
    q=${q%u*}
    echo  $q >> all.txt
    # echo -e -n '\t' >> all.txt
}
done