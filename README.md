# mktranstext

A CLI utility to generate text with transparent PNG background using
pangocairo.

It supports adding 3 labels with different font families.
You can also add a `png` icon.
Read the manual page for more information.


## build

```
make

# optionally as root
make install # installs to /usr/local/bin by default
```

## colors

As *cairo* is used as the backend for this program, which uses the 
*sRGB* color space (see IEC 61966-2-1:1999), all floating-point
arguments should be expressed on the CLI with a *period* (unicode
char U+002E) as the decimal separator.

## bugs

Report any bugs to the _issues_ section here.

Any features, add them yourself and create a PR.


