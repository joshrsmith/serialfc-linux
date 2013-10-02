# serialfc-linux
This README file is best viewed [online](http://github.com/commtech/serialfc-linux/).

## Installing Driver

##### Downloading Driver Source Code
The source code for the SerialFC driver is hosted on Github code hosting. To check
out the latest code you will need Git and to run the following in a terminal.

```
git clone git://github.com/commtech/serialfc-linux.git serialfc
```

_You can also download driver packages directly from our
[website](http://www.commtech-fastcom.com/CommtechSoftware.html).

##### Switch To Stable Version
Now that you have the latest code checked out, switch to the latest stable
version v2.0.1 is only listed here as an example.

```
git tag
git checkout v2.0.1
```

##### Build Source Code
Run the make command from within the source code directory to build the driver.

```
cd serialfc/
make
```

If you would like to enable debug prints within the driver you need to add
the DEBUG option while building the driver.

```
make DEBUG=1
```

Once debugging is enabled you will find extra kernel prints in the
/var/log/messages and /var/log/debug log files.

If the kernel header files you would like to build against are not in the
default location `/lib/modules/$(shell uname -r)/build` then you can specify
the location with the KDIR option while building the driver.

```
make KDIR="/location/to/kernel_headers/"
```

##### Loading Driver
Assuming the driver has been successfully built in the previous step you are
now ready to load the driver so you can begin using it. To do this you insert
the driver's kernel object file (serialfc.ko) into the kernel.

```
insmod serialfc.ko
```

_You will more than likely need administrator privileges for this and
the following commands._

If no cards are present you will see the following message.

```
insmod serialfc.ko
insmod: error inserting 'serialfc.ko': -1 No such device
```

_All driver load time options can be set in your modprobe.conf file for
using upon system boot_

You can verify that the new ports were detected by checking the message log.

```
tail /var/log/kern.log
```

```
ttys5 at MMIO 0xd0004800 (irq = 21) is a 16550A
ttyS6 at MMIO 0xfeafc000 (irq = 17) is a ST16650
```

If you do not see any ports or if you do not see enough ports, it may be
that your kernel's defaults are set to only detect a too-small number of
serial ports.  This can be verified by looking at your current configuration
file usually found at `/boot/config-*`. Look for the defines for:
`CONFIG_SERIAL_8250_NR_UARTS` & `CONFIG_SERIAL_8250_RUNTIME_UARTS`.

If either of these numbers is smaller than the total number of serial ports
in the system (including the reserved ttyS0-3) then you will have trouble.

You can change the number of UARTs by modifying your grub config's kernel
line to say: `8250.nr_uarts=16` (or whatever number you like).

Alternatively you could recompile your kernel with those two config
lines modified appropriately.

More information on this can be found in the FAQ section below.


##### Installing Driver
If you would like the driver to automatically load at boot use the included
installer.

```
make install
```

This will also install the header (.h) files.

To uninstall, use the included uninstaller.

```
make uninstall
```

By default the FSCC driver boots up in synchronous communication mode. To
switch to the asynchronous mode you must modify the FSCC card's FCR register
to allow for asynchronous communication. There are multiple ways of doing
this. Possibly the simplest method is using sysfs and the command line.

```
echo 03000000 > /sys/class/fscc/fscc0/registers/fcr
```


## Quick Start Guide
There is documentation for each specific function listed below, but lets get started
with a quick programming example for fun.

_This tutorial has already been set up for you at_ 
[`serialfc/examples/tutorial.c`](https://github.com/commtech/serialfc-linux/tree/master/examples/serialfc.c).

Create a new C file (named tutorial.c) with the following code.

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int fd = 0;
    char odata[] = "Hello world!";
    char idata[20];

    /* Open port 0 (ttyS4) */
    fd = open("/dev/ttyS4", O_RDWR);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Send "Hello world!" text */
    write(fd, odata, sizeof(odata));

    /* Read the data back in (with our loopback connector) */
    read(fd, idata, sizeof(idata));

    fprintf(stdout, "%s\n", idata);

    close(fd);

    return EXIT_SUCCESS;
}
```

For this example I will use the gcc compiler, but you can use your compiler of
choice.

```
# gcc -I ..\lib\raw\ tutorial.c
```

Now attach the included loopback connector.

```
# ./a.out
Hello world!
```

You have now transmitted and received an asynchronous frame!


## API Reference

There are likely other configuration options you will need to set up for your
own program. All of these options are described on their respective documentation page.

- [Connect](https://github.com/commtech/serialfc-linux/blob/master/docs/connect.md)
- [Clock Rate](https://github.com/commtech/serialfc-linux/blob/master/docs/clock_rate.md)
- [Echo Cancel](https://github.com/commtech/serialfc-linux/blob/master/docs/echo_cancel.md)
- [External Transmit](https://github.com/commtech/serialfc-linux/blob/master/docs/external_transmit.md)
- [Frame Length](https://github.com/commtech/serialfc-linux/blob/master/docs/frame_length.md)
- [Isochronous](https://github.com/commtech/serialfc-linux/blob/master/docs/isochronous.md)
- [9-Bit Protocol](https://github.com/commtech/serialfc-linux/blob/master/docs/nine_bit.md)
- [RS485](https://github.com/commtech/serialfc-linux/blob/master/docs/rs485.md)
- [RX Trigger](https://github.com/commtech/serialfc-linux/blob/master/docs/rx_trigger.md)
- [TX Trigger](https://github.com/commtech/serialfc-linux/blob/master/docs/tx_trigger.md)
- [Sample Rate](https://github.com/commtech/serialfc-linux/blob/master/docs/sample_rate.md)
- [Termination](https://github.com/commtech/serialfc-linux/blob/master/docs/termination.md)
- [Write](https://github.com/commtech/serialfc-linux/blob/master/docs/write.md)
- [Read](https://github.com/commtech/serialfc-linux/blob/master/docs/read.md)
- [Disconnect](https://github.com/commtech/serialfc-linux/blob/master/docs/disconnect.md)


### FAQ

##### Why does my system not have enough /dev/ttyS nodes?
Some Linux distributions have the default number of serial ports that are
available at boot set to a small number (usually 4). The first four serial
ports are reserved so you will need to change this value to something larger
to be able to configure more serial ports.

There are a couple ways of doing this. The easiest method is by appending
'8250.nr_uarts=x' to your grub boot line. Something like this:

kernel /boot/vmlinuz-2.6.20-15-generic ro quiet splash 8250.nr_uarts=16

This can be done temporarily by pressing 'e' at the grub menu during boot or
by permanently modifying this value which is grub version specific. To do
this please search google for one of the numerous guides on the subject.

Another method is by editing the .config file of you kernel before compiling
it to allow for more serial ports. This is not preferred because you will
need to recompile the kernel for it to take effect. The line you need to
change in the .config file is SERIAL_8250_RUNTIME_UARTS.

##### How do I give my user account permissions to touch the serial ports?
```
adduser <username> dialout
```

##### How do I prevent `setserial` from caching old serial port settings?
In Debian based distributions you can reconfigure the setserial package  and set the default option to `manual`.
```
dpkg-reconfigure setserial
```


## Dependencies
- Base Installation: >= 2.6.16 (might work with a lower version)
- Sysfs Support: >= 2.6.25
- Other: gcc, make, kernel headers


## API Compatibility
We follow [Semantic Versioning](http://semver.org/) when creating releases.


## License

Copyright (C) 2013 [Commtech, Inc.](http://commtech-fastcom.com)

Licensed under the [GNU General Public License v3](http://www.gnu.org/licenses/gpl.txt).
