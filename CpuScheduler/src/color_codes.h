#pragma once

#define ANSI_DEFAULT "\033[0m"
#define ANSI_BLACK "\033[40m"

typedef enum {
	COLOR_8, COLOR_16, COLOR_256, COLOR_TRUE
} ColorMode;

typedef struct {
	double r;
	double g;
	double b;
} rgb_double;

typedef struct {
	int r;
	int g;
	int b;
} rgb;

extern int color8_codes[];
extern int color16_codes[];
extern int color256_codes[];
extern rgb_double colortrue_codes[];

int calc_ratio_int(int index, int len, int max);
double interpolate_double(double from, double to, double t);

rgb interpolate_color(rgb_double from, rgb_double to, double t);
rgb calc_ratio_color(int index, int len, rgb_double* color_codes, int color_codes_len);
void get_ansi_color(int index, int len, ColorMode mode, char* buffer);
