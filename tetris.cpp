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
            int fy = y + i;  

            // Controllo limiti campo
            if (fx < 0 || fx >= int(m_width) || fy < 0 || fy >= int(m_height))
                return false;

            // Controllo sovrapposizione con altri pezzi
            for (node* cur = m_field; cur != nullptr; cur = cur->next) {
                const piece& other = cur->tp.p;
                int ox = cur->tp.x;
                int oy = cur->tp.y;
                uint32_t oside = other.side();

                // Evita auto-collisione quando si verifica discesa dei pezzi stessi
                if (&other == &p) {
                    continue;
                }

                for (uint32_t oi = 0; oi < oside; ++oi) {
                    for (uint32_t oj = 0; oj < oside; ++oj) {
                        if (!other(oi, oj)) continue;

                        int ofx = ox + oj;
                        int ofy = oy + oi; 

                        if (fx == ofx && fy == ofy)
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

void tetris::insert(piece const& p, int x) {
    if (p.empty())
        throw tetris_exception("Invalid insert: empty piece");

    // Trova la y più bassa possibile mantenendo y non-negativa
    int y = static_cast<int>(m_height) - static_cast<int>(p.side());
    if (y < 0) y = 0;
    while (y > 0 && containment(p, x, y - 1)) --y;

    if (!containment(p, x, y))
        throw tetris_exception("GAME OVER");

    add(p, x, y);

    // y rimane sempre >= 0 per rispettare la specifica

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
                        int fy = py + i;
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

            int relative = r - py;
            if (relative < 0 || relative >= int(side)) continue;

            uint32_t i = relative; 

            if (i < side) {
                p.cut_row(i);
            }
        }
    };

    // Loop: taglia righe piene, rimuovi pezzi vuoti, fai cadere i pezzi
    bool changed = true;
    while (changed) {
        changed = false;

        // 1. Taglia righe piene
        for (int r = 0; r < int(m_height); ) {
            if (is_row_full(r)) {
                cut_row(r);
                m_score += m_width;
                changed = true;
            } else {
                ++r;
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

        // 3. Fai cadere i pezzi se possono scendere
        for (node* n = m_field; n != nullptr; n = n->next) {
            while (n->tp.y > 0 && containment(n->tp.p, n->tp.x, n->tp.y - 1)) {
                --(n->tp.y);
                changed = true;
            }
        }
    }
}


void tetris::print_ascii_art(std::ostream& os) const {
    int** grid = new int*[m_height];
    for (uint32_t i = 0; i < m_height; ++i) {
        grid[i] = new int[m_width];
        for (uint32_t j = 0; j < m_width; ++j) {
            grid[i][j] = 0;
        }
    }

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
                int gy = py + i; 

                if (gx >= 0 && gx < int(m_width) && gy >= 0 && gy < int(m_height)) {
                    grid[gy][gx] = color;
                }
            }
        }
    }

    // Stampa la griglia (dall'alto verso il basso)
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
    if (!(is >> score >> width >> height))
        throw tetris_exception("Invalid header format");

    tetris temp(width, height, score);

    piece p;
    int x, y;
    
    while (is >> std::ws, is.peek() != EOF) {  // Controlla se ci sono altri dati
        is >> p >> x >> y;
        
        if (is.fail() && !is.eof())
            throw tetris_exception("Error reading pieces");
            
        if (is.good() || is.eof())
            temp.add(p, x, y);
    }

    t = std::move(temp);
    return is;
}


// PIECE IMPLEMENTATION---------------------------------------------------------------------------------------------------------

piece::piece() : m_side(0), m_color(0), m_grid(nullptr) {}

piece::piece(uint32_t s, uint8_t c) : m_side(0), m_color(0), m_grid(nullptr) {
    if (s == 0)
        throw tetris_exception("Side must be non-zero and a power of two.");
    // iterative check whether s is a power of two
    uint32_t tmp = s;
    while (tmp > 1 && tmp % 2 == 0)
        tmp /= 2;
    if (tmp != 1)
        throw tetris_exception("Side must be a power of two.");

    if (c == 0)
        throw tetris_exception("Color must be non-zero.");

    m_side = s;
    m_color = c;

    m_grid = new bool*[m_side];
    for (uint32_t i = 0; i < m_side; ++i) {
        m_grid[i] = new bool[m_side];
        for (uint32_t j = 0; j < m_side; ++j)
            m_grid[i][j] = false;
    }
}

piece::~piece() {
    for (uint32_t i = 0; i < m_side; ++i)
        delete[] m_grid[i];
    delete[] m_grid;
}

