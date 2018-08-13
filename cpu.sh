#!/bin/bash
NowTime=`date +"%Y-%m-%d %H:%M:%S"`
#echo $NowTime

LoadAvg=`cut -d " " -f 1-3 /proc/loadavg`
CpuTemp=`cat /sys/class/thermal/thermal_zone0/temp`
CpuTemp=`echo "scale=2;$CpuTemp / 1000" | bc`
eval $(head -n 1 /proc/stat | awk -v sum1=0 -v idle1=0 '{for(i=2;i<=8;i++){sum1=sum1+$i} printf("sum1=%d; idle1=%d;", \
	sum1, $5)}')

sleep 0.5

#echo $sum1 $idle1

eval $(head -n 1 /proc/stat | awk -v sum2=0 -v idle2=0 '{for(i=2;i<=8;i++){sum2=sum2+$i} printf("sum2=%d; idle2=%d;", \
	sum2, $5)}')

CpuUsedPerc=`echo "scale=4;(1-($idle1-$idle2)/($sum1-$sum2))*100" | bc`

CupUsedPerc=`printf "%.2lf\n" $CpuUsedPerc`

WarnLevel="normal"

if [[ `echo "$CpuTemp >= 85" | bc -l` = 1 ]]; then
	WarnLevel="fault"
elif [[ `echo "$CpuTemp >= 70" | bc -l` = 1 ]]; then
	WarnLevel="waring"
elif [[ `echo "$CpuTemp >= 50" | bc -l` = 1 ]]; then
	WarnLevel="note"
fi
echo "$NowTime  $LoadAvg  $CpuUsedPerc  摄氏度${CpuTemp}  $WarnLevel"

