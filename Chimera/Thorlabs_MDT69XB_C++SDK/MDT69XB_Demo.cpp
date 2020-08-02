// SDKSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <Windows.h>
#include <assert.h>
using namespace std;

#define BUFFER_SIZE 256

typedef int (*MDT69X_List)(unsigned char *serialNo, unsigned int length);
typedef int (*MDT69X_Open)(unsigned char* serialNo, int nBaud, int timeout);
typedef int (*MDT69X_Close)(int hdl);

typedef int (*MDT69X_GetId)(int hdl, unsigned char* id);

typedef int (*MDT69X_SetXAxisVoltage)(int hdl, double voltage);
typedef int (*MDT69X_SetXAxisMinVoltage)(int hdl, double voltage);
typedef int (*MDT69X_SetXAxisMaxVoltage)(int hdl, double voltage);
typedef int (*MDT69X_GetXAxisVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetXAxisMinVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetXAxisMaxVoltage)(int hdl, double * voltage);

typedef int (*MDT69X_SetYAxisVoltage)(int hdl, double voltage);
typedef int (*MDT69X_SetYAxisMinVoltage)(int hdl, double voltage);
typedef int (*MDT69X_SetYAxisMaxVoltage)(int hdl, double voltage);
typedef int (*MDT69X_GetYAxisVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetYAxisMinVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetYAxisMaxVoltage)(int hdl, double * voltage);

typedef int (*MDT69X_SetZAxisVoltage)(int hdl, double voltage);
typedef int (*MDT69X_SetZAxisMinVoltage)(int hdl, double voltage);
typedef int (*MDT69X_SetZAxisMaxVoltage)(int hdl, double voltage);
typedef int (*MDT69X_GetZAxisVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetZAxisMinVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetZAxisMaxVoltage)(int hdl, double * voltage);

typedef int (*MDT69X_SetXYZAxisVoltage)(int hdl, double xVoltage, double yVoltage, double zVoltage);
typedef int (*MDT69X_GetXYZAxisVoltage)(int hdl, double * xVoltage, double * yVoltage, double * zVoltage);

typedef int (*MDT69X_SetMasterScanEnable)(int hdl, int enable);
typedef int (*MDT69X_GetMasterScanEnable)(int hdl, int * enable);
typedef int (*MDT69X_SetMasterScanVoltage)(int hdl, double voltage);
typedef int (*MDT69X_GetMasterScanVoltage)(int hdl, double * voltage);
typedef int (*MDT69X_GetLimitVoltage)(int hdl, double * voltage);

typedef int (*MDT69X_SetAllVoltage)(int hdl, double voltage);

MDT69X_List List; 
MDT69X_Open Open;
MDT69X_Close Close;

MDT69X_GetId GetId;

MDT69X_SetXAxisVoltage SetXAxisVoltage;
MDT69X_SetXAxisMaxVoltage SetXAxisMinVoltage;
MDT69X_SetXAxisMaxVoltage SetXAxisMaxVoltage;
MDT69X_GetXAxisVoltage GetXAxisVoltage;
MDT69X_GetXAxisMaxVoltage GetXAxisMinVoltage;
MDT69X_GetXAxisMaxVoltage GetXAxisMaxVoltage;

MDT69X_SetYAxisVoltage SetYAxisVoltage;
MDT69X_SetYAxisMaxVoltage SetYAxisMinVoltage;
MDT69X_SetYAxisMaxVoltage SetYAxisMaxVoltage;
MDT69X_GetYAxisVoltage GetYAxisVoltage;
MDT69X_GetYAxisMaxVoltage GetYAxisMinVoltage;
MDT69X_GetYAxisMaxVoltage GetYAxisMaxVoltage;

MDT69X_SetZAxisVoltage SetZAxisVoltage;
MDT69X_SetZAxisMaxVoltage SetZAxisMinVoltage;
MDT69X_SetZAxisMaxVoltage SetZAxisMaxVoltage;
MDT69X_GetZAxisVoltage GetZAxisVoltage;
MDT69X_GetZAxisMaxVoltage GetZAxisMinVoltage;
MDT69X_GetZAxisMaxVoltage GetZAxisMaxVoltage;

MDT69X_SetXYZAxisVoltage SetXYZAxisVoltage;
MDT69X_GetXYZAxisVoltage GetXYZAxisVoltage;

