//compile:gcc -Wall -Wextra -Werror -pedantic -O3 $(pkg-config --cflags --libs
// libusb-1.0) -o $A0 $SRC
//dev:gcc -Wall -Wextra -Werror -pedantic -fsanitize=address
// -fsanitize=undefined -g $(pkg-config --cflags --libs libusb-1.0) -o $A0 $SRC

#include <libusb.h>
#include <stdio.h>
#include <unistd.h>

// this may need to be changed depending on your controller
#define VID 0x0e6f
#define PID 0x0201

unsigned char buf[BUFSIZ];

void logging_cb(__attribute__((unused)) libusb_context *ctx,
                enum libusb_log_level level, const char *str) {
  fprintf(stderr, "[LOG %d] %s", level, str);
}

int print_string_descriptor(struct libusb_device_handle *dev_handle,
                            const char *name, int i) {
  if (i < 1) {
    printf("%s: (unknown)\n", name);
    return 0;
  }
  int err = libusb_get_string_descriptor_ascii(dev_handle, i, buf, BUFSIZ - 1);
  if (err < 0) {
    fprintf(stderr, "libusb_get_string_descriptor_ascii(): %s\n",
            libusb_error_name(err));
    return -1;
  }
  buf[err] = '\0';
  printf("%s: %s\n", name, buf);
  return 0;
}

int print_device_descriptor(struct libusb_device_handle *dev_handle,
                            const struct libusb_device_descriptor *dev_desc) {
  int err;

  printf("\n=== DEVICE DESCRIPTOR ===\n");
  printf("bcdUSB: 0x%x\n", dev_desc->bcdUSB);
  printf("bDeviceClass: 0x%x\n", dev_desc->bDeviceClass);
  printf("bDeviceSubClass: 0x%x\n", dev_desc->bDeviceSubClass);
  printf("bDeviceProtocol: 0x%x\n", dev_desc->bDeviceProtocol);
  printf("bMaxPacketSize0: %d\n", dev_desc->bMaxPacketSize0);
  printf("idVendor: 0x%04x\n", dev_desc->idVendor);
  printf("idProduct: 0x%04x\n", dev_desc->idProduct);
  printf("bcdDevice: 0x%x\n", dev_desc->bcdDevice);
  err = print_string_descriptor(dev_handle, "iManufacturer",
                                dev_desc->iManufacturer);
  if (err < 0)
    return -1;
  err = print_string_descriptor(dev_handle, "iProduct", dev_desc->iProduct);
  if (err < 0)
    return -1;
  err = print_string_descriptor(dev_handle, "iSerialNumber",
                                dev_desc->iSerialNumber);
  if (err < 0)
    return -1;
  printf("bNumConfigurations: %d\n", dev_desc->bNumConfigurations);
  return 0;
}

int print_configuration_descriptor(
    struct libusb_device_handle *dev_handle,
    const struct libusb_config_descriptor *config_desc) {
  printf("\n=== CONFIGURATION DESCRIPTOR ===\n");
  printf("wTotalLength: %d\n", config_desc->wTotalLength);
  printf("bNumInterfaces: %d\n", config_desc->bNumInterfaces);
  printf("bConfigurationValue: %d\n", config_desc->bConfigurationValue);
  int err = print_string_descriptor(dev_handle, "iConfiguration",
                                    config_desc->iConfiguration);
  if (err < 0)
    return -1;
  printf("bmAttributes: %d\n", config_desc->bmAttributes);
  printf("MaxPower: %d\n", config_desc->MaxPower);
  printf("extra_length: %d\n", config_desc->extra_length);
  for (int i = 0; i < config_desc->extra_length; i++) {
    printf("%02x ", config_desc->extra[i]);
  }
  printf("\n");
  return 0;
}

