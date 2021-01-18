# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>
# include <math.h>

int compterconnexions(int node,int dimension, int *connexions)
{
	if(node < 0 || node >= pow(2,dimension)) return 0;

	unsigned long long mask = 1;
  	for (int i = 0; i < dimension; i++) 
 	{
      connexions[i] = (node^ (mask << i));
  	}
	return 1;
}

int main( int nargs, char* argv[] )
{


	int nbp, rank;
	MPI_Init( &nargs, &argv );
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	MPI_Comm_size(globComm, &nbp);
	MPI_Comm_rank(globComm, &rank);

	int dimension = 0;

	while (nbp > pow(2,dimension) )
	{
		dimension += 1;
	}
	

	int valeur = 5;
	int connexions[dimension];
	int recu, flag;
    MPI_Status Stat[dimension], status; 
    MPI_Request req[dimension];

	compterconnexions(rank,dimension, connexions);

    if (rank == 0)
	{
		for(int i = 0; i < dimension; i++ ){
			MPI_Isend(&valeur , 1, MPI_INT , connexions[i] , 1, MPI_COMM_WORLD, &req[i]);}

		double start = MPI_Wtime();
		double end = MPI_Wtime();
		while(end-start < 100 && flag == 0)
		{
			MPI_Testall(dimension , req , &flag , Stat);
			end = MPI_Wtime();
		}
		
	}    
    
	else 
	{
        MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

		for(int i = 0; i < dimension; i++ ){
			MPI_Isend(&recu , 1, MPI_INT , connexions[i] , 1, MPI_COMM_WORLD, &req[i]);}

		double start = MPI_Wtime();
		double end = MPI_Wtime();
		while(end-start < 100 && flag == 0)
		{
			MPI_Testall(dimension , req , &flag , Stat);
			end = MPI_Wtime();
		}
		
		std::cout << "Le noeud " << rank <<" a reçu <"<<recu<<"> du noeud:" <<std::endl;
		std::cout << status.MPI_SOURCE  << "\n" <<std::endl;  
    }


	MPI_Finalize();
	return EXIT_SUCCESS;
}