piece::piece(piece const& rhs) : m_side(rhs.m_side), m_color(rhs.m_color) {
    m_grid = new bool*[m_side];
    for (uint32_t i = 0; i < m_side; ++i) {
        m_grid[i] = new bool[m_side];
        for (uint32_t j = 0; j < m_side; ++j)
            m_grid[i][j] = rhs.m_grid[i][j];
    }
}

piece::piece(piece&& rhs) : m_side(rhs.m_side), m_color(rhs.m_color), m_grid(rhs.m_grid) {
    rhs.m_side = 0;
    rhs.m_color = 0;
    rhs.m_grid = nullptr;
}

piece& piece::operator=(piece const& rhs) {
    if (this == &rhs) return *this;

    // Dealloca memoria esistente
    for (uint32_t i = 0; i < m_side; ++i)
        delete[] m_grid[i];
    delete[] m_grid;

    // Copia nuovi dati
    m_side = rhs.m_side;
    m_color = rhs.m_color;

    m_grid = new bool*[m_side];
    for (uint32_t i = 0; i < m_side; ++i) {
        m_grid[i] = new bool[m_side];
        for (uint32_t j = 0; j < m_side; ++j)
            m_grid[i][j] = rhs.m_grid[i][j];
    }

    return *this;
}

piece& piece::operator=(piece&& rhs) {
    if (this == &rhs) return *this;

    // Dealloca memoria esistente
    for (uint32_t i = 0; i < m_side; ++i)
        delete[] m_grid[i];
    delete[] m_grid;

    // Prendi proprietà
    m_side = rhs.m_side;
    m_color = rhs.m_color;
    m_grid = rhs.m_grid;

    // Reset rhs
    rhs.m_side = 0;
    rhs.m_color = 0;
    rhs.m_grid = nullptr;

    return *this;
}


bool& piece::operator()(uint32_t i, uint32_t j) {
    if (i >= m_side || j >= m_side)
        throw tetris_exception("Index out of bounds");
    return m_grid[i][j];
}

bool piece::operator()(uint32_t i, uint32_t j) const {
    if (i >= m_side || j >= m_side)
        throw tetris_exception("Index out of bounds");
    return m_grid[i][j];
}

uint32_t piece::side() const {
    return m_side;
}

int piece::color() const {
    return m_color;
}

bool piece::empty() const {
    if (m_side == 0 || !m_grid) return true;
    for (uint32_t i = 0; i < m_side; ++i) {
        for (uint32_t j = 0; j < m_side; ++j) {
            if (m_grid[i][j]) return false;
        }
    }
    return true;
}

bool piece::full() const {
    if (m_side == 0 || !m_grid) return false;
    for (uint32_t i = 0; i < m_side; ++i) {
        for (uint32_t j = 0; j < m_side; ++j) {
            if (!m_grid[i][j]) return false;
        }
    }
    return true;
}

bool piece::empty(uint32_t i, uint32_t j, uint32_t s) const {
    if (s == 0 || i + s > m_side || j + s > m_side)
        throw tetris_exception("Index out of bounds");

    for (uint32_t x = i; x < i + s; ++x)
        for (uint32_t y = j; y < j + s; ++y)
            if (m_grid[x][y]) return false;
    return true;
}

bool piece::full(uint32_t i, uint32_t j, uint32_t s) const {
    if (s == 0 || i + s > m_side || j + s > m_side)
        throw tetris_exception("Index out of bounds");

    for (uint32_t x = i; x < i + s; ++x)
        for (uint32_t y = j; y < j + s; ++y)
            if (!m_grid[x][y]) return false;
    return true;
}

void piece::rotate() {
    bool** temp = new bool*[m_side];
    for (uint32_t i = 0; i < m_side; ++i)
        temp[i] = new bool[m_side];

    for (uint32_t i = 0; i < m_side; ++i) {
        for (uint32_t j = 0; j < m_side; ++j) {
            temp[j][m_side - 1 - i] = m_grid[i][j];
        }
    }

    // Dealloca griglia attuale
    for (uint32_t i = 0; i < m_side; ++i)
        delete[] m_grid[i];
    delete[] m_grid;

    m_grid = temp;
}

void piece::cut_row(uint32_t i) {
    if (i >= m_side || !m_grid)
        throw tetris_exception("cut_row: index out of bounds");

    delete[] m_grid[i];

    for (int j = i; j > 0; --j)
        m_grid[j] = m_grid[j - 1];

    m_grid[0] = new bool[m_side];
    for (uint32_t j = 0; j < m_side; ++j)
        m_grid[0][j] = false;
}