int print_interface_descriptor(
    struct libusb_device_handle *dev_handle,
    const struct libusb_interface_descriptor *int_desc) {
  printf("\n=== INTERFACE %d DESCRIPTOR ===\n", int_desc->bInterfaceNumber);
  printf("bInterfaceNumber: %d\n", int_desc->bInterfaceNumber);
  printf("bAlternateSetting: %d\n", int_desc->bAlternateSetting);
  printf("bNumEndpoints: %d\n", int_desc->bNumEndpoints);
  printf("bInterfaceClass: 0x%x\n", int_desc->bInterfaceClass);
  printf("bInterfaceSubClass: 0x%x\n", int_desc->bInterfaceSubClass);
  printf("bInterfaceProtocol: 0x%x\n", int_desc->bInterfaceProtocol);
  int err =
      print_string_descriptor(dev_handle, "iInterface", int_desc->iInterface);
  if (err < 0)
    return -1;
  printf("extra_length: %d\n", int_desc->extra_length);
  for (int i = 0; i < int_desc->extra_length; i++) {
    printf("%02x ", int_desc->extra[i]);
  }
  printf("\n");
  return 0;
}

void print_endpoint_descriptor(
    const struct libusb_endpoint_descriptor *ep_desc) {
  printf("\n=== ENDPOINT 0x%x DESCRIPTOR ===\n", ep_desc->bEndpointAddress);
  printf("bEndpointAddress: 0x%x\n", ep_desc->bEndpointAddress);
  printf("bmAttributes: 0x%x\n", ep_desc->bmAttributes);
  printf("wMaxPacketSize: %d\n", ep_desc->wMaxPacketSize);
  printf("bInterval: %d\n", ep_desc->bInterval);
  printf("extra_length: %d\n", ep_desc->extra_length);
  for (int i = 0; i < ep_desc->extra_length; i++) {
    printf("%02x ", ep_desc->extra[i]);
  }
  printf("\n");
}

