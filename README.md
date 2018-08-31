iButton PAM module authorization for Linux
---------------------------------------------------------------------------------

Imagine a system, which provides security authorization at any Linux system without classical password system.
It will be usefull for persons who don`t prefer type a long passwords in a password field,won't it?

Our system provides authorization in your personal Linux system via iButton keys or any 1-Wire device. 

The main idea it is login your personal Linux system via iButton keys. 
The iButton keys are contained in a special database-config in a linux direction, which user is able to config.

All keys are encrypted via hmac-sha256.It is a specific type of message authentication code (MAC) involving a cryptographic hash function and a secret cryptographic key.

The hash from the database`s keys is compared with serial input data by 64 symbols each.

For detecting a new serial connections is used "inotify".Inotify is a Linux kernel subsystem that acts to extend filesystems to notice changes to the filesystem, and report those changes to applications.

Availability to use password login after a timeout.(default 30 seconds for avr connection and 30 seconds for key)

Pay attention!
You are able to configure your pam module for you intentions before installation.See **ibutton_pam.config**.

Which device does we need?

See [this appliance](https://github.com/Jayawardenepura/iButton_lock).

About. In brief, this device is transmitting hash from ibutton keys, which are stored in an MCU EEPROM.

## Usage

Build project:

- $make all 

Set config and module:
- $make install

Clean:
- $make clean

If you wanna use this module for greeter (ARCHE Linux distribution) you should configure /etc/pam.d/lightdm:
https://wiki.archlinux.org/index.php/LightDM , field **Enabling interactive passwordless login**

Your /etc/pam.d/lightdm should looks like this:
```#%PAM-1.0
**auth        sufficient  **identify.so** user ingroup nopasswdlogin**
auth        include     system-login
......
```

For security you should limit permissions for the /etc/ibutton_pam.config

```$chmod 660 /etc/ibutton_pam.config```

The one unsolved feature it is non-static link COM port:

How to solve it for your arduino device see [here](https://playground.arduino.cc/Linux/Udev).

Documentations refman is really ungly,i`m so sorry. We work on this.

