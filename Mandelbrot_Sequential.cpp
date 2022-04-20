#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

int const WIDTH = 800;
int const HEIGHT = 800;
int const ITER_MAX = 500;
double const REAL_MAX = 2;
double const REAL_MIN = -2;
double const IMAG_MAX = 2;
double const IMAG_MIN = -2;
string const IMAGE_METADATA = "P3\n" + to_string(WIDTH) + " " + to_string(HEIGHT) + "\n" + to_string(255) + "\n";
 
struct coor
{
	float rl;
	float ig;
};
// a+bi (a,b)

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
 //z_(n+1) = ( z_(n) )^2 + c.

int main()
{
	auto start = high_resolution_clock().now();
	ofstream myfile;
	myfile.open("seq_graph.ppm");
	myfile << IMAGE_METADATA;
	coor z;
	int color;
	float r_scale = (REAL_MAX - REAL_MIN) / WIDTH;
	float i_scale = (IMAG_MAX - IMAG_MIN) / HEIGHT;
	for (int x = 0; x < HEIGHT; x++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			z.rl = REAL_MIN + ((float)i * r_scale);
			z.ig = IMAG_MIN + ((float)x * i_scale);
			color = cal_pixel(z);
			myfile << (color * 1) % 256 << " " << (color * 3) % 256 << " " << (color * 7) % 256 << "   ";
		}
		myfile << "\n";
	}
	myfile.close();
	
	auto finish = high_resolution_clock().now();
	double time = duration_cast<milliseconds>(finish - start).count() / 1000;
	printf("Tomo %f segundos",  time);
	return 0;
}
