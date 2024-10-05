/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * (c)2015 - 2024 Michael Amadio. Gold Coast QLD, Australia 01micko@gmail.com
 */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <assert.h>

#include <pango/pangocairo.h>

#define PROG "mktranstext"
#define THIS_VERSION "0.1.0"

void usage(){
	printf("%s-%s\n\n", PROG , THIS_VERSION);
	printf("\tGenerate PNG Linux texts.\n\n");
	printf("Usage :\n");
	printf("%s [-L,-l,-S,-s,-T,-t,-n,-x,-y,-d,-z,-e.-p,-A,-B,-C,-J.-k,-c.-h,-v]\n", PROG);
	printf("\t-n [string] image file name\n");
	printf("\t-L [string] label for an image, up to 36 chars, quoted (optional)\n"
			"\talways centred\n");
	printf("\t-l [string] a TTF font family and size in px; quoted\n");
	printf("\t-S [string] second label for an image, up to 36 chars, quoted (optional)\n"
			"\talways centred\n");
	printf("\t-s [string] second TTF font family and size in px; quoted\n");
	printf("\t-T [string] third label for an image, up to 36 chars, quoted (optional)\n"
			"\trequires \"-p x y\" to position the text\n");
	printf("\t-t [string] third TTF font family and size in px; quoted\n"
			"\tThis is good for logo or emoji fonts\n");
	printf("\t-A [int int]; First font weight and style\n");
	printf("\t-B [int int]; Second font weight and style\n");
	printf("\t-C [int int]; Third font weight and style\n");
	printf("\tFont weight and style are governed by Pango enumerations.\n"
			"\tPlease read the manual for further explanation\n");
	printf("\t-z [\"float float float\"] floating point sRGB, quoted,\n"
					"\tspace delimited values for color\n"
					"\teg: -z \"0.1 0.2 0.3\"\n");
	printf("\tOR [\"float float float float\"] with the last arg for transparency\n");
	printf("\t-p [int int] \"x y\" position of the third label\n");
	printf("\t-x [int] width of image in pixels\n");
	printf("\t-y [int] height of image in pixels\n");
	printf("\t-k add embossed effect on font\n");
	printf("\t-e [string] '/path/to/icon.png x y'; embed a png image at \"x y\"\n"
			"\t(optional) always centred\n");
	printf("\t-h : show this help and exit.\n");
	printf("\t-v : show version and exit.\n\n");
	printf("  For more comprehensive information read the man page, \"man %s\"\n", PROG);
	exit (EXIT_SUCCESS);
}

struct { 
	/* allows an icon */
	cairo_surface_t *image;
}glob;

struct text_extents { 
	int f_width;
	int s_width;
	int f_height;
	int s_height;
};

struct colors {
	double red;
	double green;
	double blue;
	double alpha;
	double red1;
	double green1;
	double blue1;
	double red2;
	double green2;
	double blue2;
};

struct font_attrs {
	int wgt;
	int sty;
	char *fam;
	double siz;
};

static const char *get_user_out_file(char *destination) {
	static char out_file[PATH_MAX];
	if (destination != NULL) {
		snprintf(out_file, sizeof(out_file), "%s", destination);
	} else {
		fprintf(stderr, "Failed to recognise directory\n");
		exit (EXIT_FAILURE);
	}
	mkdir(out_file, 0755);
	if (access(out_file, W_OK) == -1) {
		fprintf(stderr, "Failed to access directory %s\n", out_file);
		exit (EXIT_FAILURE);
	}
	return out_file;
}

PangoLayout *hlayout(const char *font, double f_size, cairo_t *c, char *label, int styleness, int boldness) {
	PangoLayout *layout;
	PangoFontDescription *font_description;
	font_description = pango_font_description_new ();
	pango_font_description_set_family (font_description, font);
	pango_font_description_set_style (font_description, styleness );
	pango_font_description_set_weight (font_description, boldness);
	pango_font_description_set_absolute_size (font_description, f_size * PANGO_SCALE);
	layout = pango_cairo_create_layout (c);
	pango_layout_set_font_description (layout, font_description);
	pango_layout_set_width (layout, 1 * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD);
	pango_layout_set_text (layout, label, -1);
	pango_font_description_free (font_description);
	return layout;
}

