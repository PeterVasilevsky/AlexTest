//
//  main.cpp
//  AlexTest
//
//  Created by Петр Василевский on 08.11.12.
//  Copyright (c) 2012 Петр Василевский. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>
#include <vector>

const float CONST_G = 9.780327;      //ускорение свободного падения
const int RULER_COUNT = 3;           //количество изменерий реакции линейкой

const int MIN_LOWER_PRESSURE = 60;  //минимальное нижнее давление
const int MAX_LOWER_PRESSURE = 90;  //максимальное нижнее давление
const int MIN_UPPER_PRESSURE = 100; //минимальное верхнее давление
const int MAX_UPPER_PRESSURE = 140; //максимальное верхнее давление

const int TIME_STEP = 10;           //промежуток времени для усреднения. Значения должны быть кратны 5, это важно
const int SCALE_DIMENSION = 20;     //количество значений по оси у

//структура "давление"
typedef struct {
    int lower;      //нижнее
    int upper;      //верхнее
} arterialPressure;

//объявляем структуру, храняющую данные о водителе
typedef struct {
    char name[10];                      //имя водителя (макс. 10 символов)
    int experience;                     //стаж
    float averageReaction;              //среднее время реакции
    arterialPressure startPressure;     //начальное давление
    arterialPressure endPressure;       //конечное давление
    
    std::vector<int> distanceVector;
    std::vector<int> pulseVectior;
    std::vector<float> averageDistanceVector;
    std::vector<float> averagePulseVector;
} driver;

int nextNumber(std::ifstream *inFile);                  //запрашивает следующее число из файла
void askReaction(driver *aDriver);                      //запрос реакции водителя
void driverInit(driver *aDriver);                       //запрос имени, опыта и реакции водителя
bool askStartPressure(driver *aDriver);                 //запрос начального АД
bool askEndPressure(driver *aDriver);                   //запрос конечного АД
bool readDistanceAndPulseFiles(driver *aDriver, char *distanceFilePath, char *pulseFilePath);        //считывание значений из файлов дистанции и пульса
void printChart(driver *aDriver);                       //построить и распечатать график
void writeToFile(driver* aDriver, char* filePath);      //записать в файл данные о водителе

int main(int argc, const char * argv[])
{
    //создаем нового водителя
    driver *aDriver = new driver;
    driverInit(aDriver);
    if (!askStartPressure(aDriver)) {
        printf("Access denied for driver. Reason: B.P.\n");
    }
//    readDistanceAndPulseFiles(aDriver);
    readDistanceAndPulseFiles(aDriver, "/users/zeelony/desktop/distance.txt", "/users/zeelony/desktop/pulse.txt");
    printChart(aDriver);
    if (!askEndPressure(aDriver)) {
        printf("Access denied for driver. Reason: B.P.\n");
    }
    writeToFile(aDriver, "/users/zeelony/desktop/out.txt");
    
    printf("done\n");
    return 0;
}

int nextNumber(std::ifstream *inFile)
{
    char stringValue[4];
    char ch = 0;
    int i = 0;
    
    //пропускаем лишнее
    do {
        ch = inFile->get();
        if (!inFile->good())
            return NULL;
    } while (ch < '0' || ch > '9');
    
    //считываем число
    while (ch >= '0' && ch <= '9') {
        stringValue[i] = ch;
        stringValue[i+1] = '\0';
        ch = inFile->get();
        i++;
    }
    
    return atoi(stringValue);
}

void askReaction(driver *aDriver)
{
    float rulerSum = 0;
    printf("Enter %i ruler values to calculate driver's average reaction\n", RULER_COUNT);
    float currentRulerValue;
    
    for (int i=0; i<RULER_COUNT; i++) {
        printf("%i) ", i+1);
        std::cin >> currentRulerValue;
        rulerSum += currentRulerValue;
    }
    
    aDriver->averageReaction = sqrt((rulerSum / 3) * 2 / (CONST_G * 100));
    printf("Average driver's reaction is %f\n", aDriver->averageReaction);
    
}

void driverInit(driver *aDriver)
{
    //запрашиваем имя водителя
    printf("Name: ");
    std::cin >> aDriver->name;
    
    //запрашиваем стаж
    printf("Experience: ");
    std::cin >> aDriver->experience;
    
    //запрашиваем значения линейки и изменряем  среднюю реакцию водителя
    askReaction(aDriver);
}

bool askStartPressure(driver *aDriver)
{
    //запрашиваем начальное давление
    printf("Driver's B.P.:\nUpper: ");
    std::cin >> aDriver->startPressure.upper;
    printf("Lower: ");
    std::cin >> aDriver->startPressure.lower;
    if (aDriver->startPressure.lower < 60 || aDriver->startPressure.lower > 90 || aDriver->startPressure.upper < 100 || aDriver->startPressure.upper > 140) {
        return false;
    }
    return true;
}

bool askEndPressure(driver *aDriver)
{
    //запрашиваем начальное давление
    printf("Driver's B.P.:\nUpper: ");
    std::cin >> aDriver->endPressure.upper;
    printf("Lower: ");
    std::cin >> aDriver->endPressure.lower;
    if (aDriver->endPressure.lower < 60 || aDriver->endPressure.lower > 90 || aDriver->endPressure.upper < 100 || aDriver->endPressure.upper > 140) {
        return false;
    }
    return true;
}

