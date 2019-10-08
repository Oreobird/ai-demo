#!/bin/bash
cd /data/workspace/ai-face-quality
nohup java -jar face_quality.jar >/dev/null 2>&1 & 
if [ $? -eq 0 ];then
    echo "ai-face-quality start sucess"
else
    echo "ai-face-quality start failed"
fi

