#include <mpi.h> 
#include<omp.h>
#include <chrono>
#include <stdio.h> 
#include<fstream>
#include<string>
#include <iostream>
using namespace std;
using namespace std::chrono;

#define N 5000000
int main(int argc, char* argv[])
{
	fstream file;
	int x = 0;
	char character;
	char* arr = (char*)malloc(N * sizeof(char));

	string word = "GCCAGATATTCCCCCCGTT";
	int wordlen = 19;
	int localcount = 0;
	int totalcount = 0;

	int rank;
	int numofprocessors;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numofprocessors);
	MPI_Barrier(MPI_COMM_WORLD);
	string filepath="C:\\Users\\pop\\source\\repos\\par\\";
	if (rank == 0) {
		//file.open(filepath+"DNA50.txt");
		//file.open(filepath+"DNA500.txt");
		//file.open(filepath+"DNA50000.txt");
		file.open(filepath+"DNA5000000.txt");

		if (file.fail()) {
			cout << " error failed to open file!!!" << endl;

			cout << " path:%s" << filepath + "DNA5000000.txt" << endl;
		}
		else {
			cout << " file opened successfully\n";

			while (!file.eof()) {
				if (x < N) {

					file.get(arr[x]);
					x++;

				}
				else {

					break;
				}
			}

		}
		cout << "number of characters in file = " << x << endl;
	}

	int localsize = N / numofprocessors;
	int remain = N % numofprocessors;
	int* sendcount = new int[numofprocessors];
	int recvcount;
	int* displics = new int[numofprocessors];

	if (remain == 0) {
		for (int i = 0; i < numofprocessors; i++) {
			sendcount[i] = localsize;
			displics[i] = i * localsize;

		}

		recvcount = localsize;

		char* recvbuff;
		recvbuff = (char*)malloc(recvcount * sizeof(char));

		MPI_Scatterv(arr, sendcount, displics, MPI_CHAR, recvbuff, recvcount, MPI_CHAR, 0, MPI_COMM_WORLD);
		int i, j,thread_id, nthreads;
		auto start = high_resolution_clock::now();
#pragma omp parallel private (thread_id)
		{
			thread_id = omp_get_thread_num();
			nthreads = omp_get_num_threads();
		//	for (i = 0; i < recvcount; i++)
		for(i=(thread_id * recvcount / nthreads); i < (thread_id + 1) * recvcount / nthreads; i++)
			{
				for (j = 0; j < wordlen; j++)
				{
					if (recvbuff[i + j] != word[j])
						break;
				}
				if (j == wordlen)
				{
					localcount++;
					//i += wordlen;
				}

			}
		}
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		cout << "Time taken by function: " << duration.count()/1000 << " mile seconds" << endl;
		MPI_Reduce(&localcount, &totalcount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		if (rank == 0)
			printf("total count of \"GCCAGATATTCCCCCCGTT\"=%d\n", totalcount);
	}

	if (remain != 0) {
		sendcount[0] = localsize + remain;
		displics[0] = 0;
		recvcount = sendcount[0];
		for (int i = 1; i < numofprocessors; i++) {
			sendcount[i] = localsize;

			displics[i] = (i * localsize) + remain;
			recvcount = sendcount[i];
		}
		char* recvbuff;
		recvcount += remain;
		recvbuff = (char*)malloc(recvcount * sizeof(char));
		MPI_Scatterv(arr, sendcount, displics, MPI_CHAR, recvbuff, recvcount, MPI_CHAR, 0, MPI_COMM_WORLD);
		printf("displics[%d]=%d\n", rank, displics[rank]);
		int i, j, thread_id=0, nthreads=0;
		//#pragma omp parallel private (thread_id)
		{
			thread_id = omp_get_thread_num();
			nthreads = omp_get_num_threads();
				for (i = 0; i < recvcount; i++)
			//for (i = (thread_id * recvcount / nthreads); i < (thread_id + 1) * recvcount / nthreads; i++) {
				for (j = 0; j < wordlen; j++)
				{
					if (recvbuff[i + j] != word[j])
						break;
				}
				if (j == wordlen)
				{
					localcount++;

				}
			}
		}

		MPI_Reduce(&localcount, &totalcount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		if (rank == 0)
			printf("total count of \"GCCAGATATTCCCCCCGTT\"=%d\n", totalcount);
	
	MPI_Finalize();

	return 0;
}