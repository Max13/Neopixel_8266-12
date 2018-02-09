# Neopixel_8266-12
Control your Neopixels with an ESP8266-12 using an HTTP API

## Pin
The configured pin is D1, you can change it on top of the file. Look at the `#define`'s

## Brightness
The brightness is hardcoded for now, changeable in the beginning of `setup()` by `neopixel.setBrightness()`.

## Routes
* GET `/pixels/{pixels}/status`: Get the status of the first led specified by `{pixels}`. Returns 0 or 1 as `text/plain`.
* GET `/pixels/{pixels}/status/{0,1}`: Set the specified pixels ON or OFF. Actually it doesn't control the power at all. Setting leds "OFF" means all the colors to black, 0, (Red, Green and Blue to 0). Setting them "ON" will set them to the last chosen color. Will return the status, 0 or 1.
* GET `/pixels/{pixels}/color`: Get the color of the first led specified by `{pixels}`. Returns its RGB values as hex as `text/plain`.
* GET `/pixels/{pixels}/color/{color}`: Sets the specified pixels as the desired RGB value, given by `{color}` as hex. Will return the set color.

## Need help?
Open an issue.
