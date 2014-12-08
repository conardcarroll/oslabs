#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	printf("argc %d first %f second %d\n", argc, atof(argv[1]), atoi(argv[2]));
	double A = atof(argv[1]);
	int C = atoi(argv[2]);
	double B = 0;
	double invB = 1;
	int i = 1;
	for(;i <= C; i++){
		invB = 1.0 + invB * (double)i / A;		
	}
	
	B = 1.0f / invB;
	printf("Pb = %f %% \n", B * 100.0);
	return B;
}