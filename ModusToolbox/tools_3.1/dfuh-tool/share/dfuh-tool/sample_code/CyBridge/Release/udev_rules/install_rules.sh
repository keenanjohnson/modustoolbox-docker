#!/bin/sh
oldpwd=$(pwd)
cd $(dirname $0)
sudo cp 57-cypress_programmer.rules /etc/udev/rules.d/
sudo service udev restart
sudo udevadm control --reload
sudo udevadm trigger --action=add
cd $oldpwd