/**
  * This snippet borrowed from qemu
  * Copyright (C) Fabrice Bellard 2006-2017 GPL-2.0
  * https://github.com/qemu/qemu/blob/master/util/cutils.c
  * Replaces strncpy
  */
void pstrcpy(char *buf, int buf_size, const char *str) {
	int c;
	char *q = buf;
	if (buf_size <= 0) {
		return;
	}
	for(;;) {
		c = *str++;
		if (c == 0 || q >= buf + buf_size - 1)
			break;
		*q++ = c;
	}
	*q = '\0';
}

int split(const char *original, int offset, char **s1, char **s2)
{
	int len;
	len = strlen(original);
	if(offset > len) {
		return(0);
	}
	*s1 = (char *)malloc(sizeof(char) * offset + 1);
	*s2 = (char *)malloc(sizeof(char) * len-offset + 1);
	if((s1 == NULL) || (s2 == NULL)) {
		return(0);
	}
	pstrcpy(*s1, offset + 1, original);
	pstrcpy(*s2, len-offset + 1, original + offset);
	return(1);
}

struct text_extents *get_extents(PangoLayout *flayout, PangoLayout *slayout, int w) {
	struct text_extents *s = malloc(sizeof(struct text_extents));
	assert(s != NULL);
	int wrect, hrect;
	if (flayout == NULL) {
		s->f_width = 0;
		s->f_height = 0;
	} else {
		PangoRectangle rect = { 0 };
		pango_layout_get_extents(flayout, NULL, &rect);
		pango_extents_to_pixels(&rect, NULL);
		wrect = rect.width;
		wrect = ((wrect) < (w)) ? (wrect) : (w);
		hrect = rect.height;
		s->f_width = wrect;
		s->f_height = hrect;
	}
	if (slayout == NULL) {
		s->s_width = 0;
		s->s_height = 0;
	} else {
		PangoRectangle rect = { 0 };
		pango_layout_get_extents(slayout, NULL, &rect);
		pango_extents_to_pixels(&rect, NULL);
		wrect = rect.width;
		wrect = ((wrect) < (w)) ? (wrect) : (w);
		hrect = rect.height;
		s->s_width = wrect;
		s->s_height = hrect;
	}
	return s;	
}

void text_extents_destroy(struct text_extents *s) {
	assert(s != NULL);
	free(s);
}

struct colors get_colors(const char *fp_color) {
	struct colors z;
	double r, g , b, a;
	char red[8];
	char green[8];
	char blue[8];
	char alpha[8] = "1.0"; /* default opaque */
	int len = strlen(fp_color);
	if (len > 32 ) {
		fprintf(stderr,"ERROR: color argument too long\n");
		exit (EXIT_FAILURE);
	}
	int result = sscanf(fp_color, "%s %s %s %s", red, green, blue, alpha);
	if (result < 3) {
		fprintf(stderr,"ERROR: less than 3 color aguments!\n");
		exit (EXIT_FAILURE);
	}
	r = atof(red);
	g = atof(green);
	b = atof(blue);
	a = atof(alpha);
	if ((r > 1.0) || (g > 1.0) || (b > 1.0) || (a > 1.0) ||
	   (r < 0.0) || (g < 0.0) || (b < 0.0) || (a < 0.0))  {
		fprintf(stderr, "Color values are out of range\n");
		exit (EXIT_FAILURE);
	}

	double r1, g1, b1, r2, g2, b2;
	r1 = r; g1 = g; b1 = b;	
	if ((r > 0.701) || (g > 0.701) || (b > 0.701)) {
		r2 = r + 0.3;
		g2 = g + 0.3;
		b2 = b + 0.3;
	} else {
		r2 = r - 0.3;
		g2 = g - 0.3;
		b2 = b - 0.3;		
	}
	z.red = r;
	z.red1 = r1;
	z.red2 = r2;
	z.green = g;
	z.green1 = g1;
	z.green2 = g2;
	z.blue = b;
	z.blue1 = b1;
	z.blue2 = b2;
	z.alpha = a;
	return z;
}

