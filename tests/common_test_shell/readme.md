fio版本为:fio-3.1 (版本不同可能造成数据处理问题)

testall_some.sh 为测试shell，里面写的是fio指令
merge_some.sh 为测试结果处理shell 
merge_2_some.sh 为测试结果处理shell

1a(写) 1b(读) 1c(随机读)
1fw(单文件多线程顺序写) 1fr(单文件多线程顺序读) 
1gw(多线程多文件顺序写) 1gr(多线程多文件顺序读) 
1hc(元数据create) 1hs(元数据stat) 1hr(元数据remove))

测试之前请将三个文件的prefix变量，改为你需要放入原始的测试结果的目录
使用请将testall_some.sh 放入需要进行测试的目录下,使用命令为：
$ bash testall_some.sh
之后请随便找个目录，运行merge_some.sh和merge_2_some.sh，使用命令为:
$ bash merge_some.sh
$ bash merge_2_some.sh
最终生成1a 1b 1c 1fw 1fr 1gw 1gr 1hc 1hs 1hr的最终测试结果文件，请将文件内容粘到 resultformat.xlsx中

1a 1b 1c 每一列的数据依次为 带宽(KB/s)	平均时延(usec)	iops
1fw 1fr 1gw 1gr 每一列的数据依次为 带宽(KB/s)	最大运行时间	最小运行时间
1fw 1fr 1gw 1gr 的iops请使用带宽/128自行计算
1hc 1hs 1hr 只有最终运行时间，请使用 testall_some.sh 中变量 op_size 的值除以 运行时间 得出iops

Tips：
1.如果出现类似于以下的报错：
    $ RTNETLINK answers: File exists 
    请使用如下命令，查看加入的时延
    $ tc qdisc show dev eno1
    之后使用如下命令，删除时延
    $ tc qdisc del dev eno1 root netem delay 时延
2.建议使用root用户跑测试，因为tc加时延可能需要root权限
3.如果想测试随机读写，请自行更改测试文件 :)


祝测试愉快 XD