/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Filename    : OWEATHER.CPP
// Description : class Weather
// Ownership   : Gilbert


#include <OWEATHER.h>
#include <ALL.h>
#include <math.h>
#include <OWORLDMT.h>

//---------- Define constant -----------//
#define RAIN_CLOUD 1
#define LIGHTNING_CLOUD 2
#define WINDY 4
#define HOT_WAVE 8
#define COLD_WAVE 0x10
#define M_PI 3.14159265359


//-------- Begin of function Lightning::rand_seed ----------//
unsigned Weather::rand_seed(unsigned MAX)
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed % MAX;
}
//-------- End of function Lightning::rand_seed ----------//

//---------- Begin of function Weather::init_date ----------//
//
void Weather::init_date(short year, short month, short day, short latitude, int quakeFreq)
{
	// ----------- initialize random seed
	seed = 2*year+1;
	(void) rand_seed(10);

	// ----------- calculate season_phase from month, day ------------//
	season_phase = (short) ( month * 30.4 + day);
	season_phase = (season_phase + 365 - 98) % 365;	// 7th Mar becomes 0

	// ----------- random number to earthquake -----------//
	quake_frequency = quakeFreq;
	day_to_quake = quakeFreq + rand_seed(quakeFreq);

	// ----------- determine avg_temp and temp_amp from latitude
	double angle = latitude * M_PI / 180.0;
	avg_temp = (short)( 35.0 - fabs(latitude / 90.0 * 40.0));
	temp_amp = (short)( 17.0 * sin(angle));	// negative for South Hemisphere

	// ----------- determine cloud ----------- //
	cur_cloud_str = rand_seed(4);
	cur_cloud_len = 5 + rand_seed(5);
	cur_cloud_type = 0;

	// ----------- determine wind ---------------//
	wind_dir = rand_seed(360);
	wind_spd = 10;
	high_wind_day = 0;
	windy_speed = 0;

	tornado_count = 1;

}
//---------- End of function Weather::init_date ----------//

//---------- Begin of function Weather::next_day ----------//
//
void Weather::next_day()
{

	season_phase = (season_phase + 1 )% 365;

	//---------- update/determine earthquake day ---------//
	if( day_to_quake)
	{
		day_to_quake--;
		if ( is_quake() )
		{
			// generate quake_x, quake_y
			quake_x = rand_seed(0x10000) * MAX_MAP_WIDTH / 0x10000;
			quake_y = rand_seed(0x10000) * MAX_MAP_HEIGHT / 0x10000;
		}
	}
	else
		day_to_quake = quake_frequency + rand_seed(quake_frequency);

	//---------- update wind ----------//
	wind_dir = (wind_dir + rand_seed(5) ) % 360;
	wind_spd += rand_seed(9) - 4 - (high_wind_day/16);
	if( wind_spd < -10)
		wind_spd = -10;
	if( wind_spd > 110)
		wind_spd = 110;
	if( wind_spd >= 20 )
	{
		high_wind_day++;
	}
	else 
	{
		high_wind_day--;
	}

	//---------- generate cloud --------//
	if( cur_cloud_len > 0)
	{
		cur_cloud_len--;
	}
	else
	{
		short t = base_temp();
		short maxCloudStr;
		if( t >= 30)
			maxCloudStr = 10;
		else if( t <= 18)
			maxCloudStr = 4;
		else
			maxCloudStr = (t -18)/2 + 4;
		cur_cloud_str = rand_seed(maxCloudStr+4)-3;	// range : -2 to maxCloudStr
		if(cur_cloud_str < 0)
			cur_cloud_str = 0;
		cur_cloud_len = 2 + rand_seed(3) + rand_seed(3);

		cur_cloud_type = 0;
		
		// ------- summer weather
		if( cur_cloud_str > 4)
		{
			if( (char) rand_seed(10) < cur_cloud_str )
				cur_cloud_type |= RAIN_CLOUD;
			if( cur_cloud_str >= 6 && (char) rand_seed(10) < cur_cloud_str-4)
				cur_cloud_type |= WINDY;
		}
		if( cur_cloud_str <= 1 && t >= 30 && rand_seed(10) <= 1)
		{
			cur_cloud_type |= HOT_WAVE;
		}

		// ------- winter weather
		if( t < 15)
		{
			if( rand_seed(20) < 2 )
				cur_cloud_type |= COLD_WAVE;

			if( t >= 10 && rand_seed(10) < 3)
				cur_cloud_type |= WINDY;
			if( t < 10 && rand_seed(10) < 7)
				cur_cloud_type |= WINDY;
		}
		if( cur_cloud_type & WINDY)
			windy_speed = 10 + cur_cloud_str * 5 + rand_seed(2*cur_cloud_str+1);
		else
		{
			windy_speed = 0;
			if( cur_cloud_str > 4 && (char)rand_seed(50) < cur_cloud_str + 2 )
				cur_cloud_type |= LIGHTNING_CLOUD;
		}

		// ---- double the time of snow ------ //
		if( snow_scale() )
			cur_cloud_len += cur_cloud_len;

	}

	// -------- update tornado_count, at least 20 days between two tornadoes -------//
	if( tornado_count > 20 && base_temp() >= 30 && wind_speed() >= 40 
		&& rand_seed(10)==0 )
	{
		tornado_count = 0;			// today has a tornado
	}
	else
	{
		tornado_count++;
	}
}
//---------- End of function Weather::next_day ----------//


