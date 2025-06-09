#include "tetris.hpp"
#include <sstream>

int main() {
    std::cout << "=== Test classe piece ===\n";

    piece p1(2, 75); // 2x2, colore 75
    p1(0, 0) = true;
    p1(0, 1) = false;
    p1(1, 0) = true;
    p1(1, 1) = true;

    std::cout << "Pezzo p1:\n";
    p1.print_ascii_art(std::cout);

    std::cout << "\nRuoto p1:\n";
    p1.rotate();
    p1.print_ascii_art(std::cout);

    std::cout << "\nTaglio riga 1:\n";
    p1.cut_row(1);
    p1.print_ascii_art(std::cout);

    std::cout << "\nVerifica empty(): " << (p1.empty() ? "SI" : "NO") << "\n";

    std::cout << "\nSerializzo p1:\n";
    std::ostringstream oss;
    oss << p1;
    std::cout << oss.str() << "\n";

    std::istringstream iss(oss.str());
    piece p2;
    iss >> p2;

    std::cout << "\nDeserializzato p2:\n";
    p2.print_ascii_art(std::cout);

    std::cout << "\np1 == p2? " << (p1 == p2 ? "SI" : "NO") << "\n";

    // ---------------------

    std::cout << "\n=== Test classe tetris ===\n";

    tetris game(6, 6);  // campo 6x6

    piece t1(2, 200);  // pezzo 2x2 rosso
    t1(0, 0) = true;
    t1(0, 1) = true;
    t1(1, 0) = true;
    t1(1, 1) = true;

    std::cout << "Inserisco pezzo t1...\n";
    game.insert(t1, 2);
    std::cout << "Inserimento completato.\n";

    std::cout << "\nCampo dopo insert:\n";
    game.print_ascii_art(std::cout);
    std::cout << "Score: " << game.score() << "\n";

    // Copia
    tetris copy = game;
    std::cout << "\nCopia uguale? " << (game == copy ? "SI" : "NO") << "\n";

    // Serializza e deserializza
    std::ostringstream game_out;
    game_out << game;

    std::cout << "\nSerializzato:\n" << game_out.str();
    
    std::istringstream game_in(game_out.str());
    tetris loaded;
    game_in >> loaded;
    std::cout << "\nDeserializzazione completata\n";

    std::cout << "\nCampo deserializzato:\n";
    loaded.print_ascii_art(std::cout);

    std::cout << "\nDeserializzato uguale all’originale? " << (loaded == game ? "SI" : "NO") << "\n";

    return 0;
}
