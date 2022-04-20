#include <iostream>
#include <fstream>
#include <mpi.h>
#include <omp.h>
#include <string>

using namespace std;

int const WIDTH = 800;
int const HEIGHT = 800;
double const REAL_MAX = 2;
double const REAL_MIN = -2;
double const IMAG_MAX = 2;
double const IMAG_MIN = -2;
int const ITER_MAX = 500;
int const OPENMP_DIV = 50;
string const IMAGE_METADATA = "P3\n" + to_string(WIDTH) + " " + to_string(HEIGHT) + "\n" + to_string(255) + "\n";

struct coor
{
	float rl;
	float ig;
};

int cal_pixel(coor prev)
{
	int count;
	coor z;
	float tep, legtsq;
	z.rl = 0;
	z.ig = 0;
	count = 0;
	do {
		tep = z.rl * z.rl - z.ig * z.ig + prev.rl;
		z.ig = 2 * z.rl * z.ig + prev.ig;
		z.rl = tep;
		legtsq = z.rl * z.rl + z.ig * z.ig;
		count++;
	} while (legtsq < 4.0 && count < ITER_MAX);
	return count;
}

int main(int argc, char* argv[])
{
	int nPes, myRank, distRow = 0;
	double scaleReal = (REAL_MAX - REAL_MIN) / WIDTH, scaleImag = (IMAG_MAX - IMAG_MIN) / HEIGHT;
	double time;

	MPI_Status status;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &nPes);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	int local[WIDTH];

	if (myRank == 0)
	{
		int count = 0;

		time = MPI_Wtime();

		for (int i = 1; i < nPes; i++) {
			MPI_Send(&distRow, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&local, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
			distRow++;
			count++;
		}

		ofstream myfile;
		myfile.open("MPI_Graph.ppm");
		myfile << IMAGE_METADATA;

		int process = 1;
		do {
			MPI_Recv(&local, WIDTH, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			count--;

			if (distRow < HEIGHT) {
				MPI_Send(&distRow, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
				distRow++;
				count++;
			}
			else {
				MPI_Send(&distRow, 1, MPI_INT, process, 1, MPI_COMM_WORLD);
			}

			if (process + 1 == nPes) process = 1;
			else process++;

			for (int y = 0; y < WIDTH; y++)
			{
				myfile << (local[y] * 1) % 256 << " " << (local[y] * 7) % 256 << " " << (local[y] * 7) % 256 << "   ";
			}
			myfile << "\n";
		} while (count > 0);

		time = MPI_Wtime() - time;
		cout << time;
		myfile.close();
	}
	else
	{
		MPI_Recv(&distRow, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&local, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
		int i = 0;
		int div = WIDTH / OPENMP_DIV;
		int filaInicial;
		while (status.MPI_TAG != 1) {
			
			coor z;
			omp_set_num_threads(OPENMP_DIV);
			#pragma omp parallel private(z, filaInicial, i) shared(local)
			{
				filaInicial = omp_get_thread_num() * div;
				for (i = filaInicial; i < filaInicial + div; i++)
				{
					z.ig = IMAG_MIN + ((float)distRow * scaleImag);
					z.rl = REAL_MIN + ((float)i * scaleReal);
					local[i] = cal_pixel(z);
				}
			}
			
			MPI_Send(&local, WIDTH, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(&distRow, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		}
	}
	MPI_Finalize();
	return 0;
}
