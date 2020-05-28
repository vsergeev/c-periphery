* v2.1.0 - 01/07/2020
    * Add LED module.
    * Add PWM module.
    * Clean up internal string handling in SPI and GPIO modules.

* v2.0.1 - 10/08/2019
    * Initialize handle state in new functions of all modules.
    * Fix performance of blocking read in `serial_read()`.
    * Return error on unexpected empty read in `serial_read()`, which may be
      caused by a serial port disconnect.
    * Improve formatting of `spi_tostring()`.
    * Fix typo in GPIO module documentation.
    * Fix cross-compilation support in Makefile to allow override of CC
      variable.

* v2.0.0 - 09/30/2019
    * Add support for character device GPIOs (`gpio-cdev`) to the GPIO module.
        * Remove support for preserve direction in `gpio_open()`.
        * Remove problematic dummy read with sysfs GPIOs from `gpio_poll()`.
        * Unexport sysfs GPIOs in `gpio_close()`.
    * Migrate to opaque handles with new/free functions in all modules.
    * Simplify error codes for MMIO, I2C, and Serial modules.
    * Fix typos in GPIO module documentation.
    * Update tests with running hints for Raspberry Pi 3.
    * Improve cross-compilation support in Makefile.
    * Contributors
        * longsky, @wangqiang1588 - d880ef7
        * jhlim, @johlim - 742d983

* v1.1.3 - 04/28/2018
    * Fix data's most significant bit getting stripped when opening a serial
      port with parity enabled in `serial_open_advanced()`.
    * Contributors
        * Ryan Barnett, @rjbarnet - 537eeac

* v1.1.2 - 04/01/2018
    * Add handling for delayed pin directory export on some platforms in
      `gpio_open()`.
    * Fix supported functions query for 64-bit in `i2c_open()`.
    * Add support for building with C++.
    * Contributors
        * Jared Bents, @jmbents - 304faf4
        * Ryan Barnett, @rjbarnet - 82ebb4f

* v1.1.1 - 04/25/2017
    * Fix blocking `gpio_poll()` for some platforms.
    * Add library version macros and functions.
    * Contributors
        * Михаил Корнилов, @iTiky - 0643fe9

* v1.1.0 - 09/27/2016
    * Add support for preserving pin direction to `gpio_open()`.
    * Fix enabling input parity check in `serial_set_parity()`.
    * Fix enabling hardware flow control in `serial_set_rtscts()`.
    * Include missing header to fix build with musl libc.
    * Omit unsupported serial baudrates to fix build on SPARC.
    * Contributors
        * Thomas Petazzoni - 27a9552, 114c715

* v1.0.3 - 05/25/2015
    * Fix portability of serial baud rate set/get with termios-provided baud rate functions.
    * Fix unlikely bug in `spi_tostring()` formatting.
    * Clean up integer argument signedness in serial API.

* v1.0.2 - 01/31/2015
    * Fix `gpio_supports_interrupts()` so it does not return an error if interrupts are not supported.
    * Fix `errno` preservation in a few error paths, mostly in the open functions.

* v1.0.1 - 12/26/2014
    * Improve Makefile.
    * Fix _BSD_SOURCE compilation warnings.

* v1.0.0 - 05/15/2014
    * Initial release.