struct font_attrs get_attrs(char *font, const char *label, char *attrs) {
	struct font_attrs f;
	double font_sz;
	char *fontfam;
	char *fontsz;
	char *p = strrchr(font, ' ');
	int roo = strlen(p);
	int wal = strlen(font);
	int jack = wal - roo;
	
	int msg_len = strlen(label);
	if (msg_len > 36) {
		fprintf(stderr,"\"%s\" is too long!\n", label);
		exit (EXIT_FAILURE);
	}
	char weight_pre[5];
	char style_pre[2];
	int weight, style;
	int attr_res = sscanf(attrs, "%s %s", weight_pre, style_pre);
	if (attr_res < 1) {
		fprintf(stderr,"ERROR\n");
		exit (EXIT_FAILURE);
	}
	weight = atoi(weight_pre);
	style = atoi(style_pre);
	f.wgt = weight;
	f.sty = style;
	int kanga = split(font, jack, &fontfam, &fontsz);
	if (kanga != 1) {
		fprintf(stderr, "Unable to parse font string\n");
		exit (EXIT_FAILURE);
	}
	font_sz = atof(fontsz);
	f.fam = fontfam;
	f.siz = font_sz;
	return f;
}

static void paint_img (char *label,
						char *font,
						char *slabel,
						char *sfont,
						char *tlabel,
						char *tfont,
						const char *name,
						int wdth,
						int hght,
						const char *fp_color,
						int kfont,
						char *dest,
						char *eicon,
						char *l_attrs,
						char *s_attrs,
						char *t_attrs,
						char *tpos,
						int join) {
	
	char icon[PATH_MAX];
	char icon_pre[PATH_MAX];
	char posx[8];
	char posy[8];

	char destimg[PATH_MAX];
	double r, g, b, r1, g1, b1, r2, g2, b2, a;
	struct colors col = get_colors(fp_color);
	r = col.red;
	r1 = col.red1;
	r2 = col.red2;
	g = col.green;
	g1 = col.green1;
	g2 = col.green2;
	b = col.blue;
	b1 = col.blue1;
	b2 = col.blue2;
	a = col.alpha;
	
	cairo_surface_t *cs;
	cs = cairo_image_surface_create
							(CAIRO_FORMAT_ARGB32, wdth, hght);
	snprintf(destimg, sizeof(destimg), "%s/%s.png", get_user_out_file(dest), name);
	cairo_t *c;
	c = cairo_create(cs);
	/* background and position */
	cairo_set_source_rgba(c, 0, 0, 0, 0);
	cairo_rectangle(c, 0.0, 0.0, wdth, hght);
	cairo_fill(c);

	/* icon and position */
	if (eicon != NULL) {
		int icon_x, icon_y;
		int icon_res;
		icon_res = sscanf(eicon, "%s 0 0", icon_pre);	
			if (icon_res > 3) {
				fprintf(stderr,"ERROR: too many arguments\n");
				exit (EXIT_FAILURE);
			}
		snprintf(icon, sizeof(icon), "%s", icon_pre);
		if (access(icon, R_OK) == -1) {
			fprintf(stderr, "Failed to access icon %s\n", icon);
			exit (EXIT_FAILURE);
		}
		icon_x = atoi(posx);
		icon_y = atoi(posy);
		glob.image = cairo_image_surface_create_from_png(icon);
		int ww = cairo_image_surface_get_width(glob.image);
		int hh = cairo_image_surface_get_height(glob.image);
		if (!label)  {
			icon_x = (wdth / 2) - (ww / 2);
			icon_y = (hght / 2) - (hh / 2);
		} else {
			icon_x = (wdth / 2) - (ww / 2);
			icon_y = (hght / 2) - hh;			
		}
		cairo_set_source_surface(c, glob.image, icon_x, icon_y);
		cairo_paint(c);
	}
	
	if ((label) && (join == 0)) {
		int weight, style;
		struct font_attrs atr = get_attrs(font, label, l_attrs);
		weight = atr.wgt;
		style = atr.sty;

		/* font color */
		double rf;
		if ((r > 0.5) || (g > 0.5) || (b > 0.5))
			rf = 0.1;
		else
			rf = 0.9;
		
		/* offset font for effect option */	
		double or, og, ob;
		int fc = 0;
		if (kfont == 1) {
			or = (r1 + r2) / 2;
			og = (g1 + g2) /2 ;
			ob = (b1 + b2) / 2;
			fc = 1;
		}
		
		/* font */
		double font_sz;
		char *fontfam;
		struct font_attrs l = get_attrs(font, label, l_attrs);
		fontfam = l.fam;
		font_sz = l.siz;
		
		PangoLayout *layout;
		layout = hlayout(fontfam, font_sz, c, label, style, weight);

		/* position of text */
		int xposi, yposi;
		int wrect, hrect;
		struct text_extents *first_label = get_extents(layout, NULL, wdth);
		wrect = first_label->f_width;
		hrect = first_label->f_height;
		text_extents_destroy(first_label);
		pango_layout_set_width(layout, wrect * PANGO_SCALE);
		if (!eicon) {
			xposi = (wdth / 2) - (wrect / 2);
			yposi = (hght / 2) - (hrect / 2);
		} else {
			xposi = (wdth / 2) - (wrect / 2);
			yposi = (hght / 2) + 10;
		}

		 /* font effect */
		cairo_move_to(c, xposi , yposi);
		cairo_set_source_rgba(c, rf, rf, rf, a);
		pango_cairo_show_layout (c, layout);

		if (fc == 1) {
			double fz = (double)font_sz;
			fz = font_sz / 30; /* 30 default font size*/
			cairo_move_to(c, xposi - (0.75 * fz) , yposi - (0.6 * fz));
			cairo_set_source_rgba(c, or, og, ob, a);
			pango_cairo_show_layout (c, layout);
		}
		g_object_unref (layout);
	}
	if ((slabel) && (join == 0)) {
		int sweight, sstyle;
		struct font_attrs satr = get_attrs(sfont, slabel, s_attrs);
		sweight = satr.wgt;
		sstyle = satr.sty;

		/* font color */
		double rf;
		if ((r > 0.5) || (g > 0.5) || (b > 0.5))
			rf = 0.1;
		else
			rf = 0.9;
		
		/* offset font for effect option */	
		double or, og, ob;
		int fc = 0;
		if (kfont == 1) {
			or = (r1 + r2) / 2;
			og = (g1 + g2) /2 ;
			ob = (b1 + b2) / 2;
			fc = 1;
		}
		
		/* font */
		double sfont_sz;
		char *sfontfam;
		struct font_attrs s = get_attrs(sfont, slabel, s_attrs);
		sfontfam = s.fam;
		sfont_sz = s.siz;
		
		PangoLayout *layout;
		layout = hlayout(sfontfam, sfont_sz, c, slabel, sstyle, sweight);

		/* position of text */
		int xposi, yposi;
		int wrect, hrect;
		struct text_extents *second_label = get_extents(NULL, layout, wdth);
		wrect = second_label->s_width;
		hrect = second_label->s_height;
		text_extents_destroy(second_label);
		pango_layout_set_width(layout, wrect * PANGO_SCALE);
		if (!eicon) {
			xposi = (wdth / 2) - (wrect / 2);
			yposi = (hght / 2) - (hrect / 2) + hrect;
		} else {
			xposi = (wdth / 2) - (wrect / 2);
			yposi = (hght / 2) + hrect;
		}

		 /* font effect */
		cairo_move_to(c, xposi , yposi);
		cairo_set_source_rgba(c, rf, rf, rf, a);
		pango_cairo_show_layout (c, layout);

		if (fc == 1) {
			double fz = (double)sfont_sz;
			fz = sfont_sz / 30; /* 30 default font size*/
			cairo_move_to(c, xposi - (0.75 * fz) , yposi - (0.6 * fz));
			cairo_set_source_rgba(c, or, og, ob, a);
			pango_cairo_show_layout (c, layout);
		}
		g_object_unref (layout);
	}
	if ((slabel) && (label) && (join == 1)) {
		int weight, style;
		struct font_attrs atr = get_attrs(font, label, l_attrs);
		weight = atr.wgt;
		style = atr.sty;

		int sweight, sstyle;
		struct font_attrs satr = get_attrs(sfont, slabel, s_attrs);
		sweight = satr.wgt;
		sstyle = satr.sty;

		/* font color */
		double rf;
		if ((r > 0.5) || (g > 0.5) || (b > 0.5))
			rf = 0.1;
		else
			rf = 0.9;
		
		/* offset font for effect option */	
		double or, og, ob;
		int fc = 0;
		if (kfont == 1) {
			or = (r1 + r2) / 2;
			og = (g1 + g2) /2 ;
			ob = (b1 + b2) / 2;
			fc = 1;
		}
		
		/* font */
		double font_sz;
		char *fontfam;
		struct font_attrs l = get_attrs(font, label, l_attrs);
		fontfam = l.fam;
		font_sz = l.siz;
		
		double sfont_sz;
		char *sfontfam;
		struct font_attrs s = get_attrs(sfont, slabel, s_attrs);
		sfontfam = s.fam;
		sfont_sz = s.siz;
		
		PangoLayout *layout;
		layout = hlayout(fontfam, font_sz, c, label, style, weight);
		PangoLayout *slayout;
		slayout = hlayout(sfontfam, sfont_sz, c, slabel, sstyle, sweight);

		/* position of text */
		int xposi, yposi, sxposi;
		int wrect, hrect, swrect;
		struct text_extents *join_label = get_extents(layout, slayout, wdth);
		wrect = join_label->f_width;
		hrect = join_label->f_height;
		swrect = join_label->s_width;
		text_extents_destroy(join_label);
		pango_layout_set_width(layout, wrect * PANGO_SCALE);
		pango_layout_set_width(slayout, swrect * PANGO_SCALE);
		int tot_wrect;
		tot_wrect = wrect + swrect;
		xposi = (wdth / 2) - (tot_wrect / 2);
		sxposi = ((wdth / 2) - (tot_wrect / 2)) + wrect;
		if (!eicon) {
			yposi = (hght / 2) - (hrect / 2);
		} else {
			yposi = (hght / 2);
		}
		 /* font effect */
		cairo_move_to(c, xposi , yposi);
		cairo_set_source_rgba(c, rf, rf, rf, a);
		pango_cairo_show_layout (c, layout);
		cairo_move_to(c, sxposi , yposi);
		cairo_set_source_rgba(c, rf, rf, rf, a);
		pango_cairo_show_layout (c, slayout);

		if (fc == 1) {
			double fz = (double)sfont_sz;
			fz = sfont_sz / 30; /* 30 default font size*/
			cairo_move_to(c, xposi - (0.75 * fz) , yposi - (0.6 * fz));
			cairo_set_source_rgba(c, or, og, ob, a);
			pango_cairo_show_layout (c, layout);
			cairo_move_to(c, sxposi - (0.75 * fz) , yposi - (0.6 * fz));
			cairo_set_source_rgba(c, or, og, ob, a);
			pango_cairo_show_layout (c, slayout);
		}
		g_object_unref (layout);
		g_object_unref (slayout);
	}
	if (tlabel) {
		int tweight, tstyle;
		struct font_attrs tatr = get_attrs(tfont, tlabel, t_attrs);
		tweight = tatr.wgt;
		tstyle = tatr.sty;

		/* font color */
		double rf;
		if ((r > 0.5) || (g > 0.5) || (b > 0.5))
			rf = 0.1;
		else
			rf = 0.9;
		
		/* offset font for effect option */	
		double or, og, ob;
		int fc = 0;
		if (kfont == 1) {
			or = (r1 + r2) / 2;
			og = (g1 + g2) /2 ;
			ob = (b1 + b2) / 2;
			fc = 1;
		}
		
		/* font */
		double tfont_sz;
		char *tfontfam;
		struct font_attrs t = get_attrs(tfont, tlabel, t_attrs);
		tfontfam = t.fam;
		tfont_sz = t.siz;
		
		PangoLayout *tlayout;
		tlayout = hlayout(tfontfam, tfont_sz, c, tlabel, tstyle, tweight);
		int xposi, yposi;
		char prex[8];
		char prey[8];
		int font_pos = sscanf(tpos, "%s %s", prex, prey);
		if (font_pos < 2) {
			fprintf(stderr,"ERROR: x and y positions are required\n");
			exit (EXIT_FAILURE);
		}
		if (font_pos > 2) {
			fprintf(stderr,"ERROR: too many args\n");
			exit (EXIT_FAILURE);
		}
		xposi = atoi(prex);
		yposi = atoi(prey);
		
		/* font effect */
		cairo_move_to(c, xposi , yposi);
		cairo_set_source_rgba(c, rf, rf, rf, a);
		pango_cairo_show_layout (c, tlayout);

		if (fc == 1) {
			double fz = (double)tfont_sz;
			fz = tfont_sz / 30; /* 30 default font size*/
			cairo_move_to(c, xposi - (0.75 * fz) , yposi - (0.6 * fz));
			cairo_set_source_rgba(c, or, og, ob, a);
			pango_cairo_show_layout (c, tlayout);
		}
		g_object_unref (tlayout);
	}

	cairo_status_t res = cairo_surface_status(cs);
	const char *ret;
	ret = cairo_status_to_string (res);
	if (res != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(cs);
		cairo_destroy(c);
		fprintf(stderr, "Cairo : %s\n", ret);
		exit (EXIT_FAILURE);
	}

	/* cleanup */
	cairo_surface_write_to_png (cs, destimg);

	if (eicon != NULL)
		cairo_surface_destroy(glob.image);
	cairo_surface_destroy(cs);
	cairo_destroy(c);
	fprintf(stdout, "image saved to %s\n", destimg);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage();
		return 0;
	}
	int hflag = 0;
	int vflag = 0;
	char *Lvalue = NULL; /* the main label */
	char *lvalue = "Sans 40"; /* the main label font and size */
	char *Svalue = NULL; /* the second label */
	char *svalue = "Sans 40"; /* the second label font and size */
	char *Tvalue = NULL; /* the third label */
	char *tvalue = "Sans 40"; /* the third label font and size */
	char *nvalue = "foomktt"; /* image name */
	char *zvalue = ".1 .1 .1"; /* fp color */
	char *evalue = NULL; /* embedded icon */
	char *dvalue = getenv("HOME");
	char *Avalue = "400 0"; /* label font weight/style */
	char *Bvalue = "400 0"; /* slabel font weight/style */
	char *Cvalue = "400 0"; /* tlabel font weight/style */
	int width = 200;
	int height = 60;
	int kflag = 0; /* embossed effect */
	int Jflag = 0;
	char *pvalue = "100 100"; /* third label position x y */
	int c;
	while ((c = getopt (argc, argv, "L:l:S:s:T:t:n:x:y:d:z:e:p:A:B:C:Jkhv")) != -1) {
		switch (c)
		{
			case 'L':
				Lvalue = optarg;
				break;
			case 'l':
				lvalue = optarg;
				break;
			case 'S':
				Svalue = optarg;
				break;
			case 's':
				svalue = optarg;
				break;
			case 'T':
				Tvalue = optarg;
				break;
			case 't':
				tvalue = optarg;
				break;
			case 'n':
				nvalue = optarg;
				break;
			case 'A':
				Avalue = optarg;
				break;
			case 'B':
				Bvalue = optarg;
				break;
			case 'C':
				Cvalue = optarg;
				break;
			case 'x':
				width = atoi(optarg);
				break;
			case 'y':
				height = atoi(optarg);
				break;
			case 'z':
				zvalue = optarg;
				break;
			case 'd':
				dvalue = optarg;
				break;
			case 'k':
				kflag = 1;
				break;
			case 'e':
				evalue = optarg;
				break;
			case 'p':
				pvalue = optarg;
				break;
			case 'J':
				Jflag = 1;
				break;
			case 'h':
				hflag = 1;
				if (hflag == 1) usage();
				break;
			case 'v':
				vflag = 1;
				if (vflag == 1) fprintf(stdout, "%s %s - GPL-2.0 Licence\n", PROG, THIS_VERSION);
				return 0;
			default:
				usage();
		}
	}
	paint_img(Lvalue, lvalue, Svalue, svalue, Tvalue, tvalue, nvalue, width,
		height, zvalue, kflag, dvalue, evalue, Avalue, Bvalue, Cvalue, pvalue, Jflag);
	return 0;
}
