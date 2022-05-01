#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Flight.h"
#include "fileHelper.h"

void	initFlight(Flight* pFlight, const AirportManager* pManager)
{
	Airport* pPortOr = setAiportToFlight(pManager, "Enter name of origin airport:");
	pFlight->nameSource = _strdup(pPortOr->name);
	int same;
	Airport* pPortDes;
	do {
		pPortDes = setAiportToFlight(pManager, "Enter name of destination airport:");
		same = isSameAirport(pPortOr, pPortDes);
		if (same)
			printf("Same origin and destination airport\n");
	} while (same);
	pFlight->nameDest = _strdup(pPortDes->name);
	initPlane(&pFlight->thePlane);
	getCorrectDate(&pFlight->date);
}

int		isFlightFromSourceName(const Flight* pFlight, const char* nameSource)
{
	if (strcmp(pFlight->nameSource, nameSource) == 0)
		return 1;
		
	return 0;
}


int		isFlightToDestName(const Flight* pFlight, const char* nameDest)
{
	if (strcmp(pFlight->nameDest, nameDest) == 0)
		return 1;

	return 0;


}

int		isPlaneCodeInFlight(const Flight* pFlight, const char*  code)
{
	if (strcmp(pFlight->thePlane.code, code) == 0)
		return 1;
	return 0;
}

int		isPlaneTypeInFlight(const Flight* pFlight, ePlaneType type)
{
	if (pFlight->thePlane.type == type)
		return 1;
	return 0;
}


void	printFlight(const Flight* pFlight)
{
	printf("Flight From %s To %s\t",pFlight->nameSource, pFlight->nameDest);
	printDate(&pFlight->date);
	printPlane(&pFlight->thePlane);
}

void	printFlightV(const void* val)
{
	const Flight* pFlight = *(const Flight**)val;
	printFlight(pFlight);
}


Airport* setAiportToFlight(const AirportManager* pManager, const char* msg)
{
	char name[MAX_STR_LEN];
	Airport* port;
	do
	{
		printf("%s\t", msg);
		myGets(name, MAX_STR_LEN,stdin);
		port = findAirportByName(pManager, name);
		if (port == NULL)
			printf("No airport with this name - try again\n");
	} while(port == NULL);

	return port;
}

void	freeFlight(Flight* pFlight)
{
	free(pFlight->nameSource);
	free(pFlight->nameDest);
	free(pFlight);
}


int saveFlightToFile(const Flight* pF, FILE* fp)
{
	if (!writeStringToFile(pF->nameSource, fp, "Error write flight source name\n"))
		return 0;

	if (!writeStringToFile(pF->nameDest, fp, "Error write flight destination name\n"))
		return 0;

	if (!savePlaneToFile(&pF->thePlane,fp))
		return 0;

	if (!saveDateToFile(&pF->date,fp))
		return 0;

	return 1;
}


int loadFlightFromFile(Flight* pF, const AirportManager* pManager, FILE* fp)
{

	pF->nameSource = readStringFromFile(fp, "Error reading source name\n");
	if (!pF->nameSource)
		return 0;

	if (findAirportByName(pManager, pF->nameSource) == NULL)
	{
		printf("Airport %s not in manager\n", pF->nameSource);
		free(pF->nameSource);
		return 0;
	}

	pF->nameDest = readStringFromFile(fp, "Error reading destination name\n");
	if (!pF->nameDest)
	{
		free(pF->nameSource);
		return 0;
	}

	if (findAirportByName(pManager, pF->nameDest) == NULL)
	{
		printf("Airport %s not in manager\n", pF->nameDest);
		free(pF->nameSource);
		free(pF->nameDest);
		return 0;
	}

	if (!loadPlaneFromFile(&pF->thePlane, fp))
	{
		free(pF->nameSource);
		free(pF->nameDest);
		return 0;
	}


	if (!loadDateFromFile(&pF->date, fp))
	{
		free(pF->nameSource);
		free(pF->nameDest);
		return 0;
	}

	return 1;
}

