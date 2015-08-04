# homeautom3G
Home Automation Cape for the BBB

To setup the Cape follow the instructions:

#Timezone
dpkg-reconfigure tzdata

#install mkeeprom.c (c) by AZKeller and create data.eeprom file
cat data.eeprom >/sys/bus/i2c/drivers/at24/1-0057/eeprom
#check for success
	cat /sys/bus/i2c/devices/1-0057/eeprom | hexdump -C
#go into the program folder where homeautom-3g.dts is located
dtc -O dtb -o /lib/firmware/homeautom-3g-00A0.dtbo -b 0 -@ homeautom-3g.dts
#check the listings in the pin groups
	cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups

#Update initrd
nano /etc/initramfs-tools/hooks/dtbo
<file dtbo>
  #!/bin/sh
  
  set -e
  
  . /usr/share/initramfs-tools/hook-functions
  
  # Copy Device Tree fragments
  mkdir -p "${DESTDIR}/lib/firmware"
  cp -p /lib/firmware/*.dtbo "${DESTDIR}/lib/firmware/"
</file>
nano /opt/scripts/tools/update_initrd.sh
<file update_initrd.sh>
  #!/bin/sh
  
  update-initramfs -u
  mkimage -A arm -O linux -T ramdisk -C none -a 0 -e 0 -n initramfs -d /boot/initrd.img-$(uname -r) /boot/uboot/initrd.img.new
</file>
sudo chmod +x /etc/initramfs-tools/hooks/dtbo
sudo chmod +x /opt/scripts/tools/update_initrd.sh
#In case of fail:
	apt-get install dos2unix
	dos2unix /opt/scripts/tools/update_initrd.sh
sudo /opt/scripts/tools/update_initrd.sh
sudo cp /boot/uboot/initrd.img /boot/uboot/initrd.img.bak
cd /boot/uboot
rm initrd.img && mv initrd.img.new initrd.img && ls -l
#See in file:
	lsinitramfs /boot/uboot/initrd.img

#Control USB power options
cd /sys/bus/usb/devices/*/power
#prevent the device from being autosuspended at all
i don't know how this is possible! workaround see net.service
#resume the device if it is already suspended
	cd /sys/bus/usb/devices/1-1.4/power && cat autosuspend
	echo -1 > autosuspend

#workaround
nano /etc/systemd/system/net.service
<file net.service>
  [Unit]
  Description=Network interfaces
  Wants=network.target
  Before=network.target
  BindsTo=sys-subsystem-net-devices-wlan0.device
  After=sys-subsystem-net-devices-wlan0.device
  
  [Service]
  Type=oneshot
  RemainAfterExit=yes
  ExecStart=/bin/sh -c "ifdown wlan0;sleep 5;ifup wlan0;echo -1 > /sys/bus/usb/devices/1-1/power/autosuspend;echo -1 > /sys/bus/usb/devices/1-1.4/power/autosuspend"
  ExecStop=/bin/sh -c "ifdown wlan0"
  
  [Install]
  WantedBy=multi-user.target
</file>
#Enable own service
systemctl enable net.service

Now you can create a autostart.service file to run the homeautom_3g firmware automatically
