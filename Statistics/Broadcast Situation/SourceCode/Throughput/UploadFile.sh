# This file is for uploading source code to aws ec2 conveniently
serverip=""
fileDir=""
fileName=""

scp -i /mnt/c/Users/${fileDir}/${fileName}.pem Publisher/Broadcast.idl ubuntu@${serverip}:/home/ubuntu/BroadcastTest/Publisher/Broadcast.idl
scp -i /mnt/c/Users/${fileDir}/${fileName}.pem Publisher/Broadcast_main.cpp ubuntu@${serverip}:/home/ubuntu/BroadcastTest/Publisher/Broadcast_main.cpp

scp -i /mnt/c/Users/${fileDir}/${fileName}.pem Subscriber/Broadcast.idl ubuntu@${serverip}:/home/ubuntu/BroadcastTest/Subscriber/Broadcast.idl
scp -i /mnt/c/Users/${fileDir}/${fileName}.pem Subscriber/Broadcast_main.cpp ubuntu@${serverip}:/home/ubuntu/BroadcastTest/Subscriber/Broadcast_main.cpp

