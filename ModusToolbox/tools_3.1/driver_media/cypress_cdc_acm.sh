#!/bin/sh

UART=1

NEW_ID=/sys/bus/usb-serial/drivers/cdc_acm/new_id
UNBIND=/sys/bus/usb/drivers/cdc_acm/unbind
BIND=/sys/bus/usb/drivers/cdc_acm/bind

sudo modprobe cdc_acm || exit 1

# make sure the vendor/product is included once
if ! grep -qs "$ID_VENDOR_ID $ID_MODEL_ID" $NEW_ID; then
    echo $ID_VENDOR_ID $ID_MODEL_ID > $NEW_ID
fi

# detach uart interface from driver
echo -n "$1:1.$UART" > $UNBIND

# re-bind the uart to driver
echo -n "$1:1.$UART" > $BIND
