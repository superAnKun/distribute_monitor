#!/bin/bash
TIME=`date +'%Y-%m-%d %H:%M:%S'`
HOSTNAME=`hostname`
VERSION=`host -V`
RUNTIME=`uptime | cut -d "," -f 1`
AVERGE_LOAD=`uptime | cut -d "," -f 4`
mem_arry=(`free -m | head -n 2 | tail -n 1 | awk '{printf("%d %d %d", $2, $3, $4)}'`)
total_mem=${mem_arry[0]}
used=${mem_arry[1]}
mem_rat=`echo "scale=3; $used/$total_mem * 100" | bc`

#磁盘总量
total_disk=`df -h | grep -v tmpfs | tail -n 2 | awk -v sum=0 '{sum=sum + $2} END {printf("%f\n", sum)}'`


used_disk=`df -h | grep -v tmpfs | tail -n 2 | awk -v sum=0 '{sum=sum + $3} END {printf("%f\n", sum)}'`
disk_rat=`echo "scale=2; $used_disk / $total_disk * 100" | bc`


CpuTemp=`cat /sys/class/thermal/thermal_zone0/temp`
CpuTemp=`echo "scale=2;$CpuTemp / 1000" | bc`


DiskLevel="normal"
if [[ `echo "$disk_rat >= 90" | bc -l` = 1 ]]; then
	DiskLevel="warning"
elif [[ `echo "$disk_rat >= 80" | bc -l` = 1 ]]; then
	DiskLevel="note"
fi

MemLevel="normal"

if [[ `echo "$mem_rat >= 90" | bc -l` = 1 ]]; then
	MemLevel="warning"
elif [[ `echo "$mem_rat >= 80" | bc -l` = 1 ]]; then
	MemLevel="note"
fi

CpuLevel="normal"
if [[ `echo "$CpuTemp >= 90" | bc -l` = 1 ]]; then
	CpuLevel="warning"
elif [[ `echo "$CpuTemp >= 80" | bc -l` = 1 ]]; then
	CpuLevel="note"
fi

echo $TIME $HOSTNAME $VERSION $RUNTIME $AVERGE_LOAD $total_disk $disk_rat"%" $mem_total $mem_rat"%"  $CpuTemp $DiskLevel $MemLevel $CpuLevel