MDT69X_SetMasterScanEnable SetMasterScanEnable;
MDT69X_GetMasterScanEnable GetMasterScanEnable;
MDT69X_SetMasterScanVoltage SetMasterScanVoltage;
MDT69X_GetMasterScanVoltage GetMasterScanVoltage;
MDT69X_GetLimitVoltage GetLimitVoltage;
MDT69X_SetAllVoltage SetAllVoltage;


bool Init()
{
	HINSTANCE hdll = LoadLibrary(TEXT("MDT_COMMAND_LIB_x64.dll"));
	if(hdll == NULL)
		return false;

	List = (MDT69X_List)GetProcAddress(hdll, "List");
	Open = (MDT69X_Open)GetProcAddress(hdll, "Open");
	Close = (MDT69X_Close)GetProcAddress(hdll, "Close");
	GetId = (MDT69X_GetId)GetProcAddress(hdll, "GetId");

	SetXAxisVoltage = (MDT69X_SetXAxisVoltage)GetProcAddress(hdll, "SetXAxisVoltage");
	SetXAxisMinVoltage = (MDT69X_SetXAxisMinVoltage)GetProcAddress(hdll, "SetXAxisMinVoltage");
	SetXAxisMaxVoltage = (MDT69X_SetXAxisMaxVoltage)GetProcAddress(hdll, "SetXAxisMaxVoltage");
	GetXAxisVoltage = (MDT69X_GetXAxisVoltage)GetProcAddress(hdll, "GetXAxisVoltage");
	GetXAxisMinVoltage = (MDT69X_GetXAxisMinVoltage)GetProcAddress(hdll, "GetXAxisMinVoltage");
	GetXAxisMaxVoltage = (MDT69X_GetXAxisMaxVoltage)GetProcAddress(hdll, "GetXAxisMaxVoltage");

	SetYAxisVoltage = (MDT69X_SetYAxisVoltage)GetProcAddress(hdll, "SetYAxisVoltage");
	SetYAxisMinVoltage = (MDT69X_SetYAxisMinVoltage)GetProcAddress(hdll, "SetYAxisMinVoltage");
	SetYAxisMaxVoltage = (MDT69X_SetYAxisMaxVoltage)GetProcAddress(hdll, "SetYAxisMaxVoltage");
	GetYAxisVoltage = (MDT69X_GetYAxisVoltage)GetProcAddress(hdll, "GetYAxisVoltage");
	GetYAxisMinVoltage = (MDT69X_GetYAxisMinVoltage)GetProcAddress(hdll, "GetYAxisMinVoltage");
	GetYAxisMaxVoltage = (MDT69X_GetYAxisMaxVoltage)GetProcAddress(hdll, "GetYAxisMaxVoltage");

	SetZAxisVoltage = (MDT69X_SetZAxisVoltage)GetProcAddress(hdll, "SetZAxisVoltage");
	SetZAxisMinVoltage = (MDT69X_SetZAxisMinVoltage)GetProcAddress(hdll, "SetZAxisMinVoltage");
	SetZAxisMaxVoltage = (MDT69X_SetZAxisMaxVoltage)GetProcAddress(hdll, "SetZAxisMaxVoltage");
	GetZAxisVoltage = (MDT69X_GetZAxisVoltage)GetProcAddress(hdll, "GetZAxisVoltage");
	GetZAxisMinVoltage = (MDT69X_GetZAxisMinVoltage)GetProcAddress(hdll, "GetZAxisMinVoltage");
	GetZAxisMaxVoltage = (MDT69X_GetZAxisMaxVoltage)GetProcAddress(hdll, "GetZAxisMaxVoltage");

	SetXYZAxisVoltage = (MDT69X_SetXYZAxisVoltage)GetProcAddress(hdll, "SetXYZAxisVoltage");
	GetXYZAxisVoltage = (MDT69X_GetXYZAxisVoltage)GetProcAddress(hdll, "GetXYZAxisVoltage");
	SetMasterScanEnable = (MDT69X_SetMasterScanEnable)GetProcAddress(hdll, "SetMasterScanEnable");
	GetMasterScanEnable = (MDT69X_GetMasterScanEnable)GetProcAddress(hdll, "GetMasterScanEnable");

	SetMasterScanVoltage = (MDT69X_SetMasterScanVoltage)GetProcAddress(hdll, "SetMasterScanVoltage");
	GetMasterScanVoltage = (MDT69X_GetMasterScanVoltage)GetProcAddress(hdll, "GetMasterScanVoltage");
	

	GetLimitVoltage = (MDT69X_GetLimitVoltage)GetProcAddress(hdll, "GetLimitVoltage");
	SetAllVoltage = (MDT69X_SetAllVoltage)GetProcAddress(hdll, "SetAllVoltage");

	if(List != NULL && Open != NULL && Close != NULL && GetId != NULL &&
		SetXAxisVoltage != NULL && SetXAxisMinVoltage != NULL && SetXAxisMaxVoltage != NULL && 
		GetXAxisVoltage != NULL && GetXAxisMinVoltage!= NULL && GetXAxisMaxVoltage != NULL &&
		SetYAxisVoltage != NULL && SetYAxisMinVoltage != NULL && SetYAxisMaxVoltage != NULL && 
		GetYAxisVoltage != NULL && GetYAxisMinVoltage!= NULL && GetYAxisMaxVoltage != NULL && 
		SetZAxisVoltage != NULL && SetZAxisMinVoltage != NULL && SetZAxisMaxVoltage != NULL && 
		GetZAxisVoltage != NULL && GetZAxisMinVoltage!= NULL && GetZAxisMaxVoltage != NULL && SetAllVoltage != NULL &&
		SetXYZAxisVoltage != NULL && GetXYZAxisVoltage != NULL && SetMasterScanEnable != NULL && GetMasterScanEnable != NULL && GetLimitVoltage != NULL)
		return true;
	else return false;
}

