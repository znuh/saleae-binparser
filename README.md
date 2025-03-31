# saleae-binparser
Fast parser for Saleae Logic 2 binary export format (Digital only)

## Requirements
Originates from Linux, works on FreeBSD as well.

Other BSDs might need minor `#ifdef` additions.

## Performance
Processing 21GiB of data takes approximately 30 seconds on a Ryzen7 with sufficient RAM and/or a fast NVMe.

## Usage
```
> ls ~/saleae_export/
digital_0.bin   digital_13.bin  digital_1.bin  digital_4.bin  digital_7.bin
digital_10.bin  digital_14.bin  digital_2.bin  digital_5.bin  digital_9.bin
digital_11.bin  digital_15.bin  digital_3.bin  digital_6.bin
> make
> ./example ~/saleae_export/digital
found channel  0 - initial state: 0,    163505214 transitions
found channel  1 - initial state: 0,    159729950 transitions
found channel  2 - initial state: 0,    159764716 transitions
found channel  3 - initial state: 0,    159629110 transitions
found channel  4 - initial state: 0,    162059466 transitions
found channel  5 - initial state: 0,    166748156 transitions
found channel  6 - initial state: 0,    158228608 transitions
found channel  7 - initial state: 0,    161867646 transitions
found channel  9 - initial state: 0,      7628523 transitions
found channel 10 - initial state: 0,       355041 transitions
found channel 11 - initial state: 0,      1069426 transitions
found channel 13 - initial state: 0,   1503871087 transitions
found channel 14 - initial state: 0,       314727 transitions
found channel 15 - initial state: 0,      1170779 transitions
14 channels found
initial state: 0000
...
```
