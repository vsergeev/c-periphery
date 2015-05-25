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
