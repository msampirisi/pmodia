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
	printf("Obteniendo Temperatura.\n");
	temperatureInicial = AD5933_GetTemperatureV2();
	printf("Temperature %f", temperatureInicial);

	printf("\n\n");
	printf("/******************************************************************************\n");
	printf("*                           Fluid Spectra App v0.0                            *\n");
	printf("*-----------------------------------------------------------------------------*\n");
	printf("* @authors: Manuel Blanco Valentin (mbvalentin@cbpf.br) - Barcelona (Spain)   *\n");
	printf("*           Yann Le Guevel (...) - Strasbourg (France)                        *\n");
	printf("*           Maximiliano Sampirisi (msampirisi@gmail.com) - Ushuaia (Argentina)*\n");
	printf("*                                                                             *\n");
	printf("* @creation: June/2017 at CBPF (Brazil)                                       *\n");
	printf("*                                                                             *\n");
	printf("------------------------------------------------------------------------------\n");

	printf("\tWould you like to set the parameters? [y/n]: ");
	string_tmp = getchar();
	if (string_tmp == 'y')
	{

		/* Main Configuration parameters */
		printf("                            Initial Parameters Setup                          \n");
		printf("                                                                              \n");
		printf("                         (ENTER -1 FOR DEFAULT VALUES)                        \n");
		printf("------------------------------------------------------------------------------\n");

		/* Range (0 to 3)
				0 -> AD5933_RANGE_2000mVpp 
				1 -> AD5933_RANGE_200mVpp
				2 -> AD5933_RANGE_400mVpp
				3 -> AD5933_RANGE_1000mVpp
			*/
		printf(" Output voltage range (0->2000mVpp, 1->200mVpp, 2->400mVpp, 3->1000mVpp): ");
		scanf("%hhu", &currentRange);
		if ((currentRange < 0) | (currentRange > 3))
		{
			printf("Setting Range to default value (2000mVpp)\n");
			currentRange = AD5933_RANGE_2000mVpp;
		}

		/* Gain (0 to 1)
				0 -> AD5933_GAIN_X5 
				1 -> AD5933_GAIN_X1
			*/
		printf("\n Control PGA Gain (0->X5, 1->X1): ");
		scanf("%hhu", &currentGain);
		if ((currentGain < 0) | (currentGain > 1))
		{
			printf("Setting Gain to default value (X1)\n");
			currentGain = AD5933_GAIN_X1;
		}

		/* Frequency Sweep Parameters */
		printf("--------------------------------------------------------------------------------\n");
		printf("                        Frequency Sweep Parameters Setup                        \n");
		printf("--------------------------------------------------------------------------------\n");

		// Starting Frequency
		printf(" Starting Frequency (current is %lu Hz): ", START_FREQ);
		scanf("%lu", &START_FREQ);
		if (START_FREQ <= 0)
		{
			printf("Setting Starting Frequency to default value (3 kHz)\n");
			START_FREQ = 3000;
		}

		// Increment Frequency
		printf(" Increment Frequency (current is %lu Hz): ", INCREMENT_FREQ);
		scanf("%lu", &INCREMENT_FREQ);
		if (INCREMENT_FREQ <= 0)
		{
			printf("Setting Increment Frequency to default value (1.5 kHz)\n");
			INCREMENT_FREQ = 1500;
		}

		// Number of points
		printf(" Sweep Number of Points (current is %hu): ", NPOINTS);
		scanf("%hu", &NPOINTS);
		if ((NPOINTS <= 0) | (NPOINTS > AD5933_MAX_INC_NUM))
		{
			printf("\tSetting Number of points to maximum value (%d)\n", AD5933_MAX_INC_NUM);
			NPOINTS = AD5933_MAX_INC_NUM;
		}

		/* Calibration parameters */
		printf("--------------------------------------------------------------------------------\n");
		printf("                         Calibration Parameters Setup                           \n");
		printf("--------------------------------------------------------------------------------\n");

		// Calibration resistor
		printf(" Calibration Resistor (current value: %f Ohm): ", AD5933_CALIBRATION_IMPEDANCE);
		scanf("%f", &Calibration_Impedance);
		if ((Calibration_Impedance > 0) & (Calibration_Impedance != AD5933_CALIBRATION_IMPEDANCE))
		{
			AD5933_CALIBRATION_IMPEDANCE = Calibration_Impedance;
		}
	}

	// Define some variables
	CurrentFrequency = START_FREQ;
	double Z_MOD[NPOINTS] = {0};
	double Z_REAL[NPOINTS] = {0};
	double Z_IMAG[NPOINTS] = {0};
	double impedance = 0;
	double impedanceMax = 0;
	double impedanceMin = __DBL_MAX__;
	double impedanceAvg = 0;
	double impedanceTot = 0;
	double systemPhase = 0;
	double meansurePhase = 0;
	double phase = 0;
	double phaseMax = 0;
	double phaseMin = __DBL_MAX__;
	double phaseAvg = 0;
	double phaseTot = 0;
	int WINDOW = 5;

	// ******************** DEMO STARTS ****************************
	// Set the Range and Gain
	printf("\n Setting Range to %d and Gain to %d     \n ", currentRange, currentGain);
	AD5933_SetRangeAndGain(currentRange, currentGain);
	printf(" Done!\n");

	// Proceso de toma de muestras
	// crear todas las variables a utilizar
	char askYN;

	char idMedicion[100];
	char idMedicionCtrl[200];
	char fileName2[204];

	FILE *gnuplot;
	FILE *fout;
	FILE *fout2;
	FILE *fctl;

	signed short RealPart = 0;
	signed short ImagPart = 0;

	double temperatureFinal = 0;
	const char *fileNameControl = "pmodia.control.txt";
	char outFormatTime2[40];

	int repetirProceso = 1;

	do{

		freq_iter = 1;

		// Configure sweep
		printf("\n Configuring the Sweep \n ");
		AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);
		printf(" Done!\n");

		// Start the sweep
		AD5933_StartSweep();

		// Calculate gain factor for calibration impedance
		printf("\n Calculating Gain Factor \n");

		gainFactor = AD5933_CalculateGainFactorAndSystemPhase(AD5933_CALIBRATION_IMPEDANCE,
															  AD5933_FUNCTION_REPEAT_FREQ, &systemPhase);
		printf("Done!...\n");
		printf("---> Gain Factor  estimated to be: %g\n", gainFactor);
		printf("---> System Phase estimated to be: %g\n", systemPhase);

		// Make a single impedance measurement to make sure we have
		// calibrated the board correctly
		magnitude = AD5933_CalculateImpedance(gainFactor,
											  AD5933_FUNCTION_REPEAT_FREQ);
		printf("Recalculated Z = %f .. \nOriginal one had a value of: %f ... \nError = %f%%\n", (1 / (gainFactor * magnitude)), AD5933_CALIBRATION_IMPEDANCE, 100 * abs((AD5933_CALIBRATION_IMPEDANCE - (1 / (gainFactor * magnitude)))) / AD5933_CALIBRATION_IMPEDANCE);

		// Punto de Replicacion
		// Crear todas las variables locales que se usaran sucesivamente

		printf("\nMeasurement Identification (reemplazar el componente a medir): ");
		scanf("%s", &string_tmp);

		sprintf(idMedicion, "%s", &string_tmp);
		sprintf(idMedicionCtrl, "%s_%g_%s", idMedicion, AD5933_CALIBRATION_IMPEDANCE, outFormatTime);

		printf("\nIdMedicion : %s (%s)\n", idMedicion, idMedicionCtrl);

		printf("Debug: Obteniendo el registro de status.\n");

		status = AD5933_GetRegisterValue(AD5933_REG_STATUS, 1);

		printf("Debug: Inicializando archivos de salida.\n");

		// Initialize variables for output txt file and gnuplot
		gnuplot = popen("gnuplot -persistent", "w");
		fout = fopen("out.txt", "w");

		sprintf(fileName2, "%s.txt", idMedicionCtrl);
		fout2 = fopen(fileName2, "w");

		RealPart = 0;
		ImagPart = 0;

		fprintf(gnuplot, "plot '-' with lines\n");
		fprintf(fout2, "P.Real\tP.Imag\timpedance\tphase\tfrequency\n");

		i = 0;
		for (i = 0; i < NPOINTS; i++)
		{
			//TEMPERATURE
			//TEMPERATURE = AD5933_GetTemperature();

			// Calculate impedance between Vout and Vin
			magnitude = AD5933_CalculateImpedanceV3(gainFactor, AD5933_FUNCTION_REPEAT_FREQ, &RealPart, &ImagPart, &meansurePhase);
			impedance = (1 / (gainFactor * magnitude));
			phase = meansurePhase - systemPhase;

			if (impedanceMax < impedance) impedanceMax = impedance;
			if (impedanceMin > impedance) impedanceMin = impedance;
			impedanceTot = impedanceTot + impedance;

			if (phaseMax < phase) phaseMax = phase;
			if (phaseMin > phase) phaseMin = phase;
			phaseTot = phaseTot + phase;

			Z_MOD[freq_iter] = impedance;
			Z_IMAG[freq_iter] = ImagPart;
			Z_REAL[freq_iter] = RealPart;
			CurrentFrequency = START_FREQ + INCREMENT_FREQ * freq_iter;

			// Print impedance
			printf("Impedance read: %f ohms (phase: %f) (@ %lu Hz) (dif: %f)\n\r", impedance, phase, CurrentFrequency, (impedance / Calibration_Impedance - 1) * 100);
			// printf("TEMPERATURE: %lu\n",TEMPERATURE);
			fprintf(fout, "%f\t%f\t%lu\n", impedance, phase, CurrentFrequency);
			fprintf(fout2, "%d\t%d\t%f\t%f\t%lu\n", RealPart, ImagPart, impedance, phase, CurrentFrequency);
			fprintf(gnuplot, "%lu %f\n", CurrentFrequency, impedance);

			fflush(fout);
			fflush(fout2);
			fflush(gnuplot);

			status = AD5933_GetRegisterValue(AD5933_REG_STATUS, 1);
			freq_iter += 1;
		}

		fprintf(gnuplot, "e\n");
		fflush(gnuplot);

		fclose(fout);
		fclose(fout2); // archivo extendido

		printf("\n Obteniendo Temperatura : \n");
		temperatureFinal = AD5933_GetTemperatureV2();
		printf("Temperature Inicial    : %f\n", temperatureInicial);
		printf("Temperature Final      : %f\n", temperatureFinal);
		printf("Temperature Diferencia : %f (%f)\n", temperatureFinal - temperatureInicial, (temperatureFinal - temperatureInicial) / temperatureInicial);

		printf("Debug: Actualizando archivo de control. Iniciando.\n");

		// Archivo de control de ejecuciones
		fctl = fopen(fileNameControl, "a");
		fseek(fctl, 0, SEEK_END);
		if (ftell(fctl) == 0)
		{
			printf("Debug: Actualizando archivo de control. Creando Titulos.\n");

			fprintf(fctl, "fechahora\t");
			fprintf(fctl, "idMedicion\t");
			fprintf(fctl, "tempIni\t");
			fprintf(fctl, "tempFin\t");
			fprintf(fctl, "Range\t");
			fprintf(fctl, "Gain\t");
			fprintf(fctl, "FreqIni\t");
			fprintf(fctl, "FreqInc\t");
			fprintf(fctl, "FreqPoi\t");
			fprintf(fctl, "calibImpedance\t");
			fprintf(fctl, "gainFactor\t");
			fprintf(fctl, "systemPhase\t");
			fprintf(fctl, "impedanceMax\t");
			fprintf(fctl, "impedanceAvg\t");
			fprintf(fctl, "impedanceMin\t");
			fprintf(fctl, "phaseMax\t");
			fprintf(fctl, "phaseAvg\t");
			fprintf(fctl, "phaseMin\t");
			fprintf(fctl, "dataFile\t");
			fprintf(fctl, "\n");
		}

		// calculo de promedios
		impedanceAvg = impedanceTot / NPOINTS;
		phaseAvg = phaseTot / NPOINTS;

		// genero un archivo de control de todas las corridas
		printf("Debug: Actualizando archivo de control. Agregando Ejecucion.\n");

		formatTime(outFormatTime2, 2);
		fprintf(fctl, "%s\t", outFormatTime2);
		fprintf(fctl, "%s\t", idMedicion);
		fprintf(fctl, "%f\t", temperatureInicial);
		fprintf(fctl, "%f\t", temperatureFinal);
		fprintf(fctl, "%hu\t", currentRange);
		fprintf(fctl, "%hu\t", currentGain);
		fprintf(fctl, "%lu\t", START_FREQ);
		fprintf(fctl, "%lu\t", INCREMENT_FREQ);
		fprintf(fctl, "%hu\t", NPOINTS);
		fprintf(fctl, "%f\t", AD5933_CALIBRATION_IMPEDANCE);
		fprintf(fctl, "%g\t", gainFactor);
		fprintf(fctl, "%g\t", systemPhase);
		fprintf(fctl, "%f\t", impedanceMax);
		fprintf(fctl, "%f\t", impedanceAvg);
		fprintf(fctl, "%f\t", impedanceMin);
		fprintf(fctl, "%f\t", phaseMax);
		fprintf(fctl, "%f\t", phaseAvg);
		fprintf(fctl, "%f\t", phaseMin);
		fprintf(fctl, "%s\t", fileName2);
		fprintf(fctl, "\n");
		fflush(fctl);
		fclose(fctl); // archivo extendido

		printf("Debug: Actualizando archivo de control. finalizando.\n");

		fflush(stdin);
		printf("\nDesea realizar una nueva medicion? (Y/N)");
		scanf("%c", &askYN);
		printf("[ %c ]",askYN);
		if (askYN != 'Y' && askYN != 'y')
		{
			break;
		}
	} while (1 == 1);

	return 0;
}
