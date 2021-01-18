#include <cstdlib>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>        
#include <SDL2/SDL_image.h>
#include <fstream>
#include <ctime>
#include <iomanip>      // std::setw
#include <chrono>
#include <mpi.h>
#include <omp.h>

#include "parametres.hpp"
#include "galaxie.hpp"
 
int main(int argc, char **argv)
{
    int rank, nbp;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_size(globComm, &nbp);
    MPI_Comm_rank(globComm, &rank);
    MPI_Status Stat[4]; 
    MPI_Request req[4];
    omp_set_num_threads(4);

    char commentaire[4096];
    int width, height;
    SDL_Event event;
    SDL_Window   * window;

    parametres param;
    std::ifstream fich("parametre.txt");
    fich >> width;
    fich.getline(commentaire, 4096);
    fich >> height;
    fich.getline(commentaire, 4096);
    fich >> param.apparition_civ;
    fich.getline(commentaire, 4096);
    fich >> param.disparition;
    fich.getline(commentaire, 4096);
    fich >> param.expansion;
    fich.getline(commentaire, 4096);
    fich >> param.inhabitable;
    fich.getline(commentaire, 4096);
    fich.close();

    std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
    std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
    std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
    std::cout << "\t Chance expansion : " << param.expansion << std::endl;
    std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
    std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
    std::srand(std::time(nullptr));
    


    int deltaT = (20*52840)/width;
    std::cout << "Pas de temps : " << deltaT << " années" << std::endl;
    std::cout << std::endl;

    unsigned long long temps = 0;

    std::chrono::time_point<std::chrono::system_clock> start, end1;
    std::chrono::duration<double> elaps1;

    if(rank==0)
    {
      SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
      window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, SDL_WINDOW_SHOWN);
      galaxie_renderer gr(window);
      galaxie g0(width, height, param.apparition_civ);
      galaxie g_next(width, height);
      while (temps < 4000000)
      {
        start = std::chrono::system_clock::now();
        gr.render(g0);
        MPI_Gather(g_next.data(), width*height/(nbp-1), MPI_CHAR, g0.data(), width*height/(nbp-1), MPI_CHAR,0, MPI_COMM_WORLD);
        
        end1 = std::chrono::system_clock::now();
        elaps1 = ((elaps1 * temps) + (end1 - start) * deltaT) / (temps + deltaT);
        temps += deltaT;
        std::cout << "Temps passe : "
                  << std::setw(10) << temps << " années"
                  << std::fixed << std::setprecision(3)
                  << "  "
                  << "total " << elaps1.count() * 1000
                  << "\r" << std::flush;

        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            std::cout << std::endl << "The end" << std::endl;
            break;}

      }
    }

    else
    {
      int size = height/(nbp-1)+2;   
      galaxie g(width, size, param.apparition_civ);
      galaxie g_next(width, size);  
      while (1)
      {
        std::vector<char> Recv_begin(2*width,0);
        std::vector<char> Recv_end(2*width,0);

        mise_a_jour(param, width, size, g.data(), g_next.data());

        if(rank == 1 && nbp > 2)
        { 
          MPI_Isend(g_next.data() + (size-2)*width*sizeof(char) , 2*width, MPI_CHAR, rank+1, 0, MPI_COMM_WORLD, &req[0]);
          MPI_Irecv(Recv_end.data(), 2*width, MPI_CHAR, rank+1, 0, MPI_COMM_WORLD, &req[1]);
          MPI_Waitall(2, req , Stat);

          UpdtEnd(width,size,Recv_end.data(), g_next.data(),g.data());

        }
        
        else if(rank == nbp-1 && nbp > 2)
        {
          MPI_Isend(g_next.data(), 2*width, MPI_CHAR, rank-1, 0, MPI_COMM_WORLD, &req[0]);
          MPI_Irecv(Recv_begin.data(), 2*width, MPI_CHAR, rank-1, 0, MPI_COMM_WORLD, &req[1]);
          MPI_Waitall(2, req , Stat);

          UpdtBegin(width,size,Recv_begin.data(), g_next.data(), g.data());
        }
        else if(nbp > 2)
        {
          MPI_Isend(g_next.data()+(size-2)*width*sizeof(char), 2*width, MPI_CHAR, rank+1, 0, MPI_COMM_WORLD, &req[0]);
          MPI_Irecv(Recv_end.data(), 2*width, MPI_CHAR, rank+1, 0, MPI_COMM_WORLD, &req[1]);
          MPI_Isend(g_next.data(), 2*width, MPI_CHAR, rank-1, 0, MPI_COMM_WORLD, &req[2]);
          MPI_Irecv(Recv_begin.data(), 2*width, MPI_CHAR, rank-1, 0, MPI_COMM_WORLD, &req[3]);
          MPI_Waitall(4, req , Stat);

          UpdtBegin(width,size,Recv_begin.data(), g_next.data(), g.data());
          UpdtEnd(width,size,Recv_end.data(), g_next.data(),g.data());
        }      

        MPI_Gather(g_next.data() + width*sizeof(char), width*height/(nbp-1), MPI_CHAR, g.data(), width*height/(nbp-1), MPI_CHAR,0, MPI_COMM_WORLD);
        g_next.swap(g);
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            std::cout << std::endl << "The end" << std::endl;
            break;}
      }
      

    }
   
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
