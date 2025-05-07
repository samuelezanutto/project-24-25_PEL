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
    // Trova la y massima possibile (cioè il punto più basso dove può cadere il pezzo)
    int y = 0;
    while (containment(p, x, y + 1)) {
        ++y;
    }

    // Se nemmeno y=0 è valido, GAME OVER
    if (!containment(p, x, y)) {
        throw tetris_exception("GAME OVER");
    }

    // Aggiungi il pezzo nella lista
    add(p, x, y);

    // Lambda interna per controllare se una riga è piena
    auto is_row_full = [this](int r) -> bool {
        for (uint32_t x = 0; x < m_width; ++x) {
            bool found = false;

            for (node* n = m_field; n != nullptr; n = n->next) {
                const piece& p = n->tp.p;
                int px = n->tp.x;
                int py = n->tp.y;
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

    // Loop per tagliare righe, rimuovere pezzi vuoti, e farli cadere se possibile
    bool changed = true;
    while (changed) {
        changed = false;

        // 1. Trova righe piene e tagliale
        for (int r = 0; r < int(m_height); ++r) {
            if (is_row_full(r)) {
                // Taglieremo con una funzione che scriveremo dopo (cut_row)
                for (node* n = m_field; n != nullptr; n = n->next) {
                    piece& p = n->tp.p;
                    int py = n->tp.y;
                    uint32_t side = p.side();

                    if (r >= py && r < py + int(side)) {
                        p.cut_row(side - 1 - (r - py));
                    }
                }

                m_score += m_width;
                changed = true;
            }
        }

        // 2. Rimuovi pezzi vuoti
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

        // 3. Fai scendere i pezzi se possibile
        for (node* n = m_field; n != nullptr; ++n) {
            while (containment(n->tp.p, n->tp.x, n->tp.y - 1)) {
                --(n->tp.y);
                changed = true;
            }
        }
    }
}


