/* Rcalc.hpp */

#ifndef RCALC_H
#define RCALC_H
#include "AutoConfig.h"

class Rcalc
{
 public:
	int configversion;
	double GasSHC; //Specific heat of combustion Удельная теплота сгорания J/m3  
	double Gas_usedV; //объем потребления газа, m3
	double HomeS; // house area, sq.m | площадь дома, m2
	double HomeCS; //area of ​​the house enclosing structure, sq.m | площадь ограждающих конструкций дома, (стены+пол+потолок) m2
	char dataFile[256]; //csv file
	int use_gas; //use gas (1) or electricity (0)
	double Electricity_used; // electricity consumption, kilowatt * hour kWt*h

	AutoConfig cfg;

	Rcalc(void)
	{	configversion =-1;
		GasSHC = 33500000.;//33.5|45.2 MJ/m3
		Gas_usedV = 0.;
		HomeS = 100.;
		HomeCS = 100*2 + 10*4*3;
		Gas_usedV = 100.;
		use_gas = 1;
		Electricity_used = 0.;
		strcpy(dataFile,"history.csv");
	}
	int init(char *ConfigFile);
	int read_csv(char *CSV_File);
};

#endif //RCALC_H
