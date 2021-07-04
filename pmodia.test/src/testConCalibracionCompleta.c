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

	/*
	debug status de los registros
	*/

	//  Setting time cycles register
	AD5933_SetRegisterValue(AD5933_REG_SETTLING_CYCLES, 100, 2);
	//

	printf("Debug - I");
	printf("Registro \t valor\n");
	printf("0x80\t%x\n", AD5933_GetRegisterValue(0x80, 2));
	printf("0x82\t%x\n", AD5933_GetRegisterValue(0x82, 1));
	printf("0x83\t%x\n", AD5933_GetRegisterValue(0x83, 2));
	printf("0x85\t%x\n", AD5933_GetRegisterValue(0x85, 1));
	printf("0x86\t%x\n", AD5933_GetRegisterValue(0x86, 2));
	printf("0x88\t%x\n", AD5933_GetRegisterValue(0x88, 2));
	printf("0x8A\t%x\n", AD5933_GetRegisterValue(0x8A, 1));
	printf("0x8B\t%x\n", AD5933_GetRegisterValue(0x8B, 1));
	printf("0x8F\t%x\n", AD5933_GetRegisterValue(0x8F, 1));
	printf("0x92\t%x\n", AD5933_GetRegisterValue(0x92, 2));
	printf("0x94\t%x\n", AD5933_GetRegisterValue(0x94, 2));
	printf("0x96\t%x\n", AD5933_GetRegisterValue(0x96, 2));
	printf("Debug - F");

	// Define some variables
	CurrentFrequency = START_FREQ;
	double Z_MOD[NPOINTS + 1] = {0};
	double Z_REAL[NPOINTS + 1] = {0};
	double Z_IMAG[NPOINTS + 1] = {0};
	double CalibrationGainFactor[NPOINTS + 1] = {0};
	double CalibrationSystemPhase[NPOINTS + 1] = {0};
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

	// Proceso de toma de muestras
	// crear todas las variables a utilizar
	char askYN;

	char idMedicion[100];
	char idMedicionCtrl[200];
	char fileName2[210];

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

	status = AD5933_GetRegisterValue(AD5933_REG_STATUS, 1);
	printf("AD5933 - status - %d\n", status);

	// Preparo el equipo para una lectura.
	printf("AD5933 - Reset\n");
	AD5933_Reset();

	// Set the Range and Gain
	printf("AD5933 - Setting Range to %d and Gain to %d \n", currentRange, currentGain);
	AD5933_SetRangeAndGain(currentRange, currentGain);

	// Configure sweep
	printf("AD5933 - Configuring the Sweep\n");
	AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);

	// Start the sweep
	printf("AD5933 - Start Sweep\n");
	AD5933_StartSweep();

	printf("AD5933 - Calculating Gain Factor and SystemPhase\n");

	freq_iter = 0;
	i = 0;
	for (i = 0; i < (NPOINTS + 1); i++)
	{
		CurrentFrequency = START_FREQ + INCREMENT_FREQ * freq_iter;
		// Calculate gain factor for calibration impedance
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

		// printf("---> Gain Factor  estimated to be: %g\n", gainFactor);
		// printf("---> System Phase estimated to be: %g\n", systemPhase);
		CalibrationGainFactor[i] = gainFactor;
		CalibrationSystemPhase[i] = systemPhase;

		// Make a single impedance measurement to make sure we have
		// calibrated the board correctly
		// printf("AD5933 - Calculating Impedance\n");
		magnitude = AD5933_CalculateImpedance(gainFactor, AD5933_FUNCTION_REPEAT_FREQ);
		printf("[Gain : %g / Phase : %g ] Recalculated Z = %f .. \nOriginal one had a value of: %f ... \nError = %f%%\n",gainFactor, systemPhase, (1 / (gainFactor * magnitude)), AD5933_CALIBRATION_IMPEDANCE, 100 * abs((AD5933_CALIBRATION_IMPEDANCE - (1 / (gainFactor * magnitude)))) / AD5933_CALIBRATION_IMPEDANCE);
		freq_iter += 1;
	}