//---------- Begin of function Weather::base_temp ----------//
//
short Weather::base_temp()
{
	return( (short)(avg_temp + temp_amp * sin(season_phase / 365.0
		* 2 * M_PI) ));
}
//---------- End of function Weather::base_temp ----------//

//---------- Begin of function Weather::cloud ----------//
//
short Weather::cloud()
{
	if( cur_cloud_str < 0)
		return 0;
	if( cur_cloud_str > 10)
		return 10;
	return cur_cloud_str;
}
//---------- End of function Weather::cloud ----------//

//---------- Begin of function Weather::temp_c ----------//
//
short Weather::temp_c()
{
	return base_temp() - (cur_cloud_str < 1 ? 0 : (cur_cloud_str < 4 ? 2:4)) +
		(cur_cloud_type & HOT_WAVE ? 8:0) - (cur_cloud_type & COLD_WAVE ? 10:0);
}
//---------- End of function Weather::temp_c ----------//

//---------- Begin of function Weather::temp_f ----------//
//
short Weather::temp_f()
{
	return ((short) (base_temp() / 5.0 * 9.0 + 32.0 ));
}
//---------- End of function Weather::temp_f ----------//

//---------- Begin of function Weather::wind_speed ----------//
//
short Weather::wind_speed()
{
	if( this == &weather && magic_weather.wind_day > 0)
		return magic_weather.wind_speed();
	short w = wind_spd + windy_speed;
	if(w < 0)
		return 0;
	if(w > 100)
		return 100;
	return w;
}
//---------- End of function Weather::wind_speed ----------//

//---------- Begin of function Weather::wind_direct ----------//
//
short Weather::wind_direct()
{
	if( this == &weather && magic_weather.wind_day > 0)
		return magic_weather.wind_direct();
	return wind_dir;
}
//---------- End of function Weather::wind_direct ----------//

//---------- Begin of function Weather::wind_direct_rad ----------//
//
double Weather::wind_direct_rad()
{
	if( this == &weather && magic_weather.wind_day > 0)
		return magic_weather.wind_direct_rad();
	return wind_dir * M_PI / 180.0;
}
//---------- End of function Weather::wind_direct ----------//


//---------- Begin of function Weather::rain_scale ----------//
//
short Weather::rain_scale()
{
	if( this == &weather && magic_weather.rain_day > 0)
		return magic_weather.rain_scale();
	return cur_cloud_str > 4 ? cur_cloud_str * 2 -8 : 0;
}
//---------- End of function Weather::rain_scale ----------//

//---------- Begin of function Weather::snow_scale ----------//
//
short Weather::snow_scale()
{
	short t = temp_c();
	if( t > 0)
		return 0;

	if( t <= -15)
	{
		if( t <= -30)
			return 8;
		if( t <= -25)
			return 7;
		if( t <= -20)
			return 6;
		return 5;
	}
	else
	{
		if(t <= -10)
			return 4;
		if( t <= -5)
			return 3;
		if( t <= -2)
			return 2;
		return 1;
	}
}
//---------- End of function Weather::snow_scale ----------//

//---------- Begin of function Weather::is_lightning ----------//
//
char Weather::is_lightning()
{
	if( magic_weather.lightning_day > 0 )
		return LIGHTNING_CLOUD;
	return( cur_cloud_type & LIGHTNING_CLOUD );
}
//---------- End of function Weather::is_lightning ----------//

//---------- Begin of function Weather::is_quake ----------//
//
char Weather::is_quake()
{
	return( day_to_quake == 0);
}
//---------- End of function Weather::is_quake ----------//

