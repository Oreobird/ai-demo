

#!/bin/bash
echo "start to shut down face_quality process"

PID=$(ps -ef |grep face_quality|grep java|awk '{print $2}')

if [ $? -eq 0 ]; then
    echo "process id:$PID"
else
    echo "process face_quality not exit"
    exit
fi


kill -9 ${PID}

if [ $? -eq 0 ];then
    echo "kill face_quality success"
else
    echo "kill face_quality fail"
fi