void piece::print_ascii_art(std::ostream& os) const {
    for (int i = m_side - 1; i >= 0; --i) {  // stampa dall’alto verso il basso
        for (uint32_t j = 0; j < m_side; ++j) {
            if (m_grid[i][j]) {
                os << "\033[48;5;" << int(m_color) << "m" << ' ' << "\033[m";
            } else {
                os << ' ';
            }
        }
        os << '\n';
    }
}

static void write_quadrant(std::ostream& os, piece const& p, uint32_t i, uint32_t j, uint32_t size) {
    if (size == 0)
        throw tetris_exception("Invalid quadrant size");

    if (size == 1) {
        os << (p(i, j) ? "()" : "[]");
        return;
    }

    if (p.full(i, j, size)) {
        os << "()";
        return;
    }
    if (p.empty(i, j, size)) {
        os << "[]";
        return;
    }

    uint32_t half = size / 2;
    os << '(';
    write_quadrant(os, p, i, j, half);              // TL
    write_quadrant(os, p, i, j + half, half);       // TR
    write_quadrant(os, p, i + half, j, half);       // BL
    write_quadrant(os, p, i + half, j + half, half); // BR
    os << ')';
}

std::ostream& operator<<(std::ostream& os, piece const& p) {
    os << p.side() << ' ' << p.color() << ' ';
    if (p.side() == 0) {
        os << "[]";
        return os;
    }
    write_quadrant(os, p, 0, 0, p.side());
    return os;
}


static void parse_quadrant_helper(std::istream& is, piece& temp, uint32_t i, uint32_t j, uint32_t size) {
    if (size == 0)
        throw tetris_exception("Invalid size");

    char c1, c2;
    
    is >> std::ws;
    if (is.eof())
        throw tetris_exception("Unexpected EOF");

    if (!is.get(c1))
        throw tetris_exception("Unexpected EOF reading quadrant");
    
    if (c1 == '(') {
        // Potrebbe essere () (pieno) o una struttura ricorsiva
        is >> std::ws;
        if (is.eof())
            throw tetris_exception("Unexpected EOF after '('");
        
        char next = is.peek();
        if (next == ')') {
            // È () - quadrante pieno
            is.get(c2);  // consuma ')'
            // Riempie tutto il quadrante
            for (uint32_t x = i; x < i + size; x++) {
                for (uint32_t y = j; y < j + size; y++) {
                    temp(x, y) = true;
                }
            }
        } else {
            // struttura ricorsiva (TL TR BL BR)
            uint32_t half = size / 2;
            parse_quadrant_helper(is, temp, i, j, half);              // TL
            parse_quadrant_helper(is, temp, i, j + half, half);       // TR
            parse_quadrant_helper(is, temp, i + half, j, half);       // BL
            parse_quadrant_helper(is, temp, i + half, j + half, half); // BR
            
            is >> std::ws;
            if (!is.get(c2) || c2 != ')')
                throw tetris_exception("Expected ')' after recursive structure");
        }
    } else if (c1 == '[') {
        // Deve essere [] - quadrante vuoto
        if (!is.get(c2) || c2 != ']')
            throw tetris_exception("Expected ']' after '['");
        
        // Non fa nulla - il quadrante è già false per default
    } else {
        std::string msg = "Expected '(' or '[', got '";
        msg += c1;
        msg += "'";
        throw tetris_exception(msg);
    }
}

std::istream& operator>>(std::istream& is, piece& p) {
    uint32_t s;
    int color;

    if (!(is >> s >> color)) {
        throw tetris_exception("Invalid piece header");
    }

    if (s == 0 || (s & (s - 1)) != 0)
        throw tetris_exception("Invalid piece side");

    if (color <= 0 || color > 255)
        throw tetris_exception("Invalid piece color");

    piece temp(s, static_cast<uint8_t>(color));

    // Inizia il parsing dall'intero pezzo
    parse_quadrant_helper(is, temp, 0, 0, s);

    is >> std::ws;
    if (is.peek() == '(' || is.peek() == '[')
        throw tetris_exception("Extra data after piece");

    p = std::move(temp);
    return is;
}


bool piece::operator==(piece const& rhs) const {
    if (m_side != rhs.m_side || m_color != rhs.m_color)
        return false;

    for (uint32_t i = 0; i < m_side; ++i)
        for (uint32_t j = 0; j < m_side; ++j)
            if (m_grid[i][j] != rhs.m_grid[i][j])
                return false;

    return true;
}

bool piece::operator!=(piece const& rhs) const {
    return !(*this == rhs);
}
