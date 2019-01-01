You may need to run this first to upload to arduino (root may not be needed):

stty -F /dev/ttyACM0 cs8 9600 ignbrk -brkint -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

To upload to arduino

/home/freemo/.arduino15/packages/arduino/tools/bossac/1.6.1-arduino/bossac --info --erase --write --verify --reset -U false -b --port=ttyACM0 /home/freemo/source/roes/bin/arduino_due_x/src/src.bin

or just

make upload
