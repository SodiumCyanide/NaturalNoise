#include <iostream>

#include "BasicBitmap.h"
#include "PerlinNoise.h"
int main()
{
	std::size_t length = 3840;
	std::size_t width = 2160;

	double sector = 0.25;

	BasicBitmap FileHandle(length, width);
	SCLib::PerlinNoise BaseNoise(length, width);
	SCLib::PerlinNoise FractalNoise[5]
	{
		{length, width},
		{length, width},
		{length, width},
		{length, width},
		{length, width},
	};

	FractalNoise[0].generate(400);
	FractalNoise[1].generate(200);
	FractalNoise[2].generate(100);
	FractalNoise[3].generate(50);
	FractalNoise[4].generate(25);

	for(int i = 0; i < length; ++i)
		for(int j = 0; j < width; ++j)
			BaseNoise(i, j) = FractalNoise[0](i, j) * 0.30
			+ FractalNoise[1](i, j) * 0.25
			+ FractalNoise[2](i, j) * 0.20
			+ FractalNoise[3](i, j) * 0.15
			+ FractalNoise[4](i, j) * 0.10;

	SCLib::ColorHSV temp_hsv{};
	SCLib::ColorRGB temp_rgb{};
	IUINT32 raw_color{};

	for(int i = 0; i < length; ++i)
		for(int j = 0; j < width; ++j)
		{
			temp_hsv = {0.0, 0.0, std::clamp((BaseNoise(i, j) + sector) / (2.0 * sector), 0.0, 1.0)};

			temp_rgb = SCLib::HSV_to_RGB(temp_hsv);
			raw_color = (static_cast<IUINT32>(0xffp0 * temp_rgb.M_red) << 16) + (static_cast<IUINT32>(0xffp0 * temp_rgb.M_green) << 8) + (static_cast<IUINT32>(0xffp0 * temp_rgb.M_blue));
			FileHandle.SetColor(i, j, raw_color);
		}

	FileHandle.SaveBmp("Example.bmp");
}