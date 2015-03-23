**Imaginable** was born as just yet another image processing toolkit.

Later it became a image processing toolkit specialised on panoramic HDR images.

The main feature is HDR (high dynamic range) images support.
The toolkit is designed to be fast and extensible and it reaches these goals so far.

It consists of the library - the core of the project and the set of front-ends - CLI and GUI applications. Tonemap is the first front-end and it is available now in both - command line and graphical user interfaces. The next front-end will be a panorama merger.

At the moment only Linux is supported, but I am going to port it to Windows soon.

## Features: ##
  * high dynamic range image
  * various colour spaces (RGB/HSV/HSL/HCY/Grayscale/...) including non-standard and custom ones
  * transparency (alpha-channel)
  * QT-based