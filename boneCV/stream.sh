#! /bin/bash
STREAM_URL="rtmp://live.justin.tv/app/live_54496724_3qqDdB7J04WgaJmhO8qvbkpbYwOmYZ"

ffmpeg -i "udp://192.168.1.4:1234" -an -vcodec copy -vb 256k -g 60 -me_method:v zero -r:v 20 -acodec aac -f flv "$STREAM_URL" -loglevel debug
 
