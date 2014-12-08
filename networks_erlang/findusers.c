#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
	int channels = 100;
	double users = 2.0;
	double gos = 0.05;
	double blockedCalls = 0.01;
	double recall = 0.5;
	double acceptable = 0.02;
	double A = 0.0;
	double p = 0.0;
	double result = 1;
	double currentResult = 0;
	double newTraffic = 0;
	
	blockedCalls = users * blockedCalls;
	recall = blockedCalls * recall;
	users = users + recall;
	printf("result %f\n", users);
	
	return 0;
}
