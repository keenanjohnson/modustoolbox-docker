#!/bin/sh

UART=1

NEW_ID=/sys/bus/usb-serial/drivers/ftdi_sio/new_id
UNBIND=/sys/bus/usb/drivers/ftdi_sio/unbind
BIND=/sys/bus/usb/drivers/ftdi_sio/bind

sudo modprobe ftdi_sio || exit 1

# make sure the vendor/product is included once
if ! grep -qs "$ID_VENDOR_ID $ID_MODEL_ID" $NEW_ID; then
    echo $ID_VENDOR_ID $ID_MODEL_ID > $NEW_ID
fi

# detach uart interface from driver
echo -n "$1:1.$UART" > $UNBIND

# re-bind the uart to driver
echo -n "$1:1.$UART" > $BIND
