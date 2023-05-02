#!/bin/sh
oldpwd=$(pwd)
cd $(dirname $0)

sudo cp 67-wiced-JTAG.rules /etc/udev/rules.d/
sudo chmod a-x /etc/udev/rules.d/67-wiced-JTAG.rules
sudo mkdir -p /opt/cypress
sudo cp cypress_ftdi.sh /opt/cypress/
sudo cp cypress_cdc_acm.sh /opt/cypress/
sudo chmod a+x /opt/cypress/cypress_ftdi.sh
sudo chmod a+x /opt/cypress/cypress_cdc_acm.sh
sudo udevadm control -R

cd $oldpwd
