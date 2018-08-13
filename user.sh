#ret=`cat /etc/sudoers | grep -v "#" | grep -v "Defaults"`


TIME=`date +'%Y-%m-%d %H:%M:%S'`
USER_CNT=`cat /etc/passwd | awk -F : -v sum=0 '{if($3 >=1000 && $3 <= 65535)sum++} END {printf("%d", sum)}'`
USER_Activity=(`last | grep -v "reboot" | \
    cut -d " " -f 1 | uniq | sort | tail -n 3`)

echo $TIME $USER_CNT $USER_Activity

