/*
*i2ctest.c
*	Raspberry Pi I2C test using wiringPi library.
*
*Copyright (c) Nahid Alam. <nahid.mahfuza.alam@gmail.com>
***********************************************************
*i2ctest is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    i2ctest is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
***********************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <time.h>
#include <unistd.h>

#include "AD5933.h"
#include "AD5933.c"

// I2c variables declaration
int i2cdevice;

// Calibration variables
unsigned long START_FREQ = 3000;	 // in Hertz (unsigned long has 32 bits, from which we will only need 24)
unsigned long INCREMENT_FREQ = 1000; // in Hertz
unsigned short NPOINTS = 100;
double gainFactor = 0;

// Measurements variables declaration
unsigned long TEMPERATURE = 0;
double magnitude = 0;
unsigned long CurrentFrequency = START_FREQ;

// Extra variables
int status = 0;
int i = 0;
float Calibration_Impedance = 0;
float AD5933_CALIBRATION_IMPEDANCE = 12000;
unsigned long freq_iter = 1;
char string_tmp;

void formatTime(char *out, int tipo)
{
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	switch (tipo)
	{
	case 1:
		sprintf(out, "%04d%02d%02d_%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		break;
	case 2:
		sprintf(out, "%04d%02d%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		break;
	default:
		break;
	}
}

int main(void)
{
	// Setup wiringPi library to use i2c module
	wiringPiSetup();

	// Stablish communication with AD933 (0x0d address)
	i2cdevice = wiringPiI2CSetup(0x0d); /*Use i2cdetect command to find your respective device address*/

	// If wiringPiI2cSetup returned -1, device is down or we could not
	// stablish communication for some reason, so return.
	if (i2cdevice == -1)
	{
		printf("Can't setup the I2C device\n");
		return -1;
	}
	else
	{
		printf("I2C device %x hash.\n", i2cdevice);
	}

	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("\nCurrent time is : %s \t", ctime(&rawtime));
	char outFormatTime[40];
	formatTime(outFormatTime, 1);
	printf("Format Time : %s \n", outFormatTime);

	double temperatureInicial = 0;
	double temperatureFinal = 0;
	printf("Obteniendo Temperatura.\n");
	temperatureInicial = AD5933_GetTemperatureV2();
	printf("Temperature %f", temperatureInicial);

	printf("\n\n");
	printf("/******************************************************************************\n");
	printf("* AD5933 - barrido de frecuencia completo                                     *\n");
	printf("* Maximiliano Sampirisi (msampirisi@gmail.com) - Ushuaia (Argentina)          *\n");
	printf("-------------------------------------------------------------------------------\n");

	currentRange = AD5933_RANGE_2000mVpp;
	printf("Setting Range to %d (%u)\n", currentRange, currentRange);
	currentGain = AD5933_GAIN_X1;
	printf("Setting Gain to %d (%u)\n", currentGain, currentGain);
	AD5933_CALIBRATION_IMPEDANCE = 100000.0F;
	printf("Calibration Resistor %f Ohm\n", AD5933_CALIBRATION_IMPEDANCE);

	// control de las frecuencias a recorrer
	unsigned long frecuenciaIni = 1l;
	unsigned long frecuenciaFin = 100000l;
	unsigned long frecuenciaInc = 1000l;
	unsigned short puntosPasada = 511;
	unsigned long puntosTotales = (frecuenciaFin - frecuenciaIni + 1) / frecuenciaInc;

	printf("I:%lu\tF:%lu\tInc:%lu\tPuntos:%lu\n", frecuenciaIni, frecuenciaFin, frecuenciaInc, puntosTotales);

	// control de las pasadas sucesivas
	unsigned long frecuenciaActIni = 0;
	unsigned long frecuenciaActFin = 0;

	// variables necesarias
	double CalibrationGainFactor[puntosPasada] = {0};
	double CalibrationSystemPhase[puntosPasada] = {0};
	double impedance = 0;
	double systemPhase = 0;
	double meansurePhase = 0;
	double phase = 0;
	signed short RealPart = 0;
	signed short ImagPart = 0;

	// Inicializando archivo de datos
	//char idMedicion[100];
	char idMedicionCtrl[200];
	char fileName[210];
	const char* idMedicion = "rango_total";
	sprintf(idMedicionCtrl, "%s_%g_%s", idMedicion, AD5933_CALIBRATION_IMPEDANCE, outFormatTime);
	printf("IdMedicion : %s (%s)\n", idMedicion, idMedicionCtrl);
	sprintf(fileName, "%s.friip.txt", idMedicionCtrl);
	FILE *fout;
	fout = fopen(fileName, "w");
	fprintf(fout, "frequency\tP.Real\tP.Imag\timpedance\tphase\n");
	fflush(stdout);

	frecuenciaActIni = frecuenciaIni;
	frecuenciaActFin = frecuenciaActIni + puntosPasada * frecuenciaInc;
	while (frecuenciaActIni <= frecuenciaFin)
	{
		printf("frecuencia => inicio : %lu / final : %lu\n", frecuenciaActIni, frecuenciaActFin);
		START_FREQ = frecuenciaActIni;
		INCREMENT_FREQ = frecuenciaInc;
		NPOINTS = puntosPasada;

		printf("AD5933 - Reset\n");
		AD5933_Reset();
		printf("AD5933 - Setting Range to %d and Gain to %d \n", currentRange, currentGain);
		AD5933_SetRangeAndGain(currentRange, currentGain);
		AD5933_SetRegisterValue(AD5933_REG_SETTLING_CYCLES, 100, 2);
		printf("AD5933 - Config Sweep [%lu][%lu][%d]", START_FREQ, INCREMENT_FREQ, NPOINTS);
		AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);
		printf("AD5933 - Start Sweep\n");
		AD5933_StartSweep();
		printf("AD5933 - Calibracion (GainFactor/SystemPhase) - inicio\n");
		freq_iter = 0;
		i = 0;
		for (i = 0; i < (NPOINTS + 1); i++)
		{
			printf(".");
			CurrentFrequency = START_FREQ + INCREMENT_FREQ * freq_iter;
			if (freq_iter == 0)
			{
				gainFactor = AD5933_CalculateGainFactorAndSystemPhase(AD5933_CALIBRATION_IMPEDANCE,
																	  AD5933_FUNCTION_REPEAT_FREQ, &systemPhase);
			}
			else
			{
				gainFactor = AD5933_CalculateGainFactorAndSystemPhase(AD5933_CALIBRATION_IMPEDANCE,
																	  AD5933_FUNCTION_INC_FREQ, &systemPhase);
			}
			CalibrationGainFactor[i] = gainFactor;
			CalibrationSystemPhase[i] = systemPhase;
			//magnitude = AD5933_CalculateImpedance(gainFactor, AD5933_FUNCTION_REPEAT_FREQ);
			freq_iter += 1;
			fflush(stdout);
		}
		printf("\n");
		printf("AD5933 - Calibracion (GainFactor/SystemPhase) - final\n");
		printf("AD5933 - Medicion\n");
		printf("AD5933 - Config Sweep [%lu][%lu][%d]", START_FREQ, INCREMENT_FREQ, NPOINTS);
		AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);
		printf("AD5933 - Start Sweep\n");
		AD5933_StartSweep();
		freq_iter = 0;
		i = 0;
		for (i = 0; i < (NPOINTS + 1); i++)
		{
			CurrentFrequency = START_FREQ + INCREMENT_FREQ * freq_iter;
			if (freq_iter == 0)
			{
				magnitude = AD5933_CalculateImpedanceV3(CalibrationGainFactor[i], AD5933_FUNCTION_REPEAT_FREQ, &RealPart, &ImagPart, &meansurePhase);
			}
			else
			{
				magnitude = AD5933_CalculateImpedanceV3(CalibrationGainFactor[i], AD5933_FUNCTION_INC_FREQ, &RealPart, &ImagPart, &meansurePhase);
			}
			if ((CalibrationGainFactor[i] * magnitude) == 0)
			{
				impedance = AD5933_CALIBRATION_IMPEDANCE;
			}
			else
			{
				impedance = (1 / (CalibrationGainFactor[i] * magnitude));
			}
			phase = meansurePhase - CalibrationSystemPhase[i];
			printf("%lu\t%d\t%d\t%f\t%f\n", CurrentFrequency, RealPart, ImagPart, impedance, phase);
			fflush(stdout);
			fprintf(fout, "%lu\t%d\t%d\t%f\t%f\n", CurrentFrequency, RealPart, ImagPart, impedance, phase);
			fflush(fout);
			
			status = AD5933_GetRegisterValue(AD5933_REG_STATUS, 1);
			freq_iter += 1;
		}


		frecuenciaActIni = frecuenciaActFin + frecuenciaInc;
		frecuenciaActFin = frecuenciaActIni + puntosPasada * frecuenciaInc;
	}

	fclose(fout);
	printf("\n Obteniendo Temperatura : \n");
	temperatureFinal = AD5933_GetTemperatureV2();
	printf("Temperature Inicial    : %f\n", temperatureInicial);
	printf("Temperature Final      : %f\n", temperatureFinal);
	printf("Temperature Diferencia : %f (%f)\n", temperatureFinal - temperatureInicial, (temperatureFinal - temperatureInicial) / temperatureInicial);

	return 0;
}
