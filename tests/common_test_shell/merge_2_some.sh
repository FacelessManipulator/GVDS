#!/bin/bash

prefix=/home/wcb/data/ #测试数据存放位置

testsize1a=(8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M)
testsize1b=(8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M)
testsize1c=(8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M)
testsize1fw=(1 10 20 50 100)
testsize1fr=(1 10 20 50 100)
testsize1gw=(1 10 20 50 100)
testsize1gr=(1 10 20 50 100)
testsize1hc=(1 10 20 50 100)
testsize1hs=(1 10 20 50 100)
testsize1hr=(1 10 20 50 100)

testname=(1a 1b 1c 1fw 1fr 1gw 1gr 1hc 1hs 1hr)
nums=(0ms 10ms 20ms 40ms 60ms 80ms 100ms)
finalfile=all.txt
nums_size=${#nums[*]}
testname_size=${#testname[*]}
for ((i=0; i<$testname_size; i++));
do
{
    array_name=testsize${testname[$i]}
    bslist=(`eval echo \\${$array_name[*]}`)
    bslistsize=${#bslist[*]}
    
    for ((k=0; k<$bslistsize; k++));
    do
    {
        for ((j=0; j<$nums_size; j++));
        do
        {
            filename=test_${testname[$i]}_${nums[$j]}_${bslist[$k]}
            if (echo $filename | grep -q '1h');then
                echo $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $2}') >> ${testname[$i]}.txt
            else
                echo -e -n $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $2}') >> ${testname[$i]}.txt
                echo -e -n '\t' >> ${testname[$i]}.txt
                echo -e -n $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $3}') >> ${testname[$i]}.txt
                echo -e -n '\t' >> ${testname[$i]}.txt
                echo $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $4}') >> ${testname[$i]}.txt
            if

            
        }
        done
    }
    done
}
done
