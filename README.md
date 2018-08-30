iButton PAM module authorization for Linux
---------------------------------------------------------------------------------

Imagine a system, which provides security authorization at any Linux system without classical password system.
It will be usefull for persons who don`t prefer type a long passwords in a password field,won't it?

Our system provides authorization in your personal Linux system via iButton keys or any 1-Wire device. 

The main idea it is login your personal Linux system via iButton keys. 
The iButton keys are contained in **/etc/ibutton_pam.config, field: ibutton_id_database** 
on the Linux side.Via this config, user is able to add any number of persons and keys. 

All keys are encrypted via hmac-sha256 and the hmac_key which user can configure in **/etc/ibutton_pam.config, field: hmac_key**.

Already encrypted struct of keys(struct ibutton_keys_) are compared with the transmitted by UART interface keys.

For tracking a new connections is used "inotify".

Moreover user can deploy the time to login via iButton key in seconds.(/etc/ibutton_pam.config, field: time_to_live).

For example if user hasn`t time to login via key or connection to device, he will have a chance to login via Linux password.

Pay attention!
You are able to employ your pam module for you intetions before install.See **ibutton_pam.config**.

Which device does we need?

See [this appliance](https://github.com/Jayawardenepura/GL-C-Embeedded/tree/master/ibutton-lock).

In brief, this device will transmit hash from ibutton keys,which are stored in MCU EEPROM.

## Usage

Build project:

- $make all 

Set config and module:
- $make install

Clean:
- $make clean

If you wanna use this module for greeter (ARCHE) you should configure your /etc/pam.d/lightdm:
https://wiki.archlinux.org/index.php/LightDM

For security you should limit permissions for the main ibutton_pam.config

**$chmod 660 /etc/ibutton_pam.config**

The one unsolved feature it is non-static link COM port:

How to solve it see [here](https://playground.arduino.cc/Linux/Udev).

Documentations is really ungly,i`m so sorry. We work on this.