void TestMDT693B(int hdl)
{
	printf("\n0: Set X-AXIS voltage	\n"
	       "1: Set X-Y-Z AXISES voltages \n"
	       "2: Get X-AXIS voltage \n"
	       "3: Get X-Y-Z AXISES voltages \n"
	       "4: Set X-AXIS min voltage \n"
	       "5: Set X-AXIS max voltage \n"
		   "6: Get X-AXIS min voltage \n"
           "7: Get X-AXIS max voltage \n"
		   "8: Set all voltage \n"
	       "9: Set master scan enable\n"
           "A: Set master scan voltage \n"
		   "B: Get master scan voltage \n"
	       "(press 'x' to quit)\n");
	char inputChart = 'e';
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

	double minX, maxX; 
	GetXAxisMinVoltage(hdl, &minX); 
	GetXAxisMaxVoltage(hdl, &maxX);

	double minY, maxY; 
	GetYAxisMinVoltage(hdl, &minY); 
	GetYAxisMaxVoltage(hdl, &maxY);

	double minZ, maxZ; 
	GetZAxisMinVoltage(hdl, &minZ); 
	GetZAxisMaxVoltage(hdl, &maxZ);

	double limtVoltage;
	GetLimitVoltage(hdl, &limtVoltage);

	double xVol, yVol, zVol;
	while(inputChart != 'x')
	{
		inputChart = getchar();
		switch (inputChart)
		{
		case '0':
			printf("\nPlease input a vaild voltage to X-AXIS (%.2f to %.2f): \n", minX, maxX);
			scanf_s("%s", buffer, sizeof(buffer));
			xVol = atof(buffer);
			if(xVol < minX || xVol > maxX)
			{
				printf("\nNot a vaild voltage! (%.2f to %.2f)\n", minX, maxX);
			}
			else
			{
				SetXAxisVoltage(hdl, xVol);
			}
			break;
		case '1':
			{
				printf("\nPlease input voltages to X,Y,Z-AXIS, use comma between each axis voltage. (Example: 12.0,14.35,12.89) : \n");
		        scanf_s("%s", buffer, sizeof(buffer));
		         
		        string str(buffer);
		        istringstream ss(str);
		        string token;
		        getline(ss, token, ',');
		        xVol = -1.0; yVol = -1.0; zVol = -1.0;
		        xVol = atof(token.c_str());
		        getline(ss, token, ',');
		        yVol = atof(token.c_str());
		        getline(ss, token, ',');
		        zVol = atof(token.c_str());
		        
		        if(xVol < minX || xVol > maxX || yVol < minY || yVol > maxY || zVol < minZ || zVol > maxZ)
		        {
		        	printf("\nNot vaild voltages! \n");
		        }
		        else
		        {
		        	SetXYZAxisVoltage(hdl, xVol, yVol, zVol);
		        }
		        break;
			}
		case '2':
			{
			    GetXAxisVoltage(hdl, &xVol);
			    printf("\nThe X-AXIS voltage is %.2f\n", xVol);
			    break;
			}
		case '3':
			{
				GetXYZAxisVoltage(hdl, &xVol, &yVol, &zVol);
			    printf("\nThe X-AXIS voltage is %.2f, the Y-AXIS voltage is %.2f, the Z-AXIS voltage is %.2f\n" , xVol, yVol, zVol);
			    break;
			}
		case '4':
			{
			    printf("\nPlease input a vaild min voltage to X-AXIS (0.00 to %.2f): \n", limtVoltage);
			    scanf_s("%s", buffer, sizeof(buffer));
			    double tempMin = atof(buffer);
			    if(tempMin < 0 || tempMin > limtVoltage)
			    {
			    	printf("\nNot a vaild voltage! (0.00 to %.2f)\n", limtVoltage);
			    }
			    else
			    {
			    	SetXAxisMinVoltage(hdl, tempMin);
			    	minX = tempMin;
			    }
			    break;
			}
		case '5':
			{
			    printf("\nPlease input a vaild max voltage to X-AXIS (%.2f to %.2f): \n", minX, limtVoltage);
			    scanf_s("%s", buffer, sizeof(buffer));
			    double tempMax = atof(buffer);
			    if(tempMax < minX || tempMax > limtVoltage)
			    {
			    	printf("\nNot a vaild voltage! (%.2f to %.2f) \n", minX, limtVoltage);
			    }
			    else
			    {
			    	SetXAxisMaxVoltage(hdl, tempMax);
			    	maxX = tempMax;
			    }
			    break;
			}
		case '6':
			{
			    GetXAxisMinVoltage(hdl, &minX);
			    printf("\nThe X-AXIS min voltage is %.2f\n", minX);
			    break;
			}
		case '7':
			{
			    GetXAxisMaxVoltage(hdl, &maxX);
			    printf("\nThe X-AXIS max voltage is %.2f\n" , maxX);
			    break;
			}
			
        case '9':
			{
			    printf("\nPlease set master scan mode (0-Disable,1-Enable): \n");
			    scanf_s("%s", buffer, sizeof(buffer));
			    int enable = atoi(buffer);
			    if(enable<0 || enable>1)
			    {
			    	printf("\nNot a vaild master scan mode! (0-Disable,1-Enable) \n");
			    }
			    else
			    {
			    	SetMasterScanEnable(hdl, enable);
			    }
			    break;
			}
		case '8':
			{
			    printf("\nPlease input a vaild voltage to X,Y,Z-AXIS 0.00 to %.2f): \n", limtVoltage);
			    scanf_s("%s", buffer, sizeof(buffer));
			    double tempVol = atof(buffer);
			    if(tempVol < 0 || tempVol > limtVoltage)
			    {
			    	printf("\nNot a vaild voltage! (0.00 to %.2f)\n", tempVol);
			    }
			    else
			    {
			    	SetAllVoltage(hdl, tempVol);
			    }
			    break;
			}
		case 'A':
			{
			    printf("\nPlease input a vaild master scan voltage  0.00 to %.2f): \n", limtVoltage);
			    scanf_s("%s", buffer, sizeof(buffer));
			    double tempVol = atof(buffer);
			    if(tempVol < 0 || tempVol > limtVoltage)
			    {
			    	printf("\nNot a vaild voltage! (0.00 to %.2f)\n", tempVol);
			    }
			    else
			    {
					SetMasterScanVoltage(hdl, tempVol);
			    }
			    break;
			}
		case 'B':
			{
				GetMasterScanVoltage(hdl, &xVol);
			    printf("\nThe master scan voltage is %.2f\n" , xVol);
			    break;
			}
		default:
			break;
		}
	}

	printf("Please select to continue:\n");

}