//---------- Begin of function Weather::desc ---------//
//
WeatherType Weather::desc()
{
	int w = WEATHER_SUNNY;
	if( rain_scale() > 0 )
		w |= WEATHER_RAIN;
	if( is_lightning() )
		w |= WEATHER_LIGHTNING;
	if( snow_scale() > 0 )
		w |= WEATHER_SNOW;
	else if( cur_cloud_type & COLD_WAVE)
		w |= WEATHER_COLD_WAVE;
	if( cur_cloud_type & HOT_WAVE)
		w |= WEATHER_HOT_WAVE;
	if( cur_cloud_type & WINDY)
		w |= WEATHER_WINDY;

	if( w == WEATHER_SUNNY && cloud() >= 4)
		w |= WEATHER_CLOUDY;

	return (WeatherType)w;
}
//---------- End of function Weather::desc ---------//


//---------- Begin of function Weather::has_tornado -------//
char Weather::has_tornado()
{
	return tornado_count == 0;
}
//---------- End of function Weather::has_tornado -------//

//---------- Begin of function Weather::tornado_x_loc -------//
//
// return where a new tornado should create
//
short	Weather::tornado_x_loc(short maxXLoc, short)
{
	short dir = (wind_direct() + 180) % 360;
	err_when(dir < 0 || dir > 360);

	if( dir < 45)
	{
		// north side
		return maxXLoc*(dir+45)/90;
	}
	else if( dir < 135)
	{
		// east side
		return maxXLoc-1;
	}
	else if( dir < 225)
	{
		// south side
		return  maxXLoc*(224-dir)/90;
	}
	else if( dir < 315)
	{
		// west side
		return 0;
	}
	else
	{
		// north side
		return maxXLoc*(dir-315)/90;
	}
}
//---------- End of function Weather::tornado_x_loc -------//

//---------- Begin of function Weather::tornado_y_loc -------//
short	Weather::tornado_y_loc(short , short maxYLoc)
{
	short dir = (wind_direct() + 180) % 360;
	err_when(dir < 0 || dir > 360);

	if( dir < 45)
	{
		// north side
		return 0;
	}
	else if( dir < 135)
	{
		// east side
		return maxYLoc*(dir-45)/90;
	}
	else if( dir < 225)
	{
		// south side
		return maxYLoc -1;
	}
	else if( dir < 315)
	{
		// west side
		return maxYLoc*(314-dir)/90;
	}
	else
	{
		// north side
		return 0;
	}
}
//---------- End of function Weather::tornado_y_loc -------//


//---------- Begin of function Weather::quake_rate -------//
short Weather::quake_rate(short x, short y)
{
	err_when( !is_quake() );

	short dist = MAX( abs(x - quake_x), abs(y - quake_y) );
	short damage = 100 - dist / 2;
	return damage > 0 ? damage : 0;
}
//---------- End of function Weather::quake_rate -------//


//--------- Begin of function MagicWeather::init ----------//
void MagicWeather::init()
{
	rain_day = 0;
	wind_day = 0;
}
//--------- End function MagicWeather::init ----------//


//--------- Begin of function MagicWeather::next_day ----------//
void MagicWeather::next_day()
{
	if( rain_day > 0 )
		--rain_day;
	if( wind_day > 0 )
		--wind_day;
	if( lightning_day > 0 )
		--lightning_day;
}
//--------- End function MagicWeather::next_day ----------//


//--------- Begin of function MagicWeather::cast_rain ----------//
void MagicWeather::cast_rain(short duration, char rainScale)
{
	// override last cast_rain
	rain_day = duration;
	rain_str = rainScale;
}
//--------- End of function MagicWeather::cast_rain ----------//


//--------- Begin of function MagicWeather::cast_wind ----------//
void MagicWeather::cast_wind(short duration, short speed, short direction)
{
	// override last cast_wind
	wind_day = duration;
	wind_spd = speed;
	wind_dir = direction;
}
//--------- End of function MagicWeather::cast_wind ----------//


//--------- Begin of function MagicWeather::cast_lightning ----------//
void MagicWeather::cast_lightning(short duration)
{
	// override last cast_lightning
	lightning_day = duration;
}
//--------- End of function MagicWeather::cast_lightning ----------//


//--------- Begin of function MagicWeather::wind_speed ----------//
short MagicWeather::wind_speed()
{
	return wind_spd;
}
//--------- End of function MagicWeather::wind_speed ----------//


//--------- Begin of function MagicWeather::wind_direct ----------//
short MagicWeather::wind_direct()
{
	return wind_dir;
}
//--------- End of function MagicWeather::wind_direct ----------//


//--------- Begin of function MagicWeather::wind_direct_rad ----------//
double MagicWeather::wind_direct_rad()
{
	return wind_dir * M_PI / 180.0;
}
//--------- End of function MagicWeather::wind_direct_rad ----------//


//--------- Begin of function MagicWeather::rain_scale ----------//
short MagicWeather::rain_scale()
{
	return rain_str;
}
//--------- End of function MagicWeather::rain_scale ----------//
