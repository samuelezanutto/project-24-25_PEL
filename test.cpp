#include "tetris.hpp"

int main() {
    // Costruisci un campo
    tetris t(8, 6);

    // Crea un pezzo 2x2 colorato
    piece p(2, 75);  // 75 = colore giallo

    // Imposta celle del pezzo manualmente
    p(0, 0) = true;
    p(0, 1) = true;
    p(1, 0) = true;
    p(1, 1) = true;

    // Inserisci il pezzo nella colonna 3
    t.insert(p, 3);

    // Stampa il campo
    t.print_ascii_art(std::cout);

    // Stampa punteggio
    std::cout << "Score: " << t.score() << "\n";

    return 0;
}
