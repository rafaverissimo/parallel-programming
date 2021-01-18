# include <iostream>
# include <cstdlib>
# include <mpi.h>

int main( int nargs, char* argv[] )
{
	MPI_Init( &nargs, &argv );
	// globComm é o comunicador global.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// Nbp é o número de processos. Esse número é definido pelo usuário quando executa o programa
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// Rank é o numero do processus entre os nbp (Vai de 0 a nbp-1)
	int rank;
	MPI_Comm_rank(globComm, &rank);

	// On peut maintenant commencer à écrire notre programme parallèle en utilisant les
	// services offerts par MPI.
	int jeton, jeton2;
	const int tag=101;
	MPI_Status status;
	MPI_Request request;

	jeton= 5*rank;
	// Primeira variavel é sempre o buffer --> Local de memoria da variavel que esta sendo comunicada
	// No geral &variavel
	// A segunda variavel é o numero de elementos sendo enviados
	// Tipo do dado (MPI_INT) é a terceira variavel
	// Em seguida, para qual tarefa estou enviando
	// Tag é um identificador da mensagem
	// Por ultimo o comunicador
	MPI_Irecv(&jeton2, 1, MPI_INT, (rank+nbp-1)%nbp, tag, globComm, &request);
	MPI_Send(&jeton, 1, MPI_INT, (rank+1)%nbp, tag, globComm);
	MPI_Wait(&request, &status);

	jeton2+=1;
	std::cout << "Hello World, I'm processus " << rank << " on " << nbp << " processes.\n";
	std::cout << rank << " : Recu et incrementer le jeton : " << jeton2 << std::flush << std::endl;
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
