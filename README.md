

# usbiss
A API and CLI tool to interface the [USB-ISS](http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm) for Linux and Windows.

<br/>
<center> <img src="./doc/readme/usb-iss.jpg.jpg" height="65%" width="65%" alt="Picture of USB-ISS adapter" title="USB-ISS adapter"/> </center>
<br/>

## Releases
| Version                                                                       | Date       | Source                                                                                                                  | Change log                         |
| ----------------------------------------------------------------------------- | ---------- | ----------------------------------------------------------------------------------------------------------------------- | ---------------------------------- |
| latest                                                                        |            | <a id="raw-url" href="">latest.zip</a> |                                    |
| [v0.0.1]() | tbd | <a id="raw-url" href="">v0.1.0.zip</a> | initial draft                      |


## [CLI](./usbiss_main.c)

### Arguments
| Argument                  | Description                                                                                                                 | Remark                                                                                    |
| ------------------------- | --------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| -p, --port=[UART]         | USB-ISS belonging _<UART>_ port                                                                                             | Windows: _COM1_ <br /> Linux: _/dev/ttyACM0_                                              |
| -b, --baud=[115200]       | communication speed Host/USB-ISS                                                                                            | UART baud rate                                                                            |
| -m, --mode=[I2C_S_100KHZ] | I2C transfer mode, use _usbiss -h_ for valid modes                                                                          | f.e. _I2C_H_400KHZ_                                                                       |
| -c, --command={cmd}       | I2C access to perform <br /> write: _adr7_ w _b0_ _bn_ <br /> read: _adr7_ r _cnt_ <br /> write-read: _adr7_ w _bn_ r _cnt_ | _adr7_: I2C slave address <br /> _bn_: write byte value <br />_cnt_: number of read bytes |
| -h, --help                | help                                                                                                                        |                                                                                           |
| --verbose                 | enable debug output                                                                                                         |                                                                                           |
| --brief                   | output only relevant USBISS responses                                                                                       | write: exit code only, read: read data only                                               |

### Run
This example modifies the EEPROM [24C256](https://ww1.microchip.com/downloads/en/devicedoc/doc0670.pdf) memory content.

#### Write
Write to device _0x50_ address _0_ the data _0x01 0x02 0x03_. The Arg eaccepts arbitary write data length, but keep the page overoll in this case in mind.

```bash
sudo ./bin/usbiss -m I2C_H_400KHz -c "0x50 w 0 0 0x01 0x02 0x03"
```

Following output:
```bash
[ INFO ]   USBISS started
[ OKAY ]   USBISS connected
             Baudrate : 115200
             Firmware : 0x09
             Serial   : 00060147
             Mode     : I2C_H_400KHZ
[ OKAY ]   Write 5 bytes to device 0x50
             0:  00 00 01 02 03 
[ OKAY ]   ended normally
```

#### Read
Read from device _0x50_ address _0_ 128 bytes.

```bash
sudo ./bin/usbiss -m I2C_H_400KHz -c "0x50 w 0 0 r 128"
```

Following output:
```bash
[ INFO ]   USBISS started
[ OKAY ]   USBISS connected
             Baudrate : 115200
             Firmware : 0x09
             Serial   : 00060147
             Mode     : I2C_H_400KHZ
[ OKAY ]   Write/Read interaction with device 0x50
           Write 2 Bytes
             0:  00 00 
           Read 128 Bytes
             00:  01 02 03 05 06 07 08 09  0a 0b 0c 0d 0e 0f 10 11 
             10:  12 13 14 15 16 17 18 19  1a 1b 1c 1d 1e 1f 20 21 
             20:  22 23 24 25 26 27 28 29  0a 0b 0c 0d 2e 2f 30 31 
             30:  32 33 34 35 36 37 38 39  3a 3b 3c 3d 3e 3f 40 41 
             40:  0a 0b 0c 0d 1b 71 a4 00  00 00 00 00 00 00 00 00 
             50:  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
             60:  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
             70:  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
[ OKAY ]   ended normally
```

## [API](./usbiss.h)






## References
  * [USB-ISS](http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm)
  * [Simple UART](https://github.com/AndreRenaud/simple_uart)