/*
	unsigned long START_FREQ = 3000;	 // in Hertz (unsigned long has 32 bits, from which we will only need 24)
	unsigned long INCREMENT_FREQ = 1000; // in Hertz
	unsigned short NPOINTS = 100;
*/
	do
	{

		// Configure sweep
		printf("AD5933 - Configuring the Sweep\n");
		AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);

		// Start the sweep
		printf("AD5933 - Start Sweep\n");
		AD5933_StartSweep();

		// Punto de Replicacion
		// Crear todas las variables locales que se usaran sucesivamente

		printf("AD5933 - Measurement Identification (reemplazar el componente a medir): ");
		scanf("%s", &string_tmp);

		sprintf(idMedicion, "%s", &string_tmp);
		sprintf(idMedicionCtrl, "%s_%g_%s", idMedicion, AD5933_CALIBRATION_IMPEDANCE, outFormatTime);

		printf("IdMedicion : %s (%s)\n", idMedicion, idMedicionCtrl);
		sprintf(fileName2, "%s.friip.txt", idMedicionCtrl);

		printf("Debug: Obteniendo el registro de status.\n");

		status = AD5933_GetRegisterValue(AD5933_REG_STATUS, 1);

		printf("Debug: Inicializando archivos de salida.\n");

		// Initialize variables for output txt file and gnuplot
		printf("Debug: Inicializando gnuplot.\n");
		gnuplot = popen("gnuplot -persistent", "w");

		printf("Debug: Inicializando fout.\n");
		fout = fopen("out.txt", "w");
		printf("Debug: Inicializando fout2.\n");
		fout2 = fopen(fileName2, "w");

		printf("Debug: Inicializando gnuplot - plot - with lines.\n");
		fprintf(gnuplot, "set title \"Impedancia\" \n");
		fprintf(gnuplot, "plot '-' with lines\n");

		printf("Debug: fout2 - titulando.\n");
		fprintf(fout2, "frequency\tP.Real\tP.Imag\timpedance\tphase\n");

		RealPart = 0;
		ImagPart = 0;
		freq_iter = 0;

		printf("Debug: Iniciando el barrido. [%d]\n", AD5933_GetRegisterValue(AD5933_REG_STATUS, 1));
		i = 0;
		for (i = 0; i < (NPOINTS + 1); i++)
		{
			CurrentFrequency = START_FREQ + INCREMENT_FREQ * freq_iter;

			//magnitude = AD5933_CalculateImpedanceV3(gainFactor, AD5933_FUNCTION_REPEAT_FREQ, &RealPart, &ImagPart, &meansurePhase);
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

			if (impedanceMax < impedance)
				impedanceMax = impedance;
			if (impedanceMin > impedance)
				impedanceMin = impedance;
			impedanceTot = impedanceTot + impedance;

			if (phaseMax < phase)
				phaseMax = phase;
			if (phaseMin > phase)
				phaseMin = phase;
			phaseTot = phaseTot + phase;

			Z_MOD[freq_iter] = impedance;
			Z_IMAG[freq_iter] = ImagPart;
			Z_REAL[freq_iter] = RealPart;

			// printf("TEMPERATURE: %lu\n",TEMPERATURE);
			fprintf(fout, "%f\t%f\t%lu\n", impedance, phase, CurrentFrequency);
			fprintf(fout2, "%lu\t%d\t%d\t%f\t%f\n", CurrentFrequency, RealPart, ImagPart, impedance, phase);
			//fprintf(gnuplot, "%lu %f\n", CurrentFrequency, impedance);
			fprintf(gnuplot, "%lu %d\n", CurrentFrequency, ImagPart);
			//fprintf(gnuplot, "%lu %d\n", CurrentFrequency, ImagPart);

			fflush(fout);
			fflush(fout2);
			fflush(gnuplot);

			status = AD5933_GetRegisterValue(AD5933_REG_STATUS, 1);
			printf("%03d Impedance read: %f ohms (phase: %f) (@ %lu Hz) [%d]\n", i, impedance, phase, CurrentFrequency, status);
			freq_iter += 1;
		}

		fprintf(gnuplot, "e\n");

		fflush(gnuplot);

		fclose(fout);
		fclose(fout2); // archivo extendido

		fclose(gnuplot);

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
		scanf("%s", &askYN);
		printf("[ %s ]", &askYN);
		if (askYN != 'Y' && askYN != 'y')
		{
			break;
		}
	} while (1 == 1);

	return 0;
}
