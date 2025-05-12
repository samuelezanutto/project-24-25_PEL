#include "tetris.hpp"

tetris::tetris(uint32_t w, uint32_t h, uint32_t s)
    : m_score(s), m_width(w), m_height(h), m_field(nullptr) {
    if (w == 0 || h == 0)
        throw tetris_exception("Width and height must be non-zero.");
}

tetris::tetris()
    : m_score(0), m_width(0), m_height(0), m_field(nullptr) 
{}


void tetris::add(piece const& p, int x, int y) {
    if (!containment(p, x, y))
        throw tetris_exception("Invalid add: piece out of bounds or overlapping");

    node* new_node = new node{{p, x, y}, m_field};
    m_field = new_node;
}
    
bool tetris::containment(piece const& p, int x, int y) const {
    uint32_t side = p.side();

    for (uint32_t i = 0; i < side; ++i) {
        for (uint32_t j = 0; j < side; ++j) {
            if (!p(i, j)) continue;

            int fx = x + j;
            int fy = y + (side - 1 - i);  // griglia dal basso

            // Controllo limiti campo
            if (fx < 0 || fx >= int(m_width) || fy < 0 || fy >= int(m_height))
                return false;

            // Controllo sovrapposizione con altri pezzi
            for (node* cur = m_field; cur != nullptr; cur = cur->next) {
                const piece& other = cur->tp.p;
                int ox = cur->tp.x;
                int oy = cur->tp.y;
                uint32_t oside = other.side();

                for (uint32_t oi = 0; oi < oside; ++oi) {
                    for (uint32_t oj = 0; oj < oside; ++oj) {
                        if (!other(oi, oj)) continue;

                        int ofx = ox + oj;
                        int ofy = oy + (oside - 1 - oi);

                        if (fx == ofx && fy == ofy)
                            return false;  // collisione
                    }
                }
            }
        }
    }
    return true;
}

void tetris::insert(piece const& p, int x) {
    // Trova y massimo
    int y = 0;
    while (containment(p, x, y + 1)) ++y;

    if (!containment(p, x, y)) throw tetris_exception("GAME OVER");

    add(p, x, y);

    // Lambda: controlla se una riga è piena
    auto is_row_full = [this](int r) -> bool {
        for (uint32_t x = 0; x < m_width; ++x) {
            bool found = false;
            for (node* n = m_field; n != nullptr; n = n->next) {
                const piece& p = n->tp.p;
                int px = n->tp.x, py = n->tp.y;
                uint32_t side = p.side();
                for (uint32_t i = 0; i < side; ++i) {
                    for (uint32_t j = 0; j < side; ++j) {
                        if (!p(i, j)) continue;
                        int fx = px + j;
                        int fy = py + (side - 1 - i);
                        if (fx == int(x) && fy == r) {
                            found = true;
                            goto found_cell;
                        }
                    }
                }
            }
        found_cell:
            if (!found) return false;
        }
        return true;
    };

    // Lambda: taglia riga r da tutti i pezzi che la toccano
    auto cut_row = [this](int r) {
        for (node* n = m_field; n != nullptr; n = n->next) {
            piece& p = n->tp.p;
            int py = n->tp.y;
            uint32_t side = p.side();

            if (r >= py && r < py + int(side)) {
                uint32_t i = side - 1 - (r - py);
                if (i < side) {
                    p.cut_row(i);
                }
            }
        }
    };

    // Loop: taglia, rimuovi vuoti, fai cadere pezzi
    bool changed = true;
    while (changed) {
        changed = false;

        for (int r = 0; r < int(m_height); ++r) {
            if (is_row_full(r)) {
                cut_row(r);
                m_score += m_width;
                changed = true;
            }
        }

        // Rimuovi pezzi vuoti
        node** curr = &m_field;
        while (*curr) {
            if ((*curr)->tp.p.empty()) {
                node* to_delete = *curr;
                *curr = (*curr)->next;
                delete to_delete;
                changed = true;
            } else {
                curr = &(*curr)->next;
            }
        }

        // Fai cadere i pezzi
        for (node* n = m_field; n != nullptr; ++n) {
            while (containment(n->tp.p, n->tp.x, n->tp.y - 1)) {
                --(n->tp.y);
                changed = true;
            }
        }
    }
}

void tetris::print_ascii_art(std::ostream& os) const {
    // Alloca la griglia m_height x m_width e inizializzala a 0
    int** grid = new int*[m_height];
    for (uint32_t i = 0; i < m_height; ++i) {
        grid[i] = new int[m_width];
        for (uint32_t j = 0; j < m_width; ++j) {
            grid[i][j] = 0;
        }
    }

    // Riempie la griglia con i colori dei pezzi
    for (node* n = m_field; n != nullptr; n = n->next) {
        const piece& p = n->tp.p;
        int px = n->tp.x;
        int py = n->tp.y;
        uint32_t side = p.side();
        int color = p.color();

        for (uint32_t i = 0; i < side; ++i) {
            for (uint32_t j = 0; j < side; ++j) {
                if (!p(i, j)) continue;

                int gx = px + j;
                int gy = py + (side - 1 - i);

                if (gx >= 0 && gx < int(m_width) && gy >= 0 && gy < int(m_height)) {
                    grid[gy][gx] = color;
                }
            }
        }
    }

    // Stampa la griglia (dall’alto verso il basso)
    for (int i = m_height - 1; i >= 0; --i) {
        for (uint32_t j = 0; j < m_width; ++j) {
            if (grid[i][j] != 0) {
                os << "\033[48;5;" << grid[i][j] << "m" << ' ' << "\033[m";
            } else {
                os << ' ';
            }
        }
        os << '\n';
    }

    // Dealloca la griglia
    for (uint32_t i = 0; i < m_height; ++i)
        delete[] grid[i];
    delete[] grid;
}




