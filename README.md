[![Linux](https://github.com/andkae/usbiss/actions/workflows/linux.yml/badge.svg)](https://github.com/andkae/usbiss/actions/workflows/linux.yml) [![Windows](https://github.com/andkae/usbiss/actions/workflows/windows.yml/badge.svg)](https://github.com/andkae/usbiss/actions/workflows/windows.yml) [![doxygen](https://github.com/andkae/usbiss/actions/workflows/doxygen.yml/badge.svg)](https://github.com/andkae/usbiss/actions/workflows/doxygen.yml)

- [usbiss](#usbiss)
  * [Features](#features)
  * [Releases](#releases)
  * [CLI](#cli)
    + [Arguments](#arguments)
    + [Test](#test)
    + [Run](#run)
      - [Scan](#scan)
      - [Write](#write)
      - [Read](#read)
  * [API](#api)
    + [List](#list)
    + [Init](#init)
    + [Verbose](#verbose)
    + [Open](#open)
    + [Close](#close)
    + [Mode](#mode)
    + [I2C-Scan](#i2c-scan)
    + [I2C-Write](#i2c-write)
    + [I2C-Read](#i2c-read)
    + [I2C-Write-Read](#i2c-write-read)
  * [Acknowledgment](#acknowledgment)
  * [References](#references)


# usbiss
[USB-ISS](http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm) C driver API and CLI tool for Linux and Windows.

<center> <img src="./doc/readme/usb-iss.jpg" height="35%" width="35%" alt="Picture of USB-ISS adapter" title="USB-ISS adapter"/> </center>
<br/>


## Features
* CLI interface to read/write arbitrary number of bytes from I2C slave
* Windows/Linux support
* [Prebuild](https://github.com/andkae/usbiss/releases) Windows executable for command line
* Easy to use C-API for project integration


## Releases
| Version                                                | Date       | Source                                                                                              | Change log                                                                                                                                         |
| ------------------------------------------------------ | ---------- | --------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| latest                                                 |            | <a id="raw-url" href="https://github.com/andkae/usbiss/archive/refs/heads/main.zip">latest.zip</a>  |                                                                                                                                                    |
| [v0.3.0](https://github.com/andkae/usbiss/tree/v0.3.0) | 2023-11-14 | <a id="raw-url" href="https://github.com/andkae/usbiss/archive/refs/tags/v0.3.0.zip">v0.3.0.zip</a> | update [Simple UART](https://github.com/AndreRenaud/simple_uart) <br /> match USB-ISS uart port by VID/PID                                         |
| [v0.2.0](https://github.com/andkae/usbiss/tree/v0.2.0) | 2023-10-21 | <a id="raw-url" href="https://github.com/andkae/usbiss/archive/refs/tags/v0.2.0.zip">v0.2.0.zip</a> | introduce CLI option '-t' and '-l' <br /> use first found UART port as default <br /> USB-ISS firmware revision check                              |
| [v0.1.0](https://github.com/andkae/usbiss/tree/v0.1.0) | 2023-08-27 | <a id="raw-url" href="https://github.com/andkae/usbiss/archive/refs/tags/v0.1.0.zip">v0.1.0.zip</a> | fix [Simple UART](https://github.com/AndreRenaud/simple_uart) compile warnings <br /> check USB-ISS status bytes before read <br /> add Doxygen CI |
| [v0.0.1](https://github.com/andkae/usbiss/tree/v0.0.1) | 2023-08-04 | <a id="raw-url" href="https://github.com/andkae/usbiss/archive/refs/tags/v0.0.1.zip">v0.0.1.zip</a> | initial draft                                                                                                                                      |


## [CLI](./usbiss_main.c)

### Arguments
| Argument                  | Description                                                                                                                 | Remark                                                                                    |
| ------------------------- | --------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| -p, --port=[first]        | USB-ISS belonging port, _usbiss -l_ for valid options                                                                       | default: first found UART port                                                            |
| -b, --baud=[230400]       | communication speed Host/USB-ISS                                                                                            | UART baud rate                                                                            |
| -m, --mode=[I2C_S_100KHZ] | I2C transfer mode, use _usbiss -h_ for valid modes                                                                          | f.e. _I2C_H_400KHZ_                                                                       |
| -c, --command={cmd}       | I2C access to perform <br /> write: _adr7_ w _b0_ _bn_ <br /> read: _adr7_ r _cnt_ <br /> write-read: _adr7_ w _bn_ r _cnt_ | _adr7_: I2C slave address <br /> _bn_: write byte value <br />_cnt_: number of read bytes |
| -s, --scan=[0x3:0x77]     | scan I2C bus for devices                                                                                                    | default: scan address range 0x3 to 0x77                                                   |
| -h, --help                | help                                                                                                                        |                                                                                           |
| -v, --version             | output USBISS revision                                                                                                      |                                                                                           |
| -l, --list                | list USB-ISS suitable ports                                                                                                 |                                                                                           |
| -t, --test                | checks USB-ISS connection                                                                                                   |                                                                                           |
| --verbose                 | enable debug output                                                                                                         |                                                                                           |
| --brief                   | output only relevant USBISS responses                                                                                       | write: exit code only, read: read data only                                               |

### Test
Checks the connection between PC and USB-ISS:
```bash
sudo ./bin/usbiss -t
```

Following output:
```bash
[ INFO ]   USBISS started
[ OKAY ]   USBISS connected
             Baudrate : 230400
             Firmware : 0x09
             Serial   : 00060147
[ OKAY ]   ended normally
```

### Run
This example uses the EEPROM [24C256](https://ww1.microchip.com/downloads/en/devicedoc/doc0670.pdf) as exclusively
device on the USB-ISS.

#### Scan
Scans I2C bus for devices.

```bash
sudo ./bin/usbiss -m I2C_H_400KHz -s
```

Following output:
```bash
[ INFO ]   USBISS started
[ OKAY ]   USBISS connected
             Port     : COM6
             Baudrate : 230400
             Firmware : 0x09
             Serial   : 00060147
             Mode     : I2C_H_400KHZ
[ OKAY ]   Scan I2C bus in range 0x3:0x77
                  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
             00:          -- -- -- -- -- -- -- -- -- -- -- -- --
             10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
             20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
             30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
             40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
             50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
             60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
             70: -- -- -- -- -- -- -- --
[ OKAY ]   ended normally
```

#### Write
Write to device _0x50_ address _0_ the data _0x01 0x02 0x03_. The Arg accepts arbitary write data length, but keep the page overoll in this case in mind.

```bash
sudo ./bin/usbiss -m I2C_H_400KHz -c "0x50 w 0 0 0x01 0x02 0x03"
```

Following output:
```bash
[ INFO ]   USBISS started
[ OKAY ]   USBISS connected
             Baudrate : 230400
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
             Baudrate : 230400
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

### List
```c
int usbiss_list_uart( char *str, size_t len, const char sep[] );
```

List USB-ISS suitable UART ports.

#### Arguments:
| Arg | Description                    |
| --- | ------------------------------ |
| str | Found UART ports on system     |
| len | max length of string           |
| sep | separator between listed ports |

#### Return:
Number of found UART ports in system.

### Init
```c
int usbiss_init( t_usbiss *self );
```

Initialize USB-ISS handle.

### Verbose
```c
void usbiss_set_verbose( t_usbiss *self, uint8_t verbose );
```

Set message level of driver.

| Arg     | Description                                                                            |
| ------- | -------------------------------------------------------------------------------------- |
| verbose | Advanced debug information <br /> 0: no debug output <br /> 1: debug output via printf |

### Open
```c
int usbiss_open( t_usbiss *self, char* port, uint32_t baud );
```

Open connection to USB-ISS.

| Arg                         | Description                                                                  |
| --------------------------- | ---------------------------------------------------------------------------- |
| port=[COM1 \| /dev/ttyACM0] | System path to USB-ISS belonging UART. Provide empty string _""_ for default |
| baud=[115200]               | Baud rate of UART connection. Provide _0_ for default                        |

### Close
```c
int usbiss_close( t_usbiss *self );
```

Close connection to USB-ISS.

### Mode
```c
int usbiss_set_mode( t_usbiss *self, const char* mode );
```

Setup USB-ISS transfer mode.
_Note: Currently only I2C modes supported._

| Arg                         | Description                                                                                                                                                                                  |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| mode=[USBISS_I2C_S_100KHZ]  | I2C Standard: _USBISS_I2C_S_20KHZ_, _USBISS_I2C_S_50KHZ_, _USBISS_I2C_S_100KHZ_, _USBISS_I2C_S_400KHZ_ <br /> I2C Fast: _USBISS_I2C_H_100KHZ_, _USBISS_I2C_H_400KHZ_, _USBISS_I2C_H_1000KHZ_ |

### I2C-Scan
```c
int usbiss_i2c_scan( t_usbiss *self, int8_t start, int8_t stop, int8_t *i2c, uint8_t len );
```

Scan given I2C address range for I2C devices.

| Arg   | Description                         |
| ----- | ----------------------------------- |
| start | scan start address                  |
| stop  | scan stop address                   |
| i2c   | array of found i2c device addresses |
| len   | _i2c_ array size                    |

### I2C-Write
```c
int usbiss_i2c_wr( t_usbiss *self, uint8_t adr7, void* data, size_t len );
```

Write arbitrary number of bytes to I2C slave.

| Arg  | Description                    |
| ---- | ------------------------------ |
| adr7 | I2C slave address (7Bit)       |
| data | array with write data          |
| len  | number of bytes in write array |

### I2C-Read
```c
int usbiss_i2c_rd( t_usbiss *self, uint8_t adr7, void* data, size_t len );
```

Read arbitrary number of bytes from I2C slave.

| Arg  | Description                    |
| ---- | ------------------------------ |
| adr7 | I2C slave address (7Bit)       |
| data | array with read data           |
| len  | number of requested read bytes |

### I2C-Write-Read
```c
int usbiss_i2c_wr_rd( t_usbiss *self, uint8_t adr7, void* data, size_t wrLen, size_t rdLen );
```

Write arbitrary number of bytes to I2C slave, sent repeated start and read arbitrary number of bytes.
Write and Read data takes place in the same _data_ buffer.

| Arg   | Description                |
| ----- | -------------------------- |
| adr7  | I2C slave address (7Bit)   |
| data  | array with write/read data |
| wrLen | number of bytes to write   |
| rdLen | number of bytes for read   |


## Acknowledgment

Special thanks to [AndreRenaud](https://github.com/AndreRenaud) for providing [simple_uart](https://github.com/AndreRenaud/simple_uart). This
module allows us to run [USBISS](https://github.com/andkae/usbiss) for Linux and Windows.


## References
  * [USB-ISS](http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm)
  * [Simple UART](https://github.com/AndreRenaud/simple_uart)