int	compareFlightBySourceName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameSource, pFlight2->nameSource);
}

int	compareFlightByDestName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameDest, pFlight2->nameDest);
}

int	compareFlightByPlaneCode(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->thePlane.code, pFlight2->thePlane.code);
}

int		compareFlightByDate(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;


	return compareDate(&pFlight1->date, &pFlight2->date);
	

	return 0;
}

int saveToFlightCompressed(const Flight * pFlight, FILE * fp)
{
	BYTE data[6] = { 0 }; //init with zero's

	int lenSource = (int)strlen(pFlight->nameSource);
	int lenDest = (int)strlen(pFlight->nameDest);

	data[0] = (lenSource << 3) | (lenDest >> 2);
	data[1] = (lenDest << 6) | (pFlight->thePlane.type << 4) | (pFlight->date.month);


	int code[4];
	for (int i = 0; i < 4; i++)
		code[i] = pFlight->thePlane.code[i] - 'A';

	data[2] = (code[0] << 3) | (code[1] >> 2);
	data[3] = (code[1] << 6) | (code[2] << 1) | (code[3] >> 4);
	data[4] = (code[3] << 4) | (pFlight->date.year - 2021);
	data[5] = (pFlight->date.day & 0x1F);

	if (fwrite(&data, sizeof(BYTE), 6, fp) != 6)
		return 0;

	if (fwrite(pFlight->nameSource, sizeof(char), lenSource, fp) != lenSource)
		return 0;

	if (fwrite(pFlight->nameDest, sizeof(char), lenDest, fp) != lenDest)
		return 0;

	return 1;
}

int loadFlightCompressed(Flight * pFlight, const AirportManager * pManager, FILE * fp)
{
	BYTE data[6];

	if (fread(&data, sizeof(BYTE), 6, fp) != 6)
		return 0;

	int lenNameSource = ((data[0] >> 3) & 0x1F);
	int lenNameDest = ((data[0] << 2) & 0x1F) | ((data[1] >> 6) & 0x3);
	pFlight->thePlane.type = ((data[1] >> 4) & 0x3);
	pFlight->date.month = data[1] & 0xF;

	pFlight->thePlane.code[0] = ((data[2] >> 3) & 0x1F) + 'A';
	pFlight->thePlane.code[1] = ((data[2] << 2) & 0x1F) | ((data[3] >> 6) & 0x3) + 'A';
	pFlight->thePlane.code[2] = ((data[3] >> 1 & 0x1F)) + 'A';
	pFlight->thePlane.code[3] = ((data[3] << 4) & 0x1F) | ((data[4] >> 4) & 0xF) + 'A';
	pFlight->date.year = (data[4] & 0xF) + 2021;
	pFlight->date.day = data[5] & 0x1F;

	pFlight->nameSource = (char*)calloc((lenNameSource + 1), sizeof(char)); //+1 for '\0'
	if (!pFlight->nameSource)
		return 0;

	if (fread(pFlight->nameSource, sizeof(char), lenNameSource, fp) != lenNameSource)
	{
		free(pFlight->nameSource);
		return 0;
	}
	if (!findAirportByName(pManager, pFlight->nameSource))
	{
		printf("Airport %s not in manager\n", pFlight->nameSource);
		free(pFlight->nameSource);
		return 0;
	}


	pFlight->nameDest = (char*)calloc((lenNameDest + 1), sizeof(char));
	if (!pFlight->nameSource)
		return 0;

	if (fread(pFlight->nameDest, sizeof(char), lenNameDest, fp) != lenNameDest)
	{
		free(pFlight->nameSource);
		free(pFlight->nameDest);
		return 0;
	}
	if (!findAirportByName(pManager, pFlight->nameDest))	//maybe not necessary
	{
		printf("Airport %s not in manager\n", pFlight->nameSource);
		free(pFlight->nameSource);
		free(pFlight->nameDest);
		return 0;
	}

	return 1;
}

