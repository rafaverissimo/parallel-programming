# include <iostream>
# include <cstdlib>
# include <mpi.h>
# include <sstream>
# include <fstream>

int main( int nargs, char* argv[] )
{
    MPI_Init(&nargs, &argv);
    int numero_du_processus, nombre_de_processus, namelen;
    char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_rank(MPI_COMM_WORLD,
                  &numero_du_processus); // ID of current process
    MPI_Comm_size(MPI_COMM_WORLD, 
                  &nombre_de_processus); //Numero de processos 
    MPI_Get_processor_name(name, &namelen); //Hostname of node
    std::cout << "Hello world from " 
              << numero_du_processus << "running on "
              << namelen << " in "
              << nombre_de_processus << " executed" 
              << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
