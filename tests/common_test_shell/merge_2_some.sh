#!/bin/bash

prefix=/home/wcb/data/ #测试数据存放位置

# testsize=(testsize1a testsize1b testsize1c testsize1d testsize1e testsize1f testsize1g testsize1h testsize1fw testsize1fr)
testsize1c=(4k 256k 4M)
testsize1d=(4k 8k 16k 32k 64k 128k 256k 512k 1024k 4M)
testsize1e=(1 2 4 8 16 32 64 128 256 512)
testsize1f=(1 2 5 10 30 50 100)
testsize1g=(1 10 100 1000)
testsize1h=(create_1 create_2 create_5 create_10 create_30 create_50 create_100 ls_1 ls_2 ls_5 ls_10 ls_30 ls_50 ls_100 rm_1 rm_2 rm_5 rm_10 rm_30 rm_50 rm_100)

testsize1a=(8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M)
testsize1b=(8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M)
testsize1fw=(1 10 20 50 100)
testsize1fr=(1 10 20 50 100)

testname=(1a 1b 1fw 1fr)
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
            echo -e -n $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $2}') >> ${testname[$i]}.txt
            echo -e -n '\t' >> ${testname[$i]}.txt
            echo -e -n $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $3}') >> ${testname[$i]}.txt
            echo -e -n '\t' >> ${testname[$i]}.txt
            echo $(awk '$1=="'$prefix''$filename'"' $finalfile | awk '{print $4}') >> ${testname[$i]}.txt
        }
        done
    }
    done
}
done
