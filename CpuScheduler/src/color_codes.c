#include "color_codes.h"
#include <math.h>
#include <stdio.h>

int color8_codes[] = { 41, 43, 42, 44, 46, 45 };
int color16_codes[] = { 101, 41, 43, 103, 102, 42, 44, 46, 106, 105, 45 };
int color256_codes[] = { 53, 54, 60, 24, 6, 30, 36, 72, 71, 77, 113, 149, 148, 184, 220 };

#include "color_codes_data.inc"

int calc_ratio_int(int index, int len, int max)
{
	return (int)round((double)max * index / len);
}

double interpolate_double(double from, double to, double t)
{
	return (to - from) * t + from;
}

rgb interpolate_color(rgb_double from, rgb_double to, double t)
{
	return (rgb) {
		(int)round(interpolate_double(from.r, to.r, t) * 255),
		(int)round(interpolate_double(from.g, to.g, t) * 255),
		(int)round(interpolate_double(from.b, to.b, t) * 255)
	};
}

rgb calc_ratio_color(int index, int len, rgb_double* color_codes, int color_codes_len)
{
	double color_index = (double)color_codes_len * index / len;
	int color_index_left = (int)color_index;
	double t = color_index - color_index_left;

	return interpolate_color(color_codes[color_index_left], color_codes[color_index_left + 1], t);
}

static int color_array_max_index(int array_len)
{
	return array_len > 0 ? array_len - 1 : 0;
}

void get_ansi_color(int index, int len, ColorMode mode, char* buffer)
{
	int color_index;
	rgb rgb_color = { 0 };

	switch (mode)
	{
	case COLOR_8:
		color_index = calc_ratio_int(index, len, color_array_max_index(sizeof(color8_codes) / sizeof(color8_codes[0])));
		sprintf(buffer, "\033[%dm", color8_codes[color_index]);
		break;
	case COLOR_16:
		color_index = calc_ratio_int(index, len, color_array_max_index(sizeof(color16_codes) / sizeof(color16_codes[0])));
		sprintf(buffer, "\033[%dm", color16_codes[color_index]);
		break;
	case COLOR_256:
		color_index = calc_ratio_int(index, len, color_array_max_index(sizeof(color256_codes) / sizeof(color256_codes[0])));
		sprintf(buffer, "\033[48;5;%dm", color256_codes[color_index]);
		break;
	case COLOR_TRUE:
		rgb_color = calc_ratio_color(index, len, colortrue_codes, sizeof(colortrue_codes) / sizeof(colortrue_codes[0]));
		sprintf(buffer, "\033[48;2;%d;%d;%dm", rgb_color.r, rgb_color.g, rgb_color.b);
		break;
	}
}