int main(int argc, char **argv) {
  int err, retval;

  if (argc != 1) {
    fprintf(stderr, "usage: %s\n", argv[0]);
    return 1;
  }

  // setup logging
  err = libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL,
                          LIBUSB_LOG_LEVEL_WARNING);
  if (err < 0) {
    fprintf(stderr, "libusb_set_option(): %s\n", libusb_error_name(err));
    return 1;
  }
  libusb_set_log_cb(NULL, logging_cb, LIBUSB_LOG_CB_GLOBAL);

  // initialize library
  err = libusb_init(NULL);
  if (err < 0) {
    fprintf(stderr, "libusb_init(): %s\n", libusb_error_name(err));
    return 1;
  }

  // get the device
  libusb_device **dev_list;
  ssize_t cnt = libusb_get_device_list(NULL, &dev_list);
  if (cnt < 0) {
    fprintf(stderr, "libusb_get_device_list(): %s\n", libusb_error_name(cnt));
    retval = 1;
    goto end;
  }

  libusb_device *dev = NULL;
  struct libusb_device_descriptor desc;
  for (ssize_t i = 0; i < cnt; i++) {
    err = libusb_get_device_descriptor(dev_list[i], &desc);
    if (err < 0) {
      fprintf(stderr, "libusb_get_device_descriptor(): %s\n",
              libusb_error_name(err));
      retval = 1;
      goto end_dev_list;
    }
    if (desc.idVendor == VID && desc.idProduct == PID) {
      dev = dev_list[i];
      break;
    }
  }
  if (!dev) {
    fprintf(stderr, "error: No device with ID %x:%x found.\n", VID, PID);
    retval = 1;
    goto end_dev_list;
  }

  // open the device
  struct libusb_device_handle *dev_handle;
  err = libusb_open(dev, &dev_handle);
  if (err < 0) {
    fprintf(stderr, "libusb_open(): %s\n", libusb_error_name(err));
    retval = 1;
    goto end_dev_handle;
  }
  libusb_set_auto_detach_kernel_driver(dev_handle, 1);

  // print device descriptor
  struct libusb_device_descriptor dev_desc;
  err = libusb_get_device_descriptor(dev, &dev_desc);
  if (err < 0) {
    fprintf(stderr, "libusb_get_device_descriptor(): %s\n",
            libusb_error_name(err));
    retval = 1;
    goto end_dev_handle;
  }
  err = print_device_descriptor(dev_handle, &dev_desc);
  if (err < 0) {
    retval = 1;
    goto end_dev_handle;
  }

  // print configuration descriptor
  err = libusb_set_configuration(dev_handle, 1);
  if (err < 0) {
    fprintf(stderr, "libusb_set_configuration(): %s\n", libusb_error_name(err));
    retval = 1;
    goto end_dev_handle;
  }
  struct libusb_config_descriptor *config_desc;
  err = libusb_get_active_config_descriptor(dev, &config_desc);
  if (err < 0) {
    fprintf(stderr, "libusb_get_active_config_descriptor(): %s\n",
            libusb_error_name(err));
    retval = 1;
    goto end_dev_handle;
  }
  err = print_configuration_descriptor(dev_handle, config_desc);
  if (err < 0) {
    retval = 1;
    goto end_config_desc;
  }

  // print all interfaces
  for (int i = 0; i < config_desc->bNumInterfaces; i++) {
    for (int j = 0; j < config_desc->interface[i].num_altsetting; j++) {
      const struct libusb_interface_descriptor *int_desc =
          &config_desc->interface[i].altsetting[j];
      err = print_interface_descriptor(dev_handle, int_desc);
      if (err < 0) {
        retval = 1;
        goto end_config_desc;
      }

      // print all endpoints
      for (int k = 0; k < int_desc->bNumEndpoints; k++)
        print_endpoint_descriptor(&int_desc->endpoint[k]);
    }
  }

  // claim interface
  err = libusb_claim_interface(dev_handle, 0);
  if (err < 0) {
    fprintf(stderr, "libusb_claim_interface(): %s\n", libusb_error_name(err));
    retval = 1;
    goto end_config_desc;
  }

  // begin reading data
  while (1) {
    int len;
    err =
        libusb_interrupt_transfer(dev_handle, 0x81, buf, sizeof(buf), &len, 0);
    if (err < 0) {
      fprintf(stderr, "libusb_interrupt_transfer(): %s\n",
              libusb_error_name(err));
      retval = 1;
      goto end_interface;
    }

    if (buf[2] == 0 && buf[3] == 0 && buf[4] == 0 && buf[5] == 0)
      continue;

    // the linux kernel xpad driver code serves ass documentation for these:
    // https://github.com/torvalds/linux/blob/8a696a29c6905594e4abf78eaafcb62165ac61f1/drivers/input/joystick/xpad.c#L810
    if (buf[2] & (1 << 0)) {
      printf("DPAD UP\n");
    }
    if (buf[2] & (1 << 1)) {
      printf("DPAD DOWN\n");
    }
    if (buf[2] & (1 << 2)) {
      printf("DPAD LEFT\n");
    }
    if (buf[2] & (1 << 3)) {
      printf("DPAD RIGHT\n");
    }
    if (buf[2] & (1 << 4)) {
      printf("START\n");
    }
    if (buf[2] & (1 << 5)) {
      printf("BACK\n");
    }
    if (buf[3] & (1 << 0)) {
      printf("LB\n");
    }
    if (buf[3] & (1 << 1)) {
      printf("RB\n");
    }
    if (buf[3] & (1 << 2)) {
      printf("MODE\n");
    }
    if (buf[3] & (1 << 4)) {
      printf("A\n");
    }
    if (buf[3] & (1 << 5)) {
      printf("B\n");
    }
    if (buf[3] & (1 << 6)) {
      printf("X\n");
    }
    if (buf[3] & (1 << 7)) {
      printf("Y\n");
    }
    if (buf[4]) {
      printf("TL %d\n", buf[4]);
    }
    if (buf[5]) {
      printf("TR %d\n", buf[5]);
    }
    // still unsure on how to interpret this data, so don't do anything with
    // it
    /* printf("ABS_LX %d\n", buf[6] | buf[7] << 8); */
    /* printf("ABS_LY %d\n", buf[8] | buf[9] << 8); */
    /* printf("ABS_RX %d\n", buf[10] | buf[11] << 8); */
    /* printf("ABS_RY %d\n", buf[12] | buf[13] << 8); */

    usleep(200000); // wait .2 seconds to reduce jitter
  }

  retval = 0;
end_interface:
  libusb_release_interface(dev_handle, 0);
end_config_desc:
  libusb_free_config_descriptor(config_desc);
end_dev_handle:
  libusb_close(dev_handle);
end_dev_list:
  libusb_free_device_list(dev_list, 1);
end:
  libusb_exit(NULL);
  return retval;
}