void TestMDT694B(int hdl)
{
	printf("\n0: Set X-AXIS voltage	\n"
	       "1: Get X-AXIS voltage \n"
	       "2: Set X-AXIS min voltage \n"
	       "3: Set X-AXIS max voltage \n"
		   "4: Get X-AXIS min voltage \n"
           "5: Get X-AXIS max voltage \n"
	       "(press 'x' to quit)\n");
	char inputChart = 'e';
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

	double minX, maxX; 
	GetXAxisMinVoltage(hdl, &minX); 
	GetXAxisMaxVoltage(hdl, &maxX);

	double minY, maxY; 
	GetYAxisMinVoltage(hdl, &minY); 
	GetYAxisMaxVoltage(hdl, &maxY);

	double minZ, maxZ; 
	GetZAxisMinVoltage(hdl, &minZ); 
	GetZAxisMaxVoltage(hdl, &maxZ);

	double limtVoltage;
	GetLimitVoltage(hdl, &limtVoltage);

	double xVol;
	while(inputChart != 'x')
	{
		inputChart = getchar();
		switch (inputChart)
		{
		case '0':
			printf("\nPlease input a vaild voltage to X-AXIS (%.2f to %.2f): \n", minX, maxX);
			scanf_s("%s", buffer, sizeof(buffer));
			xVol = atof(buffer);
			if(xVol < minX || xVol > maxX)
			{
				printf("\nNot a vaild voltage! (%.2f to %.2f)\n", minX, maxX);
			}
			else
			{
				SetXAxisVoltage(hdl, xVol);
			}
			break;
		case '1':
			{
			    GetXAxisVoltage(hdl, &xVol);
			    printf("\nThe X-AXIS voltage is %.2f\n", xVol);
			    break;
			}
		case '2':
			{
			    printf("\nPlease input a vaild min voltage to X-AXIS (0.00 to %.2f): \n", limtVoltage);
			    scanf_s("%s", buffer, sizeof(buffer));
			    double tempMin = atof(buffer);
			    if(tempMin < 0 || tempMin > limtVoltage)
			    {
			    	printf("\nNot a vaild voltage! (0.00 to %.2f)\n", limtVoltage);
			    }
			    else
			    {
			    	SetXAxisMinVoltage(hdl, tempMin);
			    	minX = tempMin;
			    }
			    break;
			}
		case '3':
			{
			    printf("\nPlease input a vaild max voltage to X-AXIS (%.2f to %.2f): \n", minX, limtVoltage);
			    scanf_s("%s", buffer, sizeof(buffer));
			    double tempMax = atof(buffer);
			    if(tempMax < minX || tempMax > limtVoltage)
			    {
			    	printf("\nNot a vaild voltage! (%.2f to %.2f)\n", minX, limtVoltage);
			    }
			    else
			    {
			    	SetXAxisMaxVoltage(hdl, tempMax);
			    	maxX = tempMax;
			    }
			    break;
			}
		case '4':
			{
			    GetXAxisMinVoltage(hdl, &minX);
			    printf("\nThe X-AXIS min voltage is %.2f\n", minX);
			    break;
			}
		case '5':
			{
			    GetXAxisMaxVoltage(hdl, &maxX);
			    printf("\nThe X-AXIS max voltage is %.2f\n" , maxX);
			    break;
			}
		default:
			break;
		}
	}

	printf("Please select to continue:\n");
}


int _tmain(int argc, _TCHAR* argv[])
{
	if(!Init()) return 0;

	unsigned char sn[BUFFER_SIZE];
	unsigned char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	List(buffer, BUFFER_SIZE);
	printf("%s.\n", buffer);
	printf("Please input com port number.\n");
	scanf_s("%s", sn, sizeof(sn));
	int hdl = Open(sn, 115200, 10);
	
	if (hdl < 0)
	{
		printf("com port fails, please check the driver installed correctly\n");
	}
	else
	{
		unsigned char id[BUFFER_SIZE];
		memset(id, 0, sizeof(id));
		GetId(hdl, id);
		
		double voltage = 0.0;
		GetLimitVoltage(hdl, &voltage);

		if(strstr(reinterpret_cast<char*>(id), "MDT693B") != NULL)
		{
			printf("\n Device: MDT693B ");
			printf("\n Limit voltage: %.1f\n", voltage);
			TestMDT693B(hdl);
		}
		else
		{
			printf("\n Device: MDT694B ");
			printf("\n Limit voltage: %.1f\n", voltage);
			TestMDT694B(hdl);
		}
	}
	return 0;
}