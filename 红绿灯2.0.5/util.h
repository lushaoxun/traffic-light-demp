#pragma once

#include<iostream>
#include <vector>
#include<Python.h>
using namespace std;

void initPythonModule();
void destroyPythonMoudle();
void getImgCircles(char * imgPath, vector<vector<int>> &circles, int h4, 
	int h5, int h6, int h7, int h8, int h9);

double calIntensity(char * path);

void getJustCurrentDir(string path, vector<string>& files);
void getJustCurrentFile( string path, vector<string>& files);

void myExec(bool isDay, const char * imgPath);