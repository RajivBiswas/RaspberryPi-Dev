# Userspace IOCTL Program to configure, control & perform Audio Playback with VS1053/VS1003 Audio Codec.

IOCTL Userspace Program to control, configure & perform Audio playback with Audio 
Codec.\Please create a new directory on your Rpi3 Home path. Copy the sources of the directory\
to the new directory.\
After, this on your Rpi3 Board, inside the sources of this folder, type,
```
make clean
make all
```
This will build the User space program for you on your Rpi3. To execute the program on your\
Rpi3, type,
```
./ioctl --help
```
This will list out all the options provided with the Program.
```
			"  reset\n"
			"  getscireg    regno\n"
			"  setscireg    regno msb lsb\n"
			"  getclockf\n"
			"  setclockf    mul add clk\n"
			"  getvolume\n"
			"  setvolume    left right\n"
			"  gettone\n"
			"  settone      tb tl bb bl\n"
			"  getinfo\n"
			"  play\n\n"
```
First, the User shall change the volume using,
```
./ioctl getvolume
./ioctl setvolume 220 220
./ioctl getvolume
```
The First 'getvolume' shall show 255, 255 as Left & Right channel Volume level values.\
Upon setting the volume levels as 220 & 220 for Left & Right channels & then Reading\
them using 'getvolume' command should successfully Read the volume levels as 220 & 220\
for Left & Right channel values. This is also a way to ensure correctness of our Linux\
Device Driver.

Please enusre that after 'insmod vs10xx.ko' there is a device node for vs10xx in /dev\
directory. By default the User space program assumes it to be in /dev/vs10xx-0.

If anything other than this is there, then the above command can be also given as,
```
./ioctl -d /dev/vs10xx-0 getvolume
./ioctl -d /dev/vs10xx-0 setvolume 220 220
./ioctl -d /dev/vs10xx-0 getvolume
```
After, setting the Volume level, we can play an Mp3 file encoded at 128bps, 44100 Hz\
Sampling Rate, Stereo, Layer 3 Mp3 file as,
```
./ioctl play
```
User needs to change the Path of the Audio file. Currently, i have hard coded it in\
ioctl.c as,
```
char filename[] = "/home/pi/Music/NinjaTuna128k.mp3";
```
Also, find the file NinjaTuna128k.mp3 present in this same git folder.\
Some important FFMPEG Commands to create 128kbps, 44100 Hz stereo Mp3 files is as\
given below:
```
To convert 192kbps Mp3 to 128kbps Mp3:
ffmpeg -i NinjaTuna.mp3 -b:a 128k -f mp3 NinjaTuna128k.mp3

To convert WAV to 128kbps Mp3:
ffmpeg -i input.wav -vn -ar 44100 -ac 2 -ab 192k -f mp3 output.mp3
```

Regards,\
Rajiv.
