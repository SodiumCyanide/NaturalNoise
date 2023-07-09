#ifndef SCLIB_PERLINNOISE_H 
#define SCLIB_PERLINNOISE_H

#include <cmath>

#include <algorithm>
#include <chrono>
#include <functional>
#include <numbers>
#include <random>
#include <utility>

namespace SCLib
{
	struct ColorRGB
	{
		double M_red, M_green, M_blue;
	};

	struct ColorHSV
	{
		double M_hue, M_saturation, M_brightness;
	};

	ColorHSV RGB_to_HSV(ColorRGB color) noexcept
	{
		static double max_weight, min_weight;
		max_weight = std::max(color.M_red, std::max(color.M_green, color.M_blue));
		min_weight = std::min(color.M_red, std::min(color.M_green, color.M_blue));

		static ColorHSV converted;

		if(max_weight == min_weight)
			converted.M_hue = 0.0;
		else if(max_weight == color.M_red)
		{
			if(color.M_green > color.M_blue)
				converted.M_hue = std::fma(1.0 / 6.0, (color.M_green - color.M_blue) / (max_weight - min_weight), 0.0);
			else
				converted.M_hue = std::fma(1.0 / 6.0, (color.M_green - color.M_blue) / (max_weight - min_weight), 1.0);
		}
		else if(max_weight == color.M_green)
			converted.M_hue = std::fma(1.0 / 6.0, (color.M_blue - color.M_red) / (max_weight - min_weight), 1.0 / 3.0);
		else if(max_weight == color.M_blue)
			converted.M_hue = std::fma(1.0 / 6.0, (color.M_red - color.M_green) / (max_weight - min_weight), 2.0 / 3.0);
		else
			converted.M_hue = 0.0;	// Exception.

		if(max_weight == 0.0)
			converted.M_saturation = 0.0;
		else
			converted.M_saturation = 1.0 - min_weight / max_weight;

		converted.M_brightness = max_weight;

		return converted;
	}

	ColorRGB HSV_to_RGB(ColorHSV color) noexcept
	{
		static double hue, f, p, q, t;
		static int selector;

		selector = static_cast<int>(hue = std::floor(color.M_hue * 6.0));
		f = color.M_hue * 6.0 - hue;
		p = color.M_brightness * (1.0 - color.M_saturation);
		q = color.M_brightness * (1.0 - f * color.M_saturation);
		t = color.M_brightness * (1.0 - (1.0 - f) * color.M_saturation);

		static ColorRGB converted;

		switch(selector)
		{
		case 0:
		case 6:
			converted.M_red = color.M_brightness;
			converted.M_green = t;
			converted.M_blue = p;
			break;
		case 1:
			converted.M_red = q;
			converted.M_green = color.M_brightness;
			converted.M_blue = p;
			break;
		case 2:
			converted.M_red = p;
			converted.M_green = color.M_brightness;
			converted.M_blue = t;
			break;
		case 3:
			converted.M_red = p;
			converted.M_green = q;
			converted.M_blue = color.M_brightness;
			break;
		case 4:
			converted.M_red = t;
			converted.M_green = p;
			converted.M_blue = color.M_brightness;
			break;
		case 5:
			converted.M_red = color.M_brightness;
			converted.M_green = p;
			converted.M_blue = q;
			break;
		default:
			converted.M_red = converted.M_green = converted.M_blue = 0.0;	// Exception.
			break;
		}

		return converted;
	}
}

namespace SCLib
{
	template<std::size_t T_level = 0>
	constexpr double smoothstep(double x)
	{
		if constexpr(T_level == 0)
			return x;
		else if constexpr(T_level == 1)
			return x * x * (-2.0 * x + 3.0);
		else if constexpr(T_level == 2)
			return x * x * x * ((6.0 * x - 15.0) * x + 10.0);
		else if constexpr(T_level == 3)
			return x * x * x * x * (((-20.0 * x + 70.0) * x - 84.0) * x + 35.0);
		else
			return x == 0.5 ? 0.5 : x < 0.5 ? 0.0 : 1.0;	// Exception.
	}
}

namespace SCLib
{
	std::mt19937_64 PerlinRandomGenerator(static_cast<std::mt19937_64::result_type>(std::chrono::system_clock::now().time_since_epoch().count()));

	std::uniform_real_distribution<double> PerlinPIUniformRealDistribution(-std::numbers::pi, std::numbers::pi);
	std::uniform_real_distribution<double> PerlinUnitUniformRealDistribution;
	
	auto AngleRandGen = std::bind(PerlinPIUniformRealDistribution, PerlinRandomGenerator);
	auto UnitRandGen = std::bind(PerlinUnitUniformRealDistribution, PerlinRandomGenerator);

	class PerlinNoise
	{
	public:
		PerlinNoise();
		PerlinNoise(std::size_t, std::size_t);
		PerlinNoise(const PerlinNoise&);
		PerlinNoise(PerlinNoise&&) noexcept;
		~PerlinNoise();

		PerlinNoise& operator = (const PerlinNoise&);
		PerlinNoise& operator = (PerlinNoise&&) noexcept;
		double& operator () (std::size_t, std::size_t);

