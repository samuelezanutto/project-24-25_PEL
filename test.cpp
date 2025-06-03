#include <iostream>
#include "tetris.hpp"

int main() {
    tetris t1(8, 6);  // campo 8x6

    piece p(2, 75);  // pezzo 2x2 giallo
    p(0, 0) = true;
    p(0, 1) = true;
    p(1, 0) = true;
    p(1, 1) = true;

    t1.insert(p, 3);  // inserisce nella colonna 3

    std::cout << "Tetris board:\n";
    t1.print_ascii_art(std::cout);

    std::cout << "Score: " << t1.score() << "\n";

    // Copia
    tetris t2 = t1;

    if (t1 == t2) {
        std::cout << "Copia riuscita!\n";
    }

    // Modifica t2 e verifica che non siano più uguali
    piece p2(2, 200);
    p2(0, 0) = true;
    p2(1, 0) = true;

    t2.insert(p2, 5);

    if (t1 != t2) {
        std::cout << "I due Tetris sono ora diversi.\n";
    }

    return 0;
}
