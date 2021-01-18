# include <iostream>
# include <cstdlib>
# include <mpi.h>
# include <sstream>
# include <fstream>
# include <iomanip>

int main (int nargs, char* argv[])
{
    MPI_Init(&nargs, &argv);
    int numero_du_processus, nombre_de_processus;

    MPI_Comm_rank(MPI_COMM_WORLD, &numero_du_processus);
    MPI_Comm_size(MPI_COMM_WORLD, &nombre_de_processus);

    std::stringstream fileName;
    fileName << "Output" << std::setfill('0') << std::setw(5) << numero_du_processus << ".txt";
    std::ofstream output(fileName.str().c_str());

    output << "Hello World from " << numero_du_processus << "in " << nombre_de_processus << std::endl;

    output.close();
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize(); 
    return EXIT_SUCCESS;
}