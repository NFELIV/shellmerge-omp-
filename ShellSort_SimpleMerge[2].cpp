// №13 Сортировка Хоара с простым слиянием 
// ЛР#2 Реализация OMP

#include "stdafx.h"
#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>

using namespace std;

void CreateArray(int arr[], int lenght) //Генерация 
{
	srand((unsigned int)time(NULL));
	for (int i = 0; i < lenght; i++)
	{
		arr[i] = rand() % 10;
	}
}

void PrintArray(int* arr, int size) //Печать 
{
	if (size < 20)
	{
		for (int i = 0; i < size; i++)
		{
			cout << arr[i] << " ";
		}
	}
	return;
}

void QuickSort(int* arr, int l, int r) //Быстрая сортировка для последовательной реализации
{
	int i = l, j = r;
	double m = arr[(r + l) / 2];
	double temp = 0;
	while (i <= j)
	{
		while (arr[i] < m)
		{
			i++;
		}
		while (arr[j] > m)
		{
			j--;
		}
		if (i <= j)
		{
			if (i < j)
			{
				temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
			i++;
			j--;
		}
	}
	if (j > l)
	{
		QuickSort(arr, l, j);
	}
	if (r > i)
	{
		QuickSort(arr, i, r);
	}
}
/*  
first_subarray_size - длина первого подмассива 
second_subarray_size - длина второго подмассива
first_subarray_start_index - начальный индекс первого подмассива
second_subarray_start_index - начальный индекс второго подмассива */
void Merge(int* array, const int first_subarray_size, const int second_subarray_size, const int first_subarray_start_index, const int second_subarray_start_index) 
{
	int i = 0, j = 0, k = 0;
	int* merged_subarray = new int[first_subarray_size + second_subarray_size];

	while (i < first_subarray_size && j < second_subarray_size) 
	{
		if (array[first_subarray_start_index + i] <= array[second_subarray_start_index + j]) 
		{
			merged_subarray[k] = array[first_subarray_start_index + i];
			i++;
		}
		else 
		{
			merged_subarray[k] = array[second_subarray_start_index + j];
			j++;
		}
		k++;
	}

	if (i < first_subarray_size)
	{
		for (int p = i; p < first_subarray_size; p++) 
		{
			merged_subarray[k] = array[first_subarray_start_index + p];
			k++;
		}
	}
	else 
	{
		for (int p = j; p < second_subarray_size; p++) 
		{
			merged_subarray[k] = array[second_subarray_start_index + p];
			k++;
		}
	}

	for (i = first_subarray_start_index; i < second_subarray_start_index + second_subarray_size; i++) // Копируем объединенный массив в исходный массив
	{
		array[i] = merged_subarray[i - first_subarray_start_index];
	}

	delete[] merged_subarray;
}

void ParallelQuickSort(int* array, const int size, const int threads)
{
	omp_set_num_threads(threads);
	int subarray_size = size / threads;
#pragma omp parallel for schedule(static) shared(array)
	for (int i = 0; i < threads; i++)
	{
		int low = i * subarray_size;
		int high = 0;
		if (i == threads - 1 && size % threads) {
			high = size - 1;
		}
		else
		{
			high = low + subarray_size - 1;
		}

		QuickSort(array, low, high);
	}
	int step = 1; //Расстояние между объединяющими 
	for (int i = threads / 2; i > 0; i /= 2)
	{
		int first_subarray_size = subarray_size * step;
		int second_subarray_size = first_subarray_size;

		// Обработка оставшихся элементов массива во время последней итерации
		if (i / 2 <= 0)
		{
			second_subarray_size += size % threads;
		}

#pragma omp parallel for schedule(static) \
    shared(array, first_subarray_size, second_subarray_size, step)
		for (int j = 0; j < i; j++)
		{
			int thread_id = omp_get_thread_num();
			int first_subarray_start_index =
				thread_id * subarray_size * step * 2;
			int second_subarray_start_index =
				first_subarray_start_index + subarray_size * step;
			Merge(array, first_subarray_size, second_subarray_size, first_subarray_start_index, second_subarray_start_index);
		}
		step *= 2;
	}
}
int main(int argc, char** argv) 
{
	int size = 10000; 
	int threads = omp_get_max_threads();
	int* array = new int[size]; // Исходный массив
	int* sequence_sorted_array = new int[size]; // Массив для последовательной версии
	int* parallel_sorted_array = new int[size]; // Массив для параллельной версии
	double* equal = new double[size]; 
	int notCorrect = 0; 
	double sequence_time = 0, start_sequence_time = 0, start_parallel_time = 0; 
	double parallel_time = 0, finish_sequence_time = 0, finish_parallel_time = 0;
	cout << "Enter of size array: ";
	cin >> size; 
	cout << "Enter of num's threads: ";
	cin >> threads;
	CreateArray(array, size);
	cout << "The Generated array: ";
	PrintArray(array, size);
	cout << endl;
	for (int i = 0; i < size; i++)
	{
		sequence_sorted_array[i] = array[i];
		parallel_sorted_array[i] = array[i];
	}
    //Параллельная реализация
	start_parallel_time = omp_get_wtime();
	ParallelQuickSort(parallel_sorted_array, size, threads);
	finish_parallel_time = omp_get_wtime();
	parallel_time = finish_parallel_time - start_parallel_time;
	//
	//Последовательная рализация
	start_sequence_time = omp_get_wtime();
	QuickSort(sequence_sorted_array, 0, size - 1);
	finish_sequence_time = omp_get_wtime();
	sequence_time = finish_sequence_time - start_sequence_time;
	//
	for (int i = 0; i < size; i++) //проверка на правильность
	{
		equal[i] = sequence_sorted_array[i] - parallel_sorted_array[i];
		if (equal[i] != 0.0) 
		{
			notCorrect++;
		}
	}
	if (notCorrect > 0) 
	{
		cout << "Sorted arrays are not equal!" << endl;
	}
	else 
	{
		cout << "Parallel sorted array: ";
		PrintArray(parallel_sorted_array, size);
		cout << endl;
		cout << "Sequence sorted array: ";
		PrintArray(sequence_sorted_array, size);
		cout << endl;
		cout << "Sorted arrays are equal!" << endl;
		cout << "Parallel version time: " << parallel_time << endl;
		cout << "Sequence version time: " << sequence_time << endl;
		cout << "Boost: " << sequence_time / parallel_time << endl;
	}
	delete[] array;
	delete[] sequence_sorted_array;
	delete[] parallel_sorted_array;
	return 0;
}