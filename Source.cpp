#include <mpi.h> 
#include<omp.h>
#include <chrono>
#include <stdio.h> 
#include<fstream>
#include<string>
#include <iostream>
using namespace std;
using namespace std::chrono;
#define N 50
int main(int argc, char* argv[])
{
	fstream file;
	int x = 0;
	char character;
	char* arr = (char*)malloc(N * sizeof(char));

	int localsize = 0;
	int localcount[4] = { 0 };
	int globalcount[4] = { 0 };

	int rank;
	int numofprocessors;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numofprocessors);
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0) {

		string filepath = "C:\\Users\\pop\\source\\repos\\par\\";

		file.open(filepath+"DNA50.txt");
		//file.open(filepath+"DNA500.txt");
		//file.open(filepath+"DNA50000.txt");
	   //file.open(filepath+"DNA5000000.txt");

		if (file.fail())
			cout << " error failed to open file!!!" << endl;
		else {
			cout << " file opened successfully\n";

			while (!file.eof()) {
				if (x < N) {

					file.get(character);
					arr[x] = character;
					//printf("newarr[%d]=%c\n", x, newarr[x] );
					x++;

				}
				else {

					break;
				}
			}
		}
		cout << "number of characters in file = " << x << endl;

	}

	localsize = N / numofprocessors;
	int remain = N % numofprocessors;
	int* sendcount = new int[numofprocessors];
	int recvcount;
	int* displics = new int[numofprocessors];

	if (remain == 0) {
		for (int i = 0; i < numofprocessors; i++) {
			sendcount[i] = localsize;
			displics[i] = i * localsize;
			recvcount = localsize;
		}
		char* recvbuff;
		recvbuff = (char*)malloc(localsize * sizeof(char));

		MPI_Scatterv(arr, sendcount, displics, MPI_CHAR, recvbuff, recvcount, MPI_CHAR, 0, MPI_COMM_WORLD);

		int nthreads, thread_id;
		auto start = high_resolution_clock::now();

		if (rank == 0)
		 start = high_resolution_clock::now();
#pragma omp parallel private (thread_id)
		{

			nthreads = omp_get_num_threads();
			thread_id = omp_get_thread_num();

			for (int i = (thread_id * sendcount[rank] / nthreads)-1; i < (thread_id + 1) * sendcount[rank] / nthreads; i++) {
				//for (int i = 0; i < sendcount[0]; i++) {

				if (recvbuff[i] == 'A')
					localcount[0]++;

				if (recvbuff[i] == 'C')
					localcount[1]++;

				if (recvbuff[i] == 'G')
					localcount[2]++;

				if (recvbuff[i] == 'T')
					localcount[3]++;
			}
#pragma omp barrier    /* valid usage    */
		}
		if (rank == 0) {
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			cout << "Time taken by function: " << duration.count()   << " micro seconds" << endl;
		}
		for (int i = (nthreads*(sendcount[rank] / nthreads))+1; i < sendcount[rank] ; i++) {
			//for (int i = 0; i < sendcount[0]; i++) {

			if (recvbuff[i] == 'A')
				localcount[0]++;

			if (recvbuff[i] == 'C')
				localcount[1]++;

			if (recvbuff[i] == 'G')
				localcount[2]++;

			if (recvbuff[i] == 'T')
				localcount[3]++;
		}
		MPI_Reduce(&localcount, &globalcount, 4, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		if (rank == 0)
			printf("count of A=%d  \n count of C=%d \n count of G=%d \ncount of T=%d \n", globalcount[0], globalcount[1], globalcount[2], globalcount[3]);


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
		recvbuff = (char*)malloc(localsize * sizeof(char));

		MPI_Scatterv(arr, sendcount, displics, MPI_CHAR, recvbuff, recvcount + remain, MPI_CHAR, 0, MPI_COMM_WORLD);
		for (int i = 0; i < sendcount[0]; i++) {

			if (recvbuff[i] == 'A')
				localcount[0]++;

			if (recvbuff[i] == 'C')
				localcount[1]++;

			if (recvbuff[i] == 'G')
				localcount[2]++;

			if (recvbuff[i] == 'T')
				localcount[3]++;
		}
		MPI_Reduce(&localcount, &globalcount, 4, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		if (rank == 0)
			printf("count of A=%d \n count of C=%d \n count of G=%d\n count of T=%d \n", globalcount[0], globalcount[1], globalcount[2], globalcount[3]);

	}

	MPI_Finalize();

	return 0;
}