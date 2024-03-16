# luna-mbx

This library is a reimplementation of a subset of Luna library (the subset used by Magical Broom Extreme) using SDL2 and OpenGL(ES) instead of DirectX 8.1.

----

## Compiling

Use following definitions to alter compilation:

* USE_GLES
  * use OpenGL ES 1 instead of OpenGL
* USE_SPEEXDSP_RESAMPLER
  * use [speexdsp library](https://github.com/xiph/speexdsp/ "speexdsp library") for resampling audio
* PANDORA
  * use code specific for Pandora console
* PYRA
  * use code specific for DragonBox Pyra console

----

## License

The source code is released with MIT license.
