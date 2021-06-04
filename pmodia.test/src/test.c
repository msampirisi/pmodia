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
unsigned long START_FREQ = 3000; // in Hertz (unsigned long has 32 bits, from which we will only need 24)
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
unsigned long  freq_iter = 1;
char string_tmp;

void formatTime(char *out){
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(out, "%04d%02d%02d_%02d%02d%02d", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}


int main (void)
{
        // Setup wiringPi library to use i2c module
        wiringPiSetup () ;
        
        // Stablish communication with AD933 (0x0d address)
        i2cdevice = wiringPiI2CSetup (0x0d) ;  /*Use i2cdetect command to find your respective device address*/
        
        // If wiringPiI2cSetup returned -1, device is down or we could not
        // stablish communication for some reason, so return.
        if(i2cdevice==-1)
        {
			printf("Can't setup the I2C device\n");
			return -1;
        }else{
			printf("I2C device %x hash.\n",i2cdevice);
		}

		time_t rawtime;
		struct tm * timeinfo;
		time (&rawtime);
		timeinfo = localtime(&rawtime);
		printf("\nCurrent time is : %s", ctime(&rawtime));
		char outFormatTime[40];
		formatTime(outFormatTime);
		printf("Format Time : %s \n", outFormatTime);

		double temperatureInicial = 0;
        printf("\nObteniendo Temperatura :\n");
		temperatureInicial = AD5933_GetTemperatureV2();
        printf("Temperature %f",temperatureInicial);

        
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
			scanf("%hhu",&currentRange);
			if ((currentRange < 0) |( currentRange > 3)) { printf("Setting Range to default value (2000mVpp)\n"); currentRange = AD5933_RANGE_2000mVpp; }
			
			/* Gain (0 to 1)
				0 -> AD5933_GAIN_X5 
				1 -> AD5933_GAIN_X1
			*/
			printf("\n Control PGA Gain (0->X5, 1->X1): ");
			scanf("%hhu",&currentGain);
			if ((currentGain < 0) |( currentGain > 1)) { printf("Setting Gain to default value (X1)\n"); currentGain = AD5933_GAIN_X1; }
		   
			
			/* Frequency Sweep Parameters */
			printf("--------------------------------------------------------------------------------\n");
			printf("                        Frequency Sweep Parameters Setup                        \n");
			printf("--------------------------------------------------------------------------------\n");
			
			// Starting Frequency
			printf(" Starting Frequency (current is %lu Hz): ",START_FREQ);
			scanf("%lu",&START_FREQ);
			if (START_FREQ <= 0) { printf("Setting Starting Frequency to default value (3 kHz)\n"); START_FREQ = 3000; }
			
			// Increment Frequency
			printf(" Increment Frequency (current is %lu Hz): ",INCREMENT_FREQ);
			scanf("%lu",&INCREMENT_FREQ);
			if (INCREMENT_FREQ <= 0) { printf("Setting Increment Frequency to default value (1.5 kHz)\n"); INCREMENT_FREQ = 1500; }
			
			// Number of points
			printf(" Sweep Number of Points (current is %hu): ",NPOINTS);
			scanf("%hu",&NPOINTS);
			if ((NPOINTS <= 0) |( NPOINTS > AD5933_MAX_INC_NUM)) { printf("\tSetting Number of points to maximum value (%d)\n",AD5933_MAX_INC_NUM); NPOINTS = AD5933_MAX_INC_NUM; }
			

			
			/* Calibration parameters */
			printf("--------------------------------------------------------------------------------\n");
			printf("                         Calibration Parameters Setup                           \n");
			printf("--------------------------------------------------------------------------------\n");
			
			// Calibration resistor
			printf(" Calibration Resistor (current value: %f Ohm): ",AD5933_CALIBRATION_IMPEDANCE);
			scanf("%f",&Calibration_Impedance);
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
		double SystemPhase = 0;
		double phase = 0;
        int WINDOW = 5;

        // ******************** DEMO STARTS ****************************
        // Set the Range and Gain
        printf("\n Setting Range to %d and Gain to %d     \n ",currentRange,currentGain);
	    AD5933_SetRangeAndGain(currentRange,currentGain);
	    printf(" Done!\n");
	    
	    // Configure sweep
	    printf("\n Configuring the Sweep \n ");
	    AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);
	    printf(" Done!\n");
	    
	    // Start the sweep
	    AD5933_StartSweep();
		   
	    // Calculate gain factor for calibration impedance
	    printf("\n Calculating Gain Factor \n");
	    /*
		gainFactor = AD5933_CalculateGainFactor(AD5933_CALIBRATION_IMPEDANCE,
									AD5933_FUNCTION_REPEAT_FREQ);
		*/
									
	    gainFactor = AD5933_CalculateGainFactorAndSystemPhase(AD5933_CALIBRATION_IMPEDANCE,
									AD5933_FUNCTION_REPEAT_FREQ, &SystemPhase);
	    printf("Done!...\n");
	    printf("---> Gain Factor  estimated to be: %g\n",gainFactor);
	    printf("---> System Phase estimated to be: %g\n",SystemPhase);
        
        // Make a single impedance measurement to make sure we have  
		// calibrated the board correctly
		magnitude = AD5933_CalculateImpedance(gainFactor,
								  AD5933_FUNCTION_REPEAT_FREQ);
	    printf("Recalculated Z = %f .. \nOriginal one had a value of: %f ... \nError = %f%%\n",(1/(gainFactor*magnitude)),AD5933_CALIBRATION_IMPEDANCE,100*abs((AD5933_CALIBRATION_IMPEDANCE-(1/(gainFactor*magnitude))))/AD5933_CALIBRATION_IMPEDANCE);
        
        //printf("\nReplace calibration component with desired one for measurement and press any key");
		// wait for user input
		//scanf("%s",&string_tmp);

		printf("\nMeasurement Identification : ");
		scanf("%s",&string_tmp);
		printf("\nIdMedicion : %s_%g_%s\n",&string_tmp,Calibration_Impedance,outFormatTime);

		printf("Debug: Obteniendo el registro de status.\n");		
			
		status = AD5933_GetRegisterValue(AD5933_REG_STATUS,1);

		printf("Debug: Inicializando archivos de salida.\n");		
			
        // Initialize variables for output txt file and gnuplot
		FILE *gnuplot = popen("gnuplot -persistent", "w");
        FILE *fout = fopen("out.txt","w");

		char fileName2[100];
		sprintf(fileName2, "%s_%g_%s.txt", &string_tmp,Calibration_Impedance,outFormatTime);
		FILE *fout2 = fopen(fileName2,"w");

		signed short RealPart = 0;
		signed short ImagPart = 0;

		fprintf(gnuplot, "plot '-' with lines\n");
		
		// While sweep is not complete...
		/*while (1)
		{
			magnitude = AD5933_CalculateImpedance(gainFactor,
								  AD5933_FUNCTION_REPEAT_FREQ);
			
			Z_MOD[freq_iter] = (1/(gainFactor*magnitude));
			CurrentFrequency = START_FREQ + INCREMENT_FREQ*freq_iter;
			
			printf("Impedance read: %f ohms (@ %lu Hz)\n\r", Z_MOD[freq_iter] , CurrentFrequency);
			
			
			
		}*/

		fprintf(fout2,"P.Real\tP.Imag\timpedance\tphase\tfrequency\n");

		i = 0;
		for (i = 0; i < NPOINTS; i++)
		{
			//TEMPERATURE
			//TEMPERATURE = AD5933_GetTemperature();
			
			// Calculate impedance between Vout and Vin
			// Linea original
			// magnitude = AD5933_CalculateImpedance(gainFactor, AD5933_FUNCTION_REPEAT_FREQ);
			// Linea que obtiene tambien los datos parte Real/Imaginaria
			magnitude = AD5933_CalculateImpedanceV3(gainFactor, AD5933_FUNCTION_REPEAT_FREQ, &RealPart, &ImagPart, &phase);

			//Z_MOD[i] = magnitude;
			//Z_MOD[freq_iter] = 1/magnitude;
			impedance = (1/(gainFactor*magnitude));
			Z_MOD[freq_iter] = impedance;
			CurrentFrequency = START_FREQ + INCREMENT_FREQ*freq_iter;
			
			// Print impedance
			printf("Impedance read: %f ohms (phase: %f) (@ %lu Hz) (dif: %f)\n\r", impedance, phase-SystemPhase, CurrentFrequency,(impedance/Calibration_Impedance-1)*100);	 				  
			// printf("TEMPERATURE: %lu\n",TEMPERATURE);
			fprintf(fout,"%f\t%f\t%lu\n",impedance,phase,CurrentFrequency);
			fprintf(fout2,"%d\t%d\t%f\t%f\t%lu\n",RealPart,ImagPart,impedance, phase-SystemPhase,CurrentFrequency);
			fprintf(gnuplot, "%lu %f\n",CurrentFrequency,impedance);
			
			fflush( fout );
			fflush( fout2 );
			fflush( gnuplot );

			status = AD5933_GetRegisterValue(AD5933_REG_STATUS,1);
			freq_iter += 1;
		}
		/*while ((status & AD5933_STAT_SWEEP_DONE) == 0)
		{
			
			//TEMPERATURE
			TEMPERATURE = AD5933_GetTemperature();
			
			// Calculate impedance between Vout and Vin
			magnitude = AD5933_CalculateImpedance(gainFactor,
								  AD5933_FUNCTION_REPEAT_FREQ);
								  
			Z_MOD[i] = magnitude;
			//Z_MOD[freq_iter] = 1/magnitude;
			//Z_MOD[freq_iter] = (1/(gainFactor*magnitude));
			CurrentFrequency = START_FREQ + INCREMENT_FREQ*freq_iter;
			
			// Print impedance
			printf("Impedance read: %f ohms (@ %lu Hz)\n\r", magnitude, CurrentFrequency);					  
			printf("TEMPERATURE: %lu\n",TEMPERATURE);
			fprintf(fout,"%f\t%lu\n",magnitude,CurrentFrequency);
			
			fprintf(gnuplot, "%lu %f\n",CurrentFrequency,magnitude);
			
			
			status = AD5933_GetRegisterValue(AD5933_REG_STATUS,1);
			freq_iter += 1;
			i += 1;
			
			
			//getchar();
			
			
		   
	   }*/
	   
	   
	   fprintf(gnuplot, "e\n");
	   fflush(gnuplot);
        
       fclose(fout);
	   fclose(fout2); // archivo extendido
        
       // Write array to txt file
	   //FILE* fout = fopen("out.txt","w");
	   //for (i=0; i<NPOINTS;i++)
	   //{
		   //fprintf(fout,"%f\n",Z[i]);
	   //}
	   //fclose(fout);

		double temperatureFinal = 0;
        printf("\n Obteniendo Temperatura : \n");
		temperatureFinal = AD5933_GetTemperatureV2();
        printf("Temperature Inicial    : %f\n",temperatureInicial);
        printf("Temperature Final      : %f\n",temperatureFinal);
        printf("Temperature Diferencia : %f (%f)\n",temperatureFinal - temperatureInicial,(temperatureFinal - temperatureInicial)/temperatureInicial);
	   
	   return 0; 
        
        
        
        
			   
		   ////Demo Program based on https://github.com/analogdevicesinc/no-OS/blob/master/Pmods/PmodIA/AD5933.c
		   
		   //unsigned long  impedanceKohms  = 0;
		   //unsigned long  impedanceOhms   = 0;
		   ////float          impedance       = 0.0f;
		   
		   
		   //unsigned long START_FREQ = 3000; // in Hertz (unsigned long has 32 bits, from which we will only need 24)
		   //unsigned long INCREMENT_FREQ = 1500; // in Hertz
		   //unsigned short NPOINTS = 1000;
		   //unsigned long f = START_FREQ;
		   //float Z[NPOINTS];
		   
		   ////// Set Range and Gain
		   //currentRange = AD5933_RANGE_2000mVpp;
		   //currentGain = AD5933_GAIN_X1;
		   //printf("Setting Range to %d and Gain to %d\n",currentRange,currentGain);
		   //AD5933_SetRangeAndGain(currentRange,currentGain);
		   
		   //// Get temperature data (just to know how to get it)
		   //TEMPERATURE = AD5933_GetTemperature();
		   //printf(" Temperature: %lu\n",TEMPERATURE);
		   
		   //// Configure sweep
		   //AD5933_ConfigSweep(START_FREQ, INCREMENT_FREQ, NPOINTS);
			   
		   //// Start the sweep
		   //AD5933_StartSweep();
		   
		   //// Calculate gain factor for an impedance of 98kohms
		   //gainFactor = AD5933_CalculateGainFactor(AD5933_CALIBRATION_IMPEDANCE,
									//AD5933_FUNCTION_REPEAT_FREQ);



			//// Change the resistor used for calibration with the one you wish to measure
			//printf("Gain Factor: %f\n",gainFactor);
			//printf("Replace calibration component with desired one for measurement and press any key\n\r");
			//// wait for user input
			//getchar();
			
			//status = AD5933_GetRegisterValue(AD5933_REG_STATUS,1);
			
			
			//FILE* fout = fopen("out.txt","w");
			//FILE *gnuplot = popen("gnuplot -persistent", "w");
			//fprintf(gnuplot, "plot '-' with lines\n");
			//while ((status & AD5933_STAT_SWEEP_DONE) == 0)
		   //{
				
				//// Calculate impedance between Vout and Vin
				//impedance = AD5933_CalculateImpedance(gainFactor,
									  //AD5933_FUNCTION_INC_FREQ);
									  
				//impedanceOhms = (unsigned long)impedance;



				//// Get real and imaginary reg parts
				//signed short RealPart = 0;
				//signed short ImagPart = 0;
				//unsigned char byte          = 0;
				//int tmp = 0;
				
				//unsigned char registerAddress = AD5933_REG_REAL_DATA;
				//for(byte = 0;byte < 2;byte ++)
				//{
					//// Read byte from specified registerAddress memory place
					//tmp = wiringPiI2CReadReg8(i2cdevice,registerAddress);
					//printf("\t\tReading from Register Address: 0x%02x...0x%02x\n",registerAddress,tmp);
					//// Add this temporal value to our registerValue (remembering that
					//// we are reading bytes that have location value, which means that
					//// each measure we have we not only have to add it to the previous
					//// register value but we also but do a bitwise shift (<< 8) by 1 byte
					//RealPart = RealPart << 8;
					//RealPart += tmp;
					//// Update value from registerAddress to read next memory position byte
					//registerAddress = registerAddress + 1;
				//}
				
				//printf("Read Real: %hi\n",RealPart);
				
				//registerAddress = AD5933_REG_IMAG_DATA;
				//for(byte = 0;byte < 2;byte ++)
				//{
					//// Read byte from specified registerAddress memory place
					//tmp = wiringPiI2CReadReg8(i2cdevice,registerAddress);
					//printf("\t\tReading from Register Address: 0x%02x...0x%02x\n",registerAddress,tmp);
					//// Add this temporal value to our registerValue (remembering that
					//// we are reading bytes that have location value, which means that
					//// each measure we have we not only have to add it to the previous
					//// register value but we also but do a bitwise shift (<< 8) by 1 byte
					//ImagPart = ImagPart << 8;
					//ImagPart += tmp;
					//// Update value from registerAddress to read next memory position byte
					//registerAddress = registerAddress + 1;
				//}
				
				//printf("Read Imag: %hi\n",ImagPart);
				
				////float magnitude = 0;
				
				//magnitude = sqrtf((RealPart * RealPart) + (ImagPart * ImagPart));
				//printf("Magnitude: %f\n",magnitude);
				
				
				
				////Z[freq_iter] = 1/(gainFactor*magnitude);
				//Z[freq_iter] = magnitude;
				//f = START_FREQ + INCREMENT_FREQ*freq_iter;
				
				//// Print impedance
				//printf("Impedance read: %f kohms (@ %lu Hz)\n\r", Z[freq_iter], f);					  
				//fprintf(fout,"%f\t%lu\n",Z[freq_iter],f);
				
				//fprintf(gnuplot, "%lu %f\n",f,Z[freq_iter]);
				
				
				//status = AD5933_GetRegisterValue(AD5933_REG_STATUS,1);
				//freq_iter += 1;
				
				
				
				////getchar();
				
				
			   
		   //}
		   
		   //fclose(fout);
		   //fprintf(gnuplot, "e\n");
		   //fflush(gnuplot);

		   
		   
		   // Write array to txt file
		   //FILE* fout = fopen("out.txt","w");
		   //for (i=0; i<NPOINTS;i++)
		   //{
			   //fprintf(fout,"%f\n",Z[i]);
		   //}
		   //fclose(fout);
		   
		   return 0;
		   

}
