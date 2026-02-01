// Rcalc.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "Rcalc.hpp"

#define APPLICATION_NAME "Rcalc"
#define VERSION "0.00.01"
#define SUBVERSION "1"
#define CONFIG_VERSION 1

#ifdef _WIN32
void CL_cleanup(void);
#else 
long file_length(char *f);
#endif

char ConfigFile[]="rcalc.cfg";
Rcalc Rc;

int main(int argc, char* argv[])
{	int rc;
#ifdef _WIN32
	rc = SetConsoleCP(1251);
	rc = SetConsoleOutputCP(1251);
	atexit(CL_cleanup);
#endif
	rc = Rc.init(ConfigFile);  // задаем начальные условия
	Rc.read_csv(Rc.dataFile);
	return 0;
}

#ifdef _WIN32

void CL_cleanup(void)
{   int rc;
    rc = SetConsoleCP(866);
	rc = SetConsoleOutputCP(866);	
}
#endif

#define CFG_AddV(form, name, type) cfg.AddV(#name, form, &name, type)
#define CFG_AddS(form, name, _nb)  cfg.AddStr(#name, form, &name, _nb)

static int configNparW=0,  configNparR=0, configNparNeed=0;

int Rcalc::init(char *ConfigFile)
{	int rc, need;
//	rc = cfg.Add(NULL,";%s:", Appname, 1);
	configNparW = 0;
	cfg.AddC(";%s v %s %s\n", (char *)APPLICATION_NAME, (char *)VERSION, (char *)SUBVERSION, NULL);
	cfg.AddC(";configversion\n",  NULL);
	if(configversion == -1)
	   configversion = CONFIG_VERSION;

	CFG_AddV("%d\n",configversion, _INT); configNparW++;
	cfg.AddC(";************ Set parameters here ****************\n",  NULL);
   cfg.AddC(";CSV data file from save data at indoor/outdoor temperatures graph in Home Assistant, \n", NULL);
   cfg.AddC(";CSV файл, получаемый из графика температур в доме и на улице data в Home Assistant через \"скачать данные\",\n", NULL);
	CFG_AddS("%s\n", dataFile, sizeof(dataFile)); configNparW++;
   cfg.AddC(";use gas (1) or electricity (0)\n", NULL);
   cfg.AddC(";отопление газом (1) или электричеством (0)\n", NULL);
	CFG_AddV("%d\n",use_gas, _INT); configNparW++;

   cfg.AddC(";Specific heat of combustion natural/liquefied gas, J/m3 33.5/45.2 *10^6\n", NULL);
   cfg.AddC(";Удельная теплота сгорания природного/сжиженного газа, Дж.м3 33.5/45.2 *10^6 \n", NULL);
   CFG_AddV("%f\n", GasSHC, _DOUBLE);  configNparW++;
   cfg.AddC(";Gas consumption, m3 | объем потребления газа, m3\n", NULL);
   CFG_AddV("%f\n", Gas_usedV, _DOUBLE);  configNparW++;
   cfg.AddC(";electricity consumption, kWt*h | объем потребления электроэнергии, кВт*час\n", NULL);
   CFG_AddV("%f\n", Electricity_used, _DOUBLE);  configNparW++;

   cfg.AddC(";************ House parameters ****************\n",  NULL);
   cfg.AddC(";house area, m2 | площадь дома, м2\n",  NULL);
   CFG_AddV("%lf\n", HomeS, _DOUBLE);  configNparW++;
   cfg.AddC(";area of the house enclosing structure, m2\n",  NULL);
   cfg.AddC(";площадь ограждающих конструкций дома, (стены+пол+потолок) м2\n",  NULL);
   CFG_AddV("%lf\n", HomeCS, _DOUBLE);  configNparW++;

   cfg.AddC(";*********************************\n",  NULL);
   cfg.AddC(";Last parameter in config: number of parameters. Don't change with hands\n",  NULL);
   CFG_AddV("%d\n", configNparW, _INT);  configNparW++;
   configNparNeed = configNparW;

    need = 0;
	rc  = cfg.Read(ConfigFile);
	if(rc)
	{  if(rc == 1)
				printf("Config %s not found\n", ConfigFile);
		need = 1;
	} else {
		if(configversion == -1 || configversion != CONFIG_VERSION)
		{	configversion = CONFIG_VERSION;
			printf("Old version of config file\n");
			need = 1;
		} else 
			if(cfg.configNparR != configNparW || cfg.configNparR != configNparNeed)
		{
			printf("Numer of parameters in config file changed\n");
			need = 1;
			configNparW = configNparNeed;
		}
	}

	if(need)
	{	printf("Default config file %s is written\nPlease restart programm\n",ConfigFile);
		rc = cfg.Write(ConfigFile);
		exit(0);
	}
	return 0;
}

int parse_dt(char *date_str, time_t &t);

