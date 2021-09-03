# This file is for executing many binary files simultaneously.
cd /home/ubuntu/BroadcastTest/Publisher/
sudo sh build.sh
cd /home/ubuntu/BroadcastTest/Subscriber/
sudo sh build.sh

cd /home/ubuntu/BroadcastTest/
Subscriber/BroadcastSubExample 01 &
Subscriber/BroadcastSubExample 02 &
Subscriber/BroadcastSubExample 03 &
Subscriber/BroadcastSubExample 04 &
Subscriber/BroadcastSubExample 05 &
Subscriber/BroadcastSubExample 06 &
Subscriber/BroadcastSubExample 07 &
Subscriber/BroadcastSubExample 08 &
Subscriber/BroadcastSubExample 09 &
Subscriber/BroadcastSubExample 10 &
Subscriber/BroadcastSubExample 11 &
Subscriber/BroadcastSubExample 12 &
Subscriber/BroadcastSubExample 13 &
Subscriber/BroadcastSubExample 14 &
Subscriber/BroadcastSubExample 15 &
Subscriber/BroadcastSubExample 16 &
Subscriber/BroadcastSubExample 17 &
Subscriber/BroadcastSubExample 18 &
Subscriber/BroadcastSubExample 19 &
Subscriber/BroadcastSubExample 20 &
sleep 3
Publisher/BroadcastPubExample