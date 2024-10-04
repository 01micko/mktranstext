mktranstext(1)

# NAME

*mktranstext* - Generate SVG or PNG Linux wallpapers.

# SYNOPSIS

*mktranstext* [options...]

# DESCRIPTION

Mktranstext is a command line utility to generate text on a transparent
background for unix like systems supporting text labels, icons and icon
or emoji fonts.

It uses Pango for font rendering and manipulation and Cairo for
icons. It only uses short options, and there are many of them. It
generates a *png* image in whatever size you specify.

It is great for shell scripting where you can batch produce many images.
It is designed to watermark or brand images where you can use the
generated png image to overlay on any other image or even video files
with the appropriate image or video editor.

# OPTIONS

*-h*

	show help and exit

*-v*

	show version and exit

*-n*

	image file name [string]

*-L*

	label for an image, up to 36 characters, quoted (optional) [string]
	This label will be centred on the image.

*-l*

	a TTF font family for the label and size; quoted [string]

*-S*

	second label for an image, up to 36 chararacters, quoted (optional)
\[string\]
	This label will be centred on the image under the first label unless
	the *-J* (join) option is used. See below..

*-s*

	a TTF font family for the second label and size; quoted [string]

*-T*

	second label for an image, up to 36 chararacters, quoted (optional)
\[string\]
	This label requires the extra *-p* option to position the text in
	the generated image.

*-t*

	a TTF font family for the third label and size; quoted [string]
	This is ideal for logo or emoji fonts.

*-J*

	Joins first and second labels.
	This option takes no argument. You can only use this if you have
	the first and second labels and it joins them on one line. For best
	results the *same* font families and sizes should be used for the
	main label and the second label however the font attributes can be
	different such as *weight* and *style*. See the *-A* and *-B* options
	below.
	
*-z*

	floating point sRGB, three quoted, space delimited values for colour
\["double double double"\]
	
	Transparency is supported with optional fourth arg in quoted string.

*-p*

	the *x* and *y* position of the third label, required for third
label.

*-x*

	width of image in pixels [int]

*-y*

	height of image in pixels [int]

*-k*

	add embossed effect on font [no arg]

*-e*

	'/path/to/icon.png x y' - embed a png image at position *x y*
(optional)

*-d*

	destination directory: (default: $HOME) [/path/to/directory] 

*-A*

	"weight style" for first label [int int](optional)
	Font weight and style attributes for the first label. Usually
	*weight* is 100 to 1000 in multples of 100 with a few exceptions
	outlined below. Lower numbers are lighter. More information on
	weight and style is below.

*-B*

	"weight style" for second label [int int](optional)
	More information on weight and style is below.

*-C*

	"weight style" for third label [int int](optional)
	More information on weight and style is below. Usually not needed
	for icon and emoji fonts.

# COLORS

	As *cairo* is used as the backend for this program, which uses the 
	*sRGB* color space (see IEC 61966-2-1:1999), all floating-point
	arguments should be expressed on the CLI with a *period* (unicode
	char U+002E) as the decimal separator.

# FONT WEIGHT

	Font weight arguments are Pango Enumerations.
	
	Please use the integer, the enum is listed for completeness.
	
	See https://docs.gtk.org/Pango/enum.Weight.html

[[ *Enum*
:- *Integer*
|  PANGO_WEIGHT_THIN
:  100
|  PANGO_WEIGHT_ULTRALIGHT
:  200
|  PANGO_WEIGHT_LIGHT
:  300
|  PANGO_WEIGHT_SEMILIGHT
:  350
|  PANGO_WEIGHT_BOOK
:  380
|  PANGO_WEIGHT_NORMAL
:  400
|  PANGO_WEIGHT_MEDIUM
:  500
|  PANGO_WEIGHT_SEMIBOLD
:  600
|  PANGO_WEIGHT_BOLD
:  700
|  PANGO_WEIGHT_ULTRABOLD
:  800
|  PANGO_WEIGHT_HEAVY
:  900
|  PANGO_WEIGHT_ULTRAHEAVY
:  1000

# FONT STYLE
	
	Font style arguments are also Pango Enumerations.
	
	See https://docs.gtk.org/Pango/enum.Style.html
	
[[ *Enum*
:- *Integer*
|  PANGO_STYLE_NORMAL
:  0
|  PANGO_STYLE_OBLIQUE
:  1
|  PANGO_STYLE_ITALIC
:  2

# SEE ALSO

_mkwallpaper_(1) 

# AUTHOR

	Mick Amadio <01micko@gmail.com>

# LICENCE

	GPLv2

# REPORT BUGS

	https://github.com/01micko/mktranstext/issues
