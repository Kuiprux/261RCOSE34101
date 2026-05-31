#include "color_codes.h"

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

void get_ansi_color(int index, int len, ColorMode mode, char* buffer)
{
	int color_index;
	rgb rgb_color = { 0 };

	switch (mode)
	{
	case COLOR_8:
		sprintf(buffer, "\033[%dm", calc_ratio_int(index, len, sizeof(color8_codes) / sizeof(color8_codes[0])));
		break;
	case COLOR_16:
		sprintf(buffer, "\033[%dm", calc_ratio_int(index, len, sizeof(color16_codes) / sizeof(color16_codes[0])));
		break;
	case COLOR_256:
		color_index = calc_ratio_int(index, len, sizeof(color256_codes) / sizeof(color256_codes[0]));
		sprintf(buffer, "\033[38:5:%dm", color256_codes[color_index]);
		break;
	case COLOR_TRUE:
		rgb_color = calc_ratio_color(index, len, colortrue_codes, sizeof(colortrue_codes) / sizeof(colortrue_codes[0]));
		sprintf(buffer, "\033[38:2:%d:%d:%dm", rgb_color.r, rgb_color.g, rgb_color.b);
		break;
	}

	return buffer;
}