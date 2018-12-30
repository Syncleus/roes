You may need to run this first to upload to arduino (root may not be needed):

stty -F /dev/ttyACM0 cs8 9600 ignbrk -brkint -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

To upload to arduino

avrdude -V -p atmega2560 -C /etc/avrdude.conf -D -c avrispmkII -b 115200 -P /dev/ttyACM0 -U flash:w:/home/freemo/source/roes/bin/mega2560//src.hex:i