bool readDistanceAndPulseFiles(driver *aDriver, char *distanceFilePath, char *pulseFilePath)
{
    std::ifstream inFile;
    inFile.open(distanceFilePath);
    if (!inFile) {
        printf("error opening distance.txt\n");
        return false;
    }
    
    int counter = 0;
    float averageDistance = 0;
    while (int distanceValue = nextNumber(&inFile)) {
        counter++;
        averageDistance += (float)distanceValue/TIME_STEP;
        if (counter == TIME_STEP) {
            aDriver->averageDistanceVector.push_back(averageDistance);
            counter = 0;
            averageDistance = 0;
        }
        aDriver->distanceVector.push_back(distanceValue);
    }
    
    inFile.close();
    
    inFile.open(pulseFilePath);
    if (!inFile) {
        printf("error opening pulse.txt\n");
        return false;
    }
    
    counter = 0;
    float averagePulse = 0;
    while (int pulseValue = nextNumber(&inFile)) {
        counter+=5;
        averagePulse += (float)pulseValue/(TIME_STEP/5);
        if (counter == TIME_STEP) {
            aDriver->averagePulseVector.push_back(averagePulse);
            counter = 0;
            averagePulse = 0;
        }
        aDriver->pulseVectior.push_back(pulseValue);
    }
    inFile.close();
    return true;
}

void printChart(driver *aDriver)
{
    //шкала для дистанции
    float minDistance = *std::min_element(aDriver->distanceVector.begin(), aDriver->distanceVector.end());
    float maxDistance = *std::max_element(aDriver->distanceVector.begin(), aDriver->distanceVector.end());
    float distanceStep = ceilf((maxDistance - minDistance + 1) / 20);
    int distanceScale[SCALE_DIMENSION];
    distanceScale[0] = minDistance;
    for (int i=1; i<SCALE_DIMENSION; i++) {
        distanceScale[i] = distanceScale[i-1] + distanceStep;
    }
    
    //шкала для пульса
    float minPulse = *std::min_element(aDriver->pulseVectior.begin(), aDriver->pulseVectior.end());
    float maxPulse = *std::max_element(aDriver->pulseVectior.begin(), aDriver->pulseVectior.end());
    float pulseStep = ceilf((maxPulse - minPulse + 1) / 20);
    int pulseScale[SCALE_DIMENSION];
    pulseScale[0] = minPulse;
    for (int i=1; i<SCALE_DIMENSION; i++) {
        pulseScale[i] = pulseScale[i-1] + pulseStep;
    }
    
    //массив для графика
	char ***chart;
	int secondDim = aDriver->averageDistanceVector.size();
	chart = new char**[SCALE_DIMENSION];
	for (int i = 0; i < SCALE_DIMENSION; i++)
		chart[i] = new char*[secondDim];
	for (int i = 0; i < SCALE_DIMENSION; i++)
		for (int j=0; j<secondDim; j++)
			chart[i][j] = new char[2];
    
    
	//const int a = aDriver->averageDistanceVector.size();
	//char chart[SCALE_DIMENSION][aDriver->averageDistanceVector.size()][2];
    
    for (int i=(SCALE_DIMENSION-1); i>=0; i--) {
        for (int j=0; j<(aDriver->averageDistanceVector.size()); j++) {
			chart[i][j][1] = ' ';
            if (aDriver->averageDistanceVector.at(j) >= distanceScale[i]) {
                if (i == (SCALE_DIMENSION - 1)) {
                    chart[i][j][0] = 'x';
				} else if (aDriver->averageDistanceVector.at(j) < distanceScale[i+1]) {
                    chart[i][j][0] = 'x';
                } else {
                    chart[i][j][0] = ' ';
                }
            } else {
                chart[i][j][0] = ' ';
            }
            
			if (j < aDriver->averagePulseVector.size())
			{
				if (aDriver->averagePulseVector.at(j) >= pulseScale[i]) {
					if (i == (SCALE_DIMENSION - 1)) {
						chart[i][j][1] = '*';
					} else if (aDriver->averagePulseVector.at(j) < pulseScale[i+1]) {
						chart[i][j][1] = '*';
					} else {
						chart[i][j][1] = ' ';
					}
				} else {
					chart[i][j][1] = ' ';
				}
			}
        }
    }
    
    printf("x\t*\t\n");
    for (int i=(SCALE_DIMENSION-1); i>=0; i--) {
        printf("%i\t%i\t", distanceScale[i], pulseScale[i]);
        for (int j=0; j<aDriver->averageDistanceVector.size(); j++) {
            printf("%c%c\t", chart[i][j][0], chart[i][j][1]);
        }
        printf("\n");
    }
    
    
    int xScaleValue = 0;
    printf("\t\t");
    for (int i=0; i<aDriver->averageDistanceVector.size(); i++) {
        printf("%i\t", xScaleValue);
        xScaleValue += TIME_STEP;
    }
    printf("\n");
    
}

void writeToFile(driver* aDriver, char* filePath)
{
    std::ofstream outFile;
    outFile.open(filePath);
    outFile << "Name:\t\t\t" << aDriver->name << std::endl;
    outFile << "Exprerience:\t\t" << aDriver->experience << std::endl;
    outFile << "Time of Reaction:\t" << aDriver->averageReaction << std::endl;
    outFile << "Start B.P.:\t\t" << aDriver->startPressure.upper << "/" << aDriver->startPressure.lower << std::endl;
    outFile << "End B.P.:\t\t" << aDriver->endPressure.upper << "/" << aDriver->endPressure.lower << std::endl;
    outFile.close();
}