int Rcalc::read_csv(char *CSV_File)
{	FILE * fp;
	char str[256], *pstr;
	char str0[256], str1[256];
	char searchstr[]="indoor";
	char searchstr1[]="outdoor";
	char *pseachstr[2];
	int i, j, is, l, lfs, rc;
	time_t tt, tt0, tt00,  tt1, tt10;
	double t, t0, t1;
	double Int0, Int1, R, Rs, Energy;
	int sts0, sts1;

	pseachstr[0] = searchstr;
	pseachstr[1] = searchstr1;

	fp = fopen(CSV_File,"r");
	if(fp == NULL) 
    {	printf("No file %s\n", CSV_File);
		exit(1); 
	}
	Int0 = Int1 = 0.;
	sts0 = sts1 = 0;
	for(;;)
	{  
	   pstr = fgets(str,250, fp);
       if(pstr == NULL)
		    break;
	   l = strlen(str);
		for(i=0; i<l;i++)
		{   if(str[i] == ',')
			   str[i] = ' ';
		}
		is = -1;
		for(j = 0; j<2; j++)
		{
			pstr = strstr(str,pseachstr[j]);
			if(!pstr)
				continue;
			lfs = strlen(pseachstr[j]);
			pstr += lfs;
			rc = sscanf(pstr,"%s %s",str0, str1);
			if(!strcmp(str0, "unavailable"))
				continue;
			else if(!strcmp(str0, "unknown"))
				continue;
			is = j;
			break;
		}
		if(is == -1)
			continue;
		rc =  sscanf(str0,"%lf", &t);
		if(rc != 1)
			continue;
		rc = parse_dt(str1, tt);
		if(rc != 0)
			continue;

		switch(is)
		{	case 0:
			if(sts0 == 0)
			{	sts0 = 1;
				t0 = t;
				tt0 = tt00 =  tt;
			} else {
				Int0 = Int0 + (t0 + t) * (tt - tt0) * 0.5;
				t0 = t;
				tt0 = tt;
			}

				break;
			case 1:
			if(sts1 == 0)
			{	sts1 = 1;
				t1 = t;
				tt1 = tt10 = tt;
			} else {
				Int1 = Int1 + (t1 + t) * (tt - tt1) * 0.5;
				t1 = t;
				tt1 = tt;
			}
				break;
		}
	}
	fclose(fp);


//	printf("Int %g %g\n", Int0, Int1);
//	printf("Int dt %g %g\n",  difftime(tt0,tt00), difftime(tt1,tt10));
	printf("Интеграл разности температур по времени: ");
	printf("%g градусов * сек\n", Int0- Int1);
	printf("Энергопотребление:\n");
	if(use_gas)
	{	printf("Газ %g м3\n", Gas_usedV);
		Energy =  Gas_usedV * GasSHC;
		printf("%g джоулей = ",Energy);
		printf("%g киловатт-часов\n", Energy / 3.6e6);
	} else {
		printf("Электроэнергия %g кВт*час\n", Electricity_used);
		Energy = Electricity_used * 3.6e6;
	}
	printf("Cредняя мощность %g киловатт\n", (Energy / 3.6e6) / (difftime(tt0,tt00)/3600.));

// W = S * (Tindoor - Toutdoor)/ R
// E = Int (W * dt) [from t0 to t1] = S/R * Int ((Tindoor - Toutdoor) * dt)[from t0 to t1]
// R = S*Int/E
//m2*град*сек/(Вт*сек)
	printf("Среднее сопротивление теплопередаче, (м2*С/Вт):\n");
	Rs = ((Int0 - Int1) * HomeS)  / Energy;
	R =  ((Int0 - Int1) * HomeCS) / Energy;
	printf("на кв.м площади: %g (Площадь %g)\n", Rs, HomeS);
	printf("на кв.м ограждающей поверхности: %g (Площадь %g)\n", R, HomeCS);

	return 0;
}



int parse_dt(char *date_str, time_t &t)
{
    int year, month, day, hour, min, sec, msec;
    struct tm tm_info = {0}; // Инициализация структуры tm

    // Формат для парсинга 'YYYY-MM-DDTHH:MM:SS.sssZ'
    // %d-%d-%dT%d:%d:%d.%dZ
    if (sscanf(date_str, "%d-%d-%dT%d:%d:%d.%dZ", &year, &month, &day, &hour, &min, &sec, &msec) == 7) {
        tm_info.tm_year = year - 1900; // Год от 1900
        tm_info.tm_mon = month - 1;    // Месяц от 0 (Январь)
        tm_info.tm_mday = day;
        tm_info.tm_hour = hour;
        tm_info.tm_min = min;
        tm_info.tm_sec = sec;
        tm_info.tm_isdst = -1; // Автоматическое определение летнего времени (не критично для UTC, но лучше установить)

        // Преобразуем в time_t (секунды с 1970-01-01 UTC)
        t = mktime(&tm_info);
#if 0
        printf("Год: %d, Месяц: %d, День: %d\n", year, month, day);
        printf("Час: %d, Мин: %d, Сек: %d, Миллисек: %d\n", hour, min, sec, msec);
        // Обратите внимание: mktime работает с локальным временем, а наша строка в UTC.
        // Для UTC лучше использовать gmtime_r
        printf("Время (Unix timestamp): %ld\n", (long)t);

        // Для UTC:
        struct tm *tm_utc;
        tm_utc = gmtime(&t); // Или gmtime_r для многопоточности
        printf("Время (UTC): %04d-%02d-%02d %02d:%02d:%02d\n",
               tm_utc->tm_year + 1900, tm_utc->tm_mon + 1, tm_utc->tm_mday,
               tm_utc->tm_hour, tm_utc->tm_min, tm_utc->tm_sec);
#endif
		return 0;
    } else {
        printf("Ошибка парсинга строки.\n");
		return 1;
    }
}

#ifndef _WIN32

long file_length(char *f)
{
    struct stat st;
    stat(f, &st);
    return st.st_size;
}

#endif