		bool generate(std::size_t, std::size_t = 0);
	private:
		std::size_t M_length, M_width;
		double* M_field;
	};

	PerlinNoise::PerlinNoise() : M_length(0), M_width(0), M_field(nullptr)
	{
	}

	PerlinNoise::PerlinNoise(std::size_t length, std::size_t width) : M_length(length), M_width(width), M_field(nullptr)
	{
		M_field = new double[M_length * M_width]{};
	}

	PerlinNoise::PerlinNoise(const PerlinNoise& source) : M_length(source.M_length), M_width(source.M_width), M_field(nullptr)
	{
		if(source.M_field)
		{
			M_field = new double[M_length * M_width]{};
			std::copy_n(source.M_field, M_length * M_width, M_field);
		}
	}

	PerlinNoise::PerlinNoise(PerlinNoise&& source) noexcept : M_length(std::exchange(source.M_length, 0)), M_width(std::exchange(source.M_width, 0)), M_field(std::exchange(source.M_field, nullptr))
	{
	}

	PerlinNoise::~PerlinNoise()
	{
		delete[] M_field;
		M_field = nullptr;
		M_length = M_width = 0;
	}

	PerlinNoise& PerlinNoise::operator = (const PerlinNoise& source)
	{
		if(this != &source)
		{
			delete[] M_field;
			M_field = nullptr;
			M_length = source.M_length;
			M_width = source.M_width;
			if(source.M_field)
			{
				M_field = new double[M_length * M_width]{};
				std::copy_n(source.M_field, M_length * M_width, M_field);
			}
		}
		return *this;
	}

	PerlinNoise& PerlinNoise::operator = (PerlinNoise&& source) noexcept
	{
		if(this != &source)
		{
			delete[] M_field;
			M_field = nullptr;
			M_length = std::exchange(source.M_length, 0);
			M_width = std::exchange(source.M_width, 0);
			M_field = std::exchange(source.M_field, nullptr);
		}
		return *this;
	}

	double& PerlinNoise::operator () (std::size_t i, std::size_t j)
	{
		return M_field[i * M_width + j];
	}

	bool PerlinNoise::generate(std::size_t grid_size, std::size_t seed)
	{
		if(M_field && grid_size >= 10)	// Cannot plot on too small grids.
		{
			std::size_t length_grids = static_cast<std::size_t>(std::ceil(static_cast<double>(M_length) / static_cast<double>(grid_size)) + 1.0);
			std::size_t width_grids = static_cast<std::size_t>(std::ceil(static_cast<double>(M_width) / static_cast<double>(grid_size)) + 1.0);

			if(length_grids >= 2 && width_grids >= 2)
			{
				if(seed)
					PerlinRandomGenerator.seed(seed);

				double* grid_unit_x = new double[length_grids * width_grids]{};
				double* grid_unit_y = new double[length_grids * width_grids]{};

				double temp_angle{};

				for(std::size_t i = 0; i < length_grids; ++i)
					for(std::size_t j = 0; j < width_grids; ++j)
					{
						temp_angle = AngleRandGen();
						grid_unit_x[i * width_grids + j] = std::sin(temp_angle);
						grid_unit_y[i * width_grids + j] = std::cos(temp_angle);
					}

				std::size_t grid_x{};
				std::size_t grid_y{};
				double offset_x{};
				double offset_y{};

				for(std::size_t i = 0; i < M_length; ++i)
				{
					grid_x = i / grid_size;
					offset_x = (0.5 + static_cast<double>(i % grid_size)) / static_cast<double>(grid_size);
					for(std::size_t j = 0; j < M_width; ++j)
					{
						grid_y = j / grid_size;
						offset_y = (0.5 + static_cast<double>(j % grid_size)) / static_cast<double>(grid_size);

						M_field[i * M_width + j] = std::lerp(
							std::lerp(
								grid_unit_x[grid_x * width_grids + grid_y] * offset_x + grid_unit_y[grid_x * width_grids + grid_y] * offset_y,
								grid_unit_x[(grid_x + 1) * width_grids + grid_y] * (offset_x - 1.0) + grid_unit_y[(grid_x + 1) * width_grids + grid_y] * offset_y,
								smoothstep<2>(offset_x)
							),
							std::lerp(
								grid_unit_x[grid_x * width_grids + (grid_y + 1)] * offset_x + grid_unit_y[grid_x * width_grids + (grid_y + 1)] * (offset_y - 1.0),
								grid_unit_x[(grid_x + 1) * width_grids + (grid_y + 1)] * (offset_x - 1.0) + grid_unit_y[(grid_x + 1) * width_grids + (grid_y + 1)] * (offset_y - 1.0),
								smoothstep<2>(offset_x)
							),
							smoothstep<2>(offset_y)
						);
					}
				}

				if(seed)
					PerlinRandomGenerator.seed(static_cast<std::mt19937_64::result_type>(std::chrono::system_clock::now().time_since_epoch().count()));

				delete[] grid_unit_x;
				delete[] grid_unit_y;

				return true;
			}
			else
				return false;	
		}
		else
			return false;
	}
}

#endif // !SCLIB_PERLINNOISE_H
