#!/bin/bash

prefix=/home/wcb/data  #测试数据存放位置
nums=(0ms 10ms 20ms 40ms 60ms 80ms 100ms) #测试时延
runtime=10s 
size=1G
direct=1
nums_size=${#nums[*]}
for ((k=0; k<$nums_size; k++));
do
{
    tc qdisc add dev eno1 root netem delay ${nums[$k]}

    block_size=(8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M) #单次操作的块大小
    bs_length=${#block_size[*]}
    #1a:顺序写
    testname=1a #测试名称
    for ((i=0;i<$bs_length;i++));
    do
    {
        fio -filename=$testname -fallocate=none  -direct=$direct -iodepth 1 -thread -rw=write -ioengine=libaio -bs=${block_size[$i]} -size=$size   -runtime=$runtime -numjobs=1 -name=$testname  > $prefix/test_$testname\_${nums[$k]}_${block_size[$i]}
    }
    done
    echo test_$testname\_${nums[$k]} done

    #1b：顺序读
    testname=1b
    for ((i=0;i<$bs_length;i++));
    do
    {
        fio -filename=$testname -fallocate=none  -direct=$direct -iodepth 1 -thread -rw=read -ioengine=libaio -bs=${block_size[$i]} -size=$size   -runtime=$runtime -numjobs=1 -name=$testname  > $prefix/test_$testname\_${nums[$k]}_${block_size[$i]}
    }
    done
    echo test_$testname\_${nums[$k]} done

    job_list=(1 10 20 50 100) #线程数目
    jlist_size=${#job_list[*]}
    #1fw：单文件多线程顺序写
    testname=1fw
    for ((i=0;i<$jlist_size;i++));
    do
    {
        fio -filename=$testname -fallocate=none  -direct=$direct -iodepth 1 -thread -rw=write -ioengine=libaio -bs=128k -size=$size   -runtime=$runtime -numjobs=${job_list[i]} -name=$testname  > $prefix/test_$testname\_${nums[$k]}_${job_list[$i]}
    }
    done
    echo test_$testname\_${nums[$k]} done

    #1fr：单文件多线程顺序读
    testname=1fr
    for ((i=0;i<$jlist_size;i++));
    do
    {
        fio -filename=$testname -fallocate=none  -direct=$direct -iodepth 1 -thread -rw=read -ioengine=libaio -bs=128k -size=$size   -runtime=$runtime -numjobs=${job_list[i]} -name=$testname  > $prefix/test_$testname\_${nums[$k]}_${job_list[$i]}
    }
    done
    echo test_$testname\_${nums[$k]} done

    tc qdisc del dev eno1 root netem delay ${nums[$k]}
}
done