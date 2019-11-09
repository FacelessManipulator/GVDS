#!/bin/bash

prefix=/home/wcb/data  #测试数据存放位置
nums=(0ms 10ms 20ms 40ms 60ms 80ms 100ms) #测试时延
op_size=300 #元数操作数

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

     #1c：随机读
    testname=1c
    for ((i=0;i<$bs_length;i++));
    do
    {
        fio -filename=$testname -fallocate=none  -direct=$direct -iodepth 1 -thread -rw=randread -ioengine=libaio -bs=${block_size[$i]} -size=$size   -runtime=$runtime -numjobs=1 -name=$testname  > $prefix/test_$testname\_${nums[$k]}_${block_size[$i]}
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

    #1gw：多文件多线程顺序写
    testname=1gw
    for ((i=0;i<$jlist_size;i++));
    do
    {
        fio -filename_format='test.$jobnum' -fallocate=none -direct=$direct -iodepth 1 -thread -rw=write -ioengine=libaio -bs=128k -size=$size   -runtime=$runtime -numjobs=${job_list[i]} -name=$testname   > $prefix/test_$testname\_${nums[$k]}_${job_list[$i]}
    }
    done
    echo test_$testname\_${nums[$k]} done

    #1gr：多文件多线程顺序读
    testname=1gr
    for ((i=0;i<$jlist_size;i++));
    do
    {
        fio -filename_format='test.$jobnum' -fallocate=none -direct=$direct -iodepth 1 -thread -rw=read -ioengine=libaio -bs=128k -size=$size   -runtime=$runtime -numjobs=${job_list[i]} -name=$testname   > $prefix/test_$testname\_${nums[$k]}_${job_list[$i]}
    }
    done
    echo test_$testname\_${nums[$k]} done

    #1h：元数据操作
    threads=(1 10 20 50 100) #线程数目
    
    threads_size=${#threads[*]}
    
    mkdir meta >& /dev/null

    for ((g=0; g<=$threads_size; g++)); 
    do
    {
        num=${threads[$g]}

        # create
        testname=1hc
        (time {
            for ((i=1; i<=$num; i++)); do
            {
                mkdir meta/test$i
                
                for ((j=1; j<=$[$op_size/$num]; j++));
                do
                {
                    touch meta/test$i/test$i$j
                }
                done

            }&   
            done
            wait
        }) >& $prefix/test_$testname\_${nums[$k]}_$num

        # stat
        testname=1hs
        (time {
            for ((i=1; i<=$num; i++)); do
            {
                for ((j=1; j<=$[$op_size/$num]; j++));
                do
                {
                    stat meta/test$i/test$i$j > /dev/null
                }
                done

            }&   
            done
            wait
        }) >& $prefix/test_$testname\_${nums[$k]}_$num

        # rm
        testname=1hr
        (time {
            for ((i=1; i<=$num; i++));
            do
            {
                rm meta/test$i -r
            }&
            done
            wait
        }) >& $prefix/test_$testname\_${nums[$k]}_$num
        rm meta/* -r >& /dev/null

    }
    done
    
    echo test_1h_${nums[$k]} done

    tc qdisc del dev eno1 root netem delay ${nums[$k]}
}
done