#include "stdafx.h"
#include "algo.h"

using namespace std;

void shedulerWork()
{
	vector<vector<int>> matrix = vector<vector<int>>();
	vector<int> row = vector<int>();
	vector<int> column = vector<int>();
	vector<int> vectorTask = vector<int>();
	vector<int> vectorResourse = vector<int>();

	long long time = 0;
	int size;
	cout << "Enter size of matrix: ";
	cin >> size;
	generateMatrix(matrix, row, column, size);

	cout << "\nGenerated matrix:\n" << endl;
	showMatrix(matrix, row, column);

	createVectorTask(vectorTask, matrix);
	createVectorResourse(vectorResourse, matrix);
	algo(matrix, row, column, vectorTask, vectorResourse, time, true);
	
	cout << "\nTime:" << time<<"ms" << endl;
}

void generateMatrix(vector<vector<int>>& matrix, vector<int>& row, vector<int>& column, int size)
{
	srand(time(NULL));

	for (int i = 0; i < size; i++)
	{
		int sumRow = 0;
		int sumColumn = 0;
		vector<int> task = vector<int>();
		for (int j = 0; j < size; j++)
		{
			task.push_back(rand() % 2);
		}
		matrix.push_back(task);
	}

	for (int i = 0; i < size; i++)
	{
		row.push_back(i);
		column.push_back(i);
	}
}

void showMatrix(vector<vector<int>> matrix, vector<int> row, vector<int> column)
{
	cout << "   ";
	for (int i = 0; i < column.size(); i++)
	{
		printf("%2d", column[i] + 1);
	}
	cout << endl;
	cout << "   ";
	for (int i = 0; i < column.size(); i++)
	{
		cout << "__";
	}
	cout << endl;

	for (int i = 0; i < matrix.size(); i++)
	{
		printf("%2d |", row[i] + 1);

		for (int j = 0; j <matrix.size(); j++)
		{
			cout << matrix[row[i]][column[j]] << " ";
		}
		cout << endl;
	}
}

void createVectorTask(vector<int>& vec, vector<vector<int>> matrix)
{
	for (int i = 0; i < matrix.size(); i++)
	{
		int sum = 0;
		for (int j = 0; j < matrix.size(); j++)
		{
			sum += matrix[i][j];
		}
		vec.push_back(sum);
	}
}

void createVectorResourse(vector<int>& vec, vector<vector<int>> matrix)
{
	for (int i = 0; i < matrix.size(); i++)
	{
		int sum = 0;
		for (int j = 0; j < matrix.size(); j++)
		{
			sum += matrix[j][i];
		}
		vec.push_back(sum);
	}
}

void changeVector(vector<int>& vec, int i, int j)
{
	int vecNum = vec[i];
	vec[i] = vec[j];
	vec[j] = vecNum;
}

int findMin(vector<int> vec, int size)
{
	int result = size;
	int min = vec.size();
	for (int i = size; i < vec.size(); i++)
	{
		if (vec[i] < min)
		{
			min = vec[i];
			result = i;
		}
	}
	return result;
}

int findMax(vector<int> vec, vector<vector<int>> matrix, vector<int>& row, vector<int>& column, int size)
{
	int result = size;
	int max = 0;
	for (int i = size; i < vec.size(); i++)
	{
		if (vec[i] > max && matrix[row[size]][column[i]] == 1)
		{
			max = vec[i];
			result = i;
		}
	}
	return result;
}

void algo(vector<vector<int>>& matrix, vector<int>& row, vector<int>& column,
	vector<int>& vectorTask, vector<int>& vectorResourse, long long& time, bool show)
{
	for (int i = 0; i < matrix.size(); i++)
	{
		long long start = clock();

		int j = findMin(vectorTask, i);
		changeVector(row, i, j);
		changeVector(vectorTask, i, j);

		int z = findMax(vectorResourse, matrix, row, column, i);
		changeVector(column, i, z);
		changeVector(vectorResourse, i, z);

		long long finish = clock();
		time += (finish - start);

		if (show)
		{
			cout << "\nMatrix after step " << i + 1 << ":\n" << endl;
			showMatrix(matrix, row, column);
		}
	}
}

void show(vector<int> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		cout << vec[i] << " ";
	}
}