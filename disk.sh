#!/bin/bash

NowTime=`date +"%Y-%m-%d_%H:%M:%S"`

eval $(df -T -m -x tmpfs -x devtmpfs | tail -n +2 | grep "dev" \
    | awk -v DiskSum=0 -v LeftSum=0 '{printf("paramount["NR"]=%d;parleft["NR"]=%d;\
    parname["NR"]=%s; usedperc["NR"]=%s;", $3, $5, $7, $6);DiskSum=DiskSum+$3;LeftSum=LeftSum+$5} \
END {printf("parnum=%d;DiskSum=%d;LeftSum=%d", NR, DiskSum, LeftSum)}')


for (( i = 1; i <= $parnum; i++)); do
    echo "$NowTime 1 ${parname[$i]} ${paramount[$i]} ${parleft[$i]} ${usedperc[$i]}"
done


UsedPercSum=$[ (100-$LeftSum*100/$DiskSum) ]
echo "$NowTime 0 disk $DiskSum $LeftSum ${UsedPercSum}%"
