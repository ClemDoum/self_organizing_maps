#include "Base.h"

// g++ -o basic-example -Wall -ansi -O3 basic-example.cc

// On va stocker les imagettes-prototypes au sein d'une grille.
#define WIDTH  5
#define HEIGHT 4
typedef uci::Map<WIDTH,HEIGHT,
uci::Database::imagette::width,
uci::Database::imagette::height> Prototypes;

// Pour une imagette, les indices sont definis comme suit
//
//         j
//         |
//   ......|.....
//   ......|.....
//   ......|.....
//   ......|.....
//   ......|.....
//   ......#------ i   img(i,j)
//   ............
//   ............
//   ............
//   ............
//   ............  


// Cette fonction permet d'affecter un prototype (dont les pixels sont
// des double dans [0,255]) a une imagette tiree de la base (dont les
// pixels sont des unsigned char). Le & evite les copies inutiles.
void initProto(Prototypes::imagette& w,
 const uci::Database::imagette& xi) {
  for(int i = 0 ; i < uci::Database::imagette::height ; ++i)
    for(int j = 0 ; j < uci::Database::imagette::width ; ++j){
      w(i,j) = (double)(xi(i,j));
    }
  }

  int main(int argc, char* argv[]) {
    Prototypes prototypes;

    // On peut afficher la grille de prototypes.
    prototypes.PPM("proto",1);

    // Utilisons la base de donnees.
    uci::Database database;

    // Pour obtenir une nouvelle imagette...
    database.Next();

    uci::Database::imagette& xi = database.input; // le & fait que xi est un pointeur, on evite une copie.
    std::cout << "L'imagette tiree de la base est un " << database.what << std::endl;


    // Utilisons notre fonction de copie pour affecter un prototype à la valeur de l'imagette.
    initProto(prototypes(2,3),xi);

    // On affiche a nopuveau la grille de prototypes.
    prototypes.PPM("proto",2);

    return 0;
  }