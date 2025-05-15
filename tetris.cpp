#include "tetris.hpp"

tetris::tetris(uint32_t w, uint32_t h, uint32_t s)
    : m_score(s), m_width(w), m_height(h), m_field(nullptr) {
    if (w == 0 || h == 0)
        throw tetris_exception("Width and height must be non-zero.");
}

tetris::tetris()
    : m_score(0), m_width(0), m_height(0), m_field(nullptr) 
{}

tetris::~tetris() {
    while (m_field) {
        node* temp = m_field;
        m_field = m_field->next;
        delete temp;
    }
}

tetris::tetris(tetris const& rhs)
    : m_score(rhs.m_score), m_width(rhs.m_width), m_height(rhs.m_height), m_field(nullptr) {
    node** curr = &m_field;
    for (node* n = rhs.m_field; n != nullptr; n = n->next) {
        *curr = new node{n->tp, nullptr};  // copia profonda del tetris_piece (usa il copy constructor di piece)
        curr = &(*curr)->next;
    }
}



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

tetris& tetris::operator=(tetris const& rhs) {
    if (this == &rhs) return *this;

    // Libera la lista attuale
    while (m_field) {
        node* temp = m_field;
        m_field = m_field->next;
        delete temp;
    }

    // Copia i dati
    m_score = rhs.m_score;
    m_width = rhs.m_width;
    m_height = rhs.m_height;
    m_field = nullptr;

    node** curr = &m_field;
    for (node* n = rhs.m_field; n != nullptr; n = n->next) {
        *curr = new node{n->tp, nullptr};
        curr = &(*curr)->next;
    }

    return *this;
}

tetris::tetris(tetris&& rhs)
    : m_score(rhs.m_score), m_width(rhs.m_width), m_height(rhs.m_height), m_field(rhs.m_field) {
    rhs.m_score = rhs.m_width = rhs.m_height = 0;
    rhs.m_field = nullptr;
}

tetris& tetris::operator=(tetris&& rhs) {
    if (this == &rhs) return *this;

    // Libera memoria attuale
    while (m_field) {
        node* temp = m_field;
        m_field = m_field->next;
        delete temp;
    }

    // Trasferisce i dati
    m_score = rhs.m_score;
    m_width = rhs.m_width;
    m_height = rhs.m_height;
    m_field = rhs.m_field;

    rhs.m_score = rhs.m_width = rhs.m_height = 0;
    rhs.m_field = nullptr;

    return *this;
}

uint32_t tetris::score() const {
    return m_score;
}

uint32_t tetris::width() const {
    return m_width;
}

uint32_t tetris::height() const {
    return m_height;
}

bool tetris::operator==(tetris const& rhs) const {
    if (m_score != rhs.m_score ||
        m_width != rhs.m_width ||
        m_height != rhs.m_height)
        return false;

    node* a = m_field;
    node* b = rhs.m_field;

    while (a && b) {
        if (!(a->tp.p == b->tp.p) || a->tp.x != b->tp.x || a->tp.y != b->tp.y)
            return false;
        a = a->next;
        b = b->next;
    }

    return (a == nullptr && b == nullptr);
}

bool tetris::operator!=(tetris const& rhs) const {
    return !(*this == rhs);
}

tetris::iterator::iterator(node* ptr) : m_ptr(ptr) {}

tetris::iterator::reference tetris::iterator::operator*() {
    return m_ptr->tp;
}

tetris::iterator::pointer tetris::iterator::operator->() {
    return &(m_ptr->tp);
}

tetris::iterator& tetris::iterator::operator++() {
    m_ptr = m_ptr->next;
    return *this;
}

tetris::iterator tetris::iterator::operator++(int) {
    iterator temp = *this;
    m_ptr = m_ptr->next;
    return temp;
}

bool tetris::iterator::operator==(iterator const& rhs) const {
    return m_ptr == rhs.m_ptr;
}

bool tetris::iterator::operator!=(iterator const& rhs) const {
    return m_ptr != rhs.m_ptr;
}

tetris::const_iterator::const_iterator(node const* ptr) : m_ptr(ptr) {}

tetris::const_iterator::reference tetris::const_iterator::operator*() const {
    return m_ptr->tp;
}

tetris::const_iterator::pointer tetris::const_iterator::operator->() const {
    return &(m_ptr->tp);
}

tetris::const_iterator& tetris::const_iterator::operator++() {
    m_ptr = m_ptr->next;
    return *this;
}

tetris::const_iterator tetris::const_iterator::operator++(int) {
    const_iterator temp = *this;
    m_ptr = m_ptr->next;
    return temp;
}

bool tetris::const_iterator::operator==(const_iterator const& rhs) const {
    return m_ptr == rhs.m_ptr;
}

bool tetris::const_iterator::operator!=(const_iterator const& rhs) const {
    return m_ptr != rhs.m_ptr;
}

tetris::iterator tetris::begin() {
    return iterator(m_field);
}

tetris::iterator tetris::end() {
    return iterator(nullptr);
}

tetris::const_iterator tetris::begin() const {
    return const_iterator(m_field);
}

tetris::const_iterator tetris::end() const {
    return const_iterator(nullptr);
}

std::ostream& operator<<(std::ostream& os, tetris const& t) {
    os << t.score() << ' ' << t.width() << ' ' << t.height() << '\n';

    for (auto const& tp : t) {
        os << tp.p << ' ' << tp.x << ' ' << tp.y << '\n';
    }

    return os;
}

std::istream& operator>>(std::istream& is, tetris& t) {
    uint32_t score, width, height;
    is >> score >> width >> height;

    if (!is) throw tetris_exception("Invalid header format");

    t = tetris(width, height, score);

    // Lista temporanea (in coda)
    struct temp_node {
        piece p;
        int x, y;
        temp_node* next;
    };

    temp_node* head = nullptr;
    temp_node* tail = nullptr;

    while (is) {
        piece p;
        int x, y;

        std::streampos pos = is.tellg();  // salva posizione

        try {
            is >> p >> x >> y;
        } catch (...) {
            is.clear();
            is.seekg(pos);
            break;
        }

        if (!is) break;

        temp_node* new_node = new temp_node{p, x, y, nullptr};

        if (!head) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    // Inserisci i pezzi **in ordine originale** usando `add` (che mette in testa)
    // → inseriamo partendo dalla fine della lista temporanea
    // Quindi dobbiamo fare **una seconda passata** al contrario

    // Step 1: inverte la lista temporanea
    temp_node* reversed = nullptr;
    while (head) {
        temp_node* tmp = head;
        head = head->next;
        tmp->next = reversed;
        reversed = tmp;
    }

    // Step 2: inserisce nella lista tetris
    while (reversed) {
        t.add(reversed->p, reversed->x, reversed->y);
        temp_node* tmp = reversed;
        reversed = reversed->next;
        delete tmp;
    }

    return is;
}
