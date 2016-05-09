#!/bin/bash

num_lines=$(samtools view dx:SRR504516.bam 10:1000000-2000000 | wc -l)
if [ $num_lines != 35548 ]; then
    exit 1
fi

md5_hash=$(samtools view dx:SRR504516.bam 10:1000000-2000000 | md5sum | awk '{print $1}')
if [ $md5_hash != "10a7aece2b3b70a5850026065cca65f7" ]; then
    exit 1
fi
