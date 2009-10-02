// Filename    : OWEATHER.H
// Description : Header file for class Weather
// Ownership   : Gilbert


#ifndef __OWEATHER_H
#define __OWEATHER_H

#ifndef __ALL_H
#include <ALL.h>
#endif

//--------- Define constant ----------//
#define MAX_WEATHER_FORECAST 3


//--------- Define WeatherType ----------//
typedef enum _WeatherType
{
	WEATHER_SUNNY = 0x00,
	WEATHER_CLOUDY = 0x01,

	WEATHER_RAIN = 0x02,
	WEATHER_LIGHTNING = 0x04,
		WEATHER_LIGHTN_RAIN = 0x06,
	WEATHER_WINDY = 0x08,
		WEATHER_WINDY_STORM = 0x0a,
	WEATHER_HOT_WAVE = 0x10,

	WEATHER_COLD_WAVE = 0x20,
	WEATHER_SNOW = 0x40
} WeatherType;


//--------- Define class Weather ----------//

#pragma pack(1)
class Weather
{
private:
	unsigned seed;
	short	season_phase;			// 0 = early spring, 364 = end of winter
	short day_to_quake;
	short avg_temp;
	short temp_amp;

	short wind_spd;
	// #### begin Gilbert 31/10 #######//
	long	high_wind_day;
	// #### end Gilbert 31/10 #######//
	short	wind_dir;
	short windy_speed;
	short	tornado_count;			// 0=today has tornado, 1... no. of days of last tornado

	char	cur_cloud_str;			// 0 (shine) to 10 (dark)
	char	cur_cloud_len;
	char	cur_cloud_type;		// type of cloud
	int	quake_frequency;

public:
	short	quake_x;					// center of quake, generated on the day of quake
	short	quake_y;

public:
	void	init_date(short year, short month, short day, short latitude, int quakeFreq);
	void	next_day();				// called when a day has passed

	short	cloud();					// return 0 (shine) to 10 (dark)
	short	temp_c();				// temperature in degree C
	short	temp_f();				// temperature in degree F
//	short	humidity();				// relative humidity, 0 to 100
	short	wind_speed();			// wind speed 0 to 100
	short wind_direct();			// 0 to 360
	double wind_direct_rad();	// in radian

	short	rain_scale();			// rain scale, 0 (no rain) to 12 (heavy rain)
	short	snow_scale();			// snow scale, 0 (no snow) to 8 (heavy snow)
	char	is_lightning();
	char	is_quake();
	char	has_tornado();
	short	tornado_x_loc(short maxXLoc, short maxYLoc);
	short	tornado_y_loc(short maxXLoc, short maxYLoc);

	WeatherType desc();
	short quake_rate(short x, short y);		// 0-100

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

private:
	short base_temp();
	unsigned rand_seed(unsigned);
};
#pragma pack()


// ------- define class MagicWeather -----------//

#pragma pack(1)
class MagicWeather
{
private:
	char	rain_str;
	short	wind_spd;
	short	wind_dir;

public:
	short	rain_day;
	short	wind_day;
	short lightning_day;

public:
	void	init();

	void	next_day();
	void	cast_rain(short duration, char rainScale);
	void	cast_wind(short duration, short speed, short direction);
	void	cast_lightning(short duration);

	short	wind_speed();			// wind speed 0 to 100
	short wind_direct();			// 0 to 360
	double wind_direct_rad();	// in radian
	short	rain_scale();			// rain scale, 0 (no rain) to 12 (heavy rain)

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

	friend class Weather;
};
#pragma pack()

extern Weather weather, weather_forecast[MAX_WEATHER_FORECAST];
extern MagicWeather magic_weather;

#endif
