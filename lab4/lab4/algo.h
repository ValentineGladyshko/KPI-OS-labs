#pragma once
#include <random>
#include <algorithm>
#include <vector>
#include <iostream>
#include <time.h> 


using namespace std;

void shedulerWork();
void generateMatrix(vector<vector<int>>& matrix, vector<int>& row, vector<int>& column, int size);
void showMatrix(vector<vector<int>> matrix, vector<int> row, vector<int> column);
void createVectorTask(vector<int>& vec, vector<vector<int>> matrix);
void createVectorResourse(vector<int>& vec, vector<vector<int>> matrix);
void changeVector(vector<int>& vec, int i, int j);
int findMin(vector<int> vec, int size);
int findMax(vector<int> vec, vector<vector<int>> matrix, vector<int>& row, vector<int>& column, int size);
void algo(vector<vector<int>>& matrix, vector<int>& row, vector<int>& column,
	vector<int>& vectorTask, vector<int>& vectorResourse, long long& time, bool show);
void show(vector<int> vec);