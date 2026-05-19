#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <queue>
#include <numeric>
#include <utility>
#include <cmath>

using namespace std;

struct MacierzGrafu {
    int n;
    // Macierz o wymiarach (n+1) x (n+5).
    // Kolumny n+1, n+2, n+3, n+4 to początki list LN, LP, LB, LC.
    vector<vector<int>> macierz; 

    MacierzGrafu(int wierzcholki) {
        n = wierzcholki;
        macierz.assign(n + 1, vector<int>(n + 5, 0));
    }

    void zbuduj(const vector<vector<int>>& mat_sasiedztwa, bool skierowany) {
        for (int i = 1; i <= n; i++) {
            vector<int> ln, lp, lb, lc;
            
            // Podział krawędzi zgodnie z logiką ze zdjęcia
            for (int j = 1; j <= n; j++) {
                bool krawedz_w_przod = (mat_sasiedztwa[i][j] == 1);
                bool krawedz_w_tyl = (mat_sasiedztwa[j][i] == 1);

                if (skierowany) {
                    if (krawedz_w_przod && krawedz_w_tyl) lc.push_back(j);       // Cykl 0-1 (wzajemne)
                    else if (krawedz_w_przod && !krawedz_w_tyl) ln.push_back(j); // Tylko następnik
                    else if (!krawedz_w_przod && krawedz_w_tyl) lp.push_back(j); // Tylko poprzednik
                    else lb.push_back(j);                                        // Brak incydencji
                } else {
                    // W grafie nieskierowanym wszystkie krawędzie lądują w LN
                    if (i == j) {
                        if (krawedz_w_przod) lc.push_back(j); else lb.push_back(j);
                    } else {
                        if (krawedz_w_przod) ln.push_back(j); else lb.push_back(j);
                    }
                }
            }

            // Sprytna funkcja ładująca wskaźniki do komórek macierzy
            auto link_list = [&](const vector<int>& list, int offset, int sign, int start_col) {
                if (list.empty()) {
                    macierz[i][start_col] = 0; // Brak krawędzi w danej liście
                } else {
                    macierz[i][start_col] = list[0];
                    for (size_t k = 0; k < list.size(); k++) {
                        int curr = list[k];
                        // Jeśli to ostatni element, wskazuje na siebie
                        int next_val = (k + 1 < list.size()) ? list[k + 1] : curr;
                        macierz[i][curr] = sign * (next_val + offset);
                    }
                }
            };

            link_list(ln, 0, 1, n + 1);       // LN - startuje w kolumnie N+1
            link_list(lp, n, 1, n + 2);       // LP - startuje w kolumnie N+2 (wartości + N)
            link_list(lb, 0, -1, n + 3);      // LB - startuje w kolumnie N+3 (wartości ujemne)
            link_list(lc, 2 * n, 1, n + 4);   // LC - startuje w kolumnie N+4 (wartości + 2N)
        }
    }

    void getDegrees(int u, int& out_deg, int& in_deg) const {
        out_deg = 0; in_deg = 0;
        
        // Zliczanie z Listy Następników (LN)
        int curr = macierz[u][n + 1];
        while (curr != 0) {
            out_deg++;
            int val = macierz[u][curr];
            if (val == curr) break; // Koniec listy
            curr = val;
        }
        // Zliczanie z Listy Poprzedników (LP)
        curr = macierz[u][n + 2];
        while (curr != 0) {
            in_deg++;
            int val = macierz[u][curr] - n;
            if (val == curr) break;
            curr = val;
        }
        // Zliczanie z Listy Cykli (LC) - cykle 0-1 to krawędzie w obie strony
        curr = macierz[u][n + 4];
        while (curr != 0) {
            out_deg++; in_deg++;
            int val = macierz[u][curr] - 2 * n;
            if (val == curr) break;
            curr = val;
        }
    }
};

void obliczStopnie(const vector<vector<int>>& mat, int n, bool skierowany, vector<int>& in_deg, vector<int>& out_deg) {
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (mat[i][j] == 1) {
                out_deg[i]++; //wychodzace kraw
                in_deg[j]++; //wchodzace kraw
            }
        }
    }
}

bool czySpojnyNieskierowany(const vector<vector<int>>& mat, int n, const vector<int>& stopnie) { //wykorzystuje algorytm BFS
    int start = 0; 
    for (int i = 1; i <= n; i++) {
        if (stopnie[i] > 0) {
            start = i;
            break;
        }
    }
    if (start == 0) return true; // Graf pusty (bez krawedzi) trywialnie spelnia warunek

    vector<bool> odwiedzone(n + 1, false);
    queue<int> q;
    q.push(start);
    odwiedzone[start] = true;

    while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (int u = 1; u <= n; u++) {
            if (mat[v][u] == 1 && !odwiedzone[u]) {
                odwiedzone[u] = true;
                q.push(u);
            }
        }
    }

    for (int i = 1; i <= n; i++) {
        if (stopnie[i] > 0 && !odwiedzone[i]) return false; //jesli jakiegos wierzcholka nie udalo sie odwiedzic nie jest spojny
    }
    return true;
}

// Sprawdzanie silnej spojnosci dla grafu skierowanego
bool czySilnieSpojny(const vector<vector<int>>& mat, int n, const vector<int>& out_deg, const vector<int>& in_deg) {
    int start = 0;
    for (int i = 1; i <= n; i++) {
        if (out_deg[i] > 0 || in_deg[i] > 0) {
            start = i;
            break;
        }
    }
    if (start == 0) return true;

    //BFS na oryginalnym grafie
    vector<bool> odwiedzone(n + 1, false);
    queue<int> q;
    q.push(start);
    odwiedzone[start] = true;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u = 1; u <= n; u++) {
            if (mat[v][u] == 1 && !odwiedzone[u]) {
                odwiedzone[u] = true; q.push(u);
            }
        }
    }
    //jesli nie udalo sie odwiedzic wszystkich wierzcholkow
    for (int i = 1; i <= n; i++) if ((out_deg[i] > 0 || in_deg[i] > 0) && !odwiedzone[i]) return false;

    //BFS na transponowanym grafie
    fill(odwiedzone.begin(), odwiedzone.end(), false);
    q.push(start);
    odwiedzone[start] = true;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u = 1; u <= n; u++) {
            if (mat[u][v] == 1 && !odwiedzone[u]) { // odwrocony kierunek
                odwiedzone[u] = true; q.push(u);
            }
        }
    }
    //jesli nie udalo sie odwiedzic wszystkich wierzcholkow
    for (int i = 1; i <= n; i++) if ((out_deg[i] > 0 || in_deg[i] > 0) && !odwiedzone[i]) return false;

    return true;
}

// DEC - Cykl Eulera - macierz sasiedztwa
bool DEC_Euler(const vector<vector<int>>& mat, int n, bool skierowany) {
    vector<int> in_deg(n + 1, 0), out_deg(n + 1, 0);
    obliczStopnie(mat, n, skierowany, in_deg, out_deg);

    if (!skierowany) {
        // warunki dla nieskierowanego: wszystkie stopnie parzyste i graf spojny
        for (int i = 1; i <= n; i++) {
            if (out_deg[i] % 2 != 0) return false;
        }
        return czySpojnyNieskierowany(mat, n, out_deg);
    } else {
        // Waruneki dla skierowanego: wchodzace = wychodzace kraw i silnie spojny
        for (int i = 1; i <= n; i++) {
            if (in_deg[i] != out_deg[i]) return false;
        }
        return czySilnieSpojny(mat, n, out_deg, in_deg);
    }
}

// DHC - Cykl Hamiltona - macierz grafu
bool DHC_Hamilton(const MacierzGrafu& mg, bool skierowany) {
    int n = mg.n;
    
    // Sprawdzamy twierdzenia dla każdej pary (u, v) braku incydencji
    for (int u = 1; u <= n; u++) {
        int out_u, in_u;
        mg.getDegrees(u, out_u, in_u); // Wyciągamy stopień u

        int curr = mg.macierz[u][n + 3]; // Zaczynamy czytać Listę Braku Incydencji (LB) dla u
        
        while (curr != 0) {
            int v = curr; // v jest wierzchołkiem z którym u nie ma krawędzi
            
            int out_v, in_v;
            mg.getDegrees(v, out_v, in_v); // Wyciągamy stopień v

            if (!skierowany) {
                // Twierdzenie Orego
                if (out_u + out_v < n) return false;
            } else {
                // Twierdzenie Woodalla
                if (out_u + in_v < n) return false;
            }

            // Przejście do następnego braku krawędzi 
            int val = -mg.macierz[u][curr];
            if (val == curr) break; // koniec listy LB
            curr = val;
        }
    }
    return true;
}

vector<pair<int, int>> wczytajZPliku(int& n, bool skierowany) {
    string nazwa;
    cout << "Podaj nazwe pliku: ";
    cin >> nazwa;
    ifstream plik(nazwa);
    
    if (!plik.is_open()) {
        cout << "[BLAD] Nie udalo sie otworzyc pliku!\n";
        return {};
    }
    
    int m;
    if (!(plik >> n >> m)) {
        cout << "[BLAD] Zly format pliku.\n";
        return {};
    }

    vector<pair<int, int>> krawedzie;
    for (int i = 0; i < m; i++) {
        int u, v;
        if (plik >> u >> v) {
            if (u >= 1 && v >= 1 && u <= n && v <= n && u != v) {
                krawedzie.push_back({u, v});
            }
        }
    }
    return krawedzie;
}

vector<pair<int, int>> generujLosowy(int n, int s_procent, bool skierowany) {
    vector<pair<int, int>> krawedzie;
    vector<vector<int>> tmp(n+1, vector<int>(n+1,0));
    int max_krawedzi = skierowany ? n * (n - 1) : n * (n - 1) / 2;
    int docelowe_krawedzie = (max_krawedzi * s_procent) / 100;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, n);

    int dodane = 0;
    while (dodane < docelowe_krawedzie) {
        int u = dist(gen);
        int v = dist(gen);
        if (u != v && tmp[u][v] == 0) {
            tmp[u][v] = 1;
            krawedzie.push_back({u, v});
            if (!skierowany) tmp[v][u] = 1;
            dodane++;
        }
    }
    return krawedzie;
}

void wyswietlMacierzSasiedztwa(const vector<vector<int>>& mat, int n) {
    cout << "\nmacierz sasiedztwa\n";
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            cout << mat[i][j] << " ";
        }
        cout << "\n";
    }
}

void wyswietlMacierzGrafu(const MacierzGrafu& mg) {
    cout << "\nmacierz grafu\n";
    
    // Wypisanie nagłówków kolumn (1..n oraz LN, LP, LB, LC)
    cout << "i\\j\t";
    for (int j = 1; j <= mg.n + 4; j++) {
        if (j <= mg.n) cout << j << "\t";
        else if (j == mg.n + 1) cout << "LN\t";
        else if (j == mg.n + 2) cout << "LP\t";
        else if (j == mg.n + 3) cout << "LB\t";
        else if (j == mg.n + 4) cout << "LC\t";
    }
    cout << "\n";

    // Wypisanie wierszy z danymi
    for (int i = 1; i <= mg.n; i++) {
        cout << i << " |\t";
        for (int j = 1; j <= mg.n + 4; j++) {
            cout << mg.macierz[i][j] << "\t";
        }
        cout << "\n";
    }
}

void trybDemonstracyjny() {
    int problem, typ, wejscie;
    cout << "\ntryb demo\n";
    cout << "1. Cykl Eulera (DEC)\n2. Cykl Hamiltona (DHC)\nWybierz: ";
    cin >> problem;
    cout << "1. Graf Nieskierowany\n2. Graf Skierowany\nWybierz: ";
    cin >> typ;
    cout << "1. Wczytaj z pliku\n2. Generuj losowy\nWybierz: ";
    cin >> wejscie;

    bool skierowany = (typ == 2);
    int n = 0;
    vector<pair<int, int>> krawedzie;

    if (wejscie == 1) {
        krawedzie = wczytajZPliku(n, skierowany);
    } else {
        int s;
        cout << "Podaj liczbe wierzcholkow (n): ";
        cin >> n;
        cout << "Podaj nasycenie grafu (w %): ";
        cin >> s;
        krawedzie = generujLosowy(n, s, skierowany);
    }
    if (n == 0) return;

    auto start = chrono::high_resolution_clock::now();
    bool wynik = false;
    
    if (problem == 1) {
        vector<vector<int>> macierz_sasiedztwa(n + 1, vector<int>(n + 1, 0));
        for (auto& k : krawedzie) {
            macierz_sasiedztwa[k.first][k.second] = 1;
            if (!skierowany) macierz_sasiedztwa[k.second][k.first] = 1;
        }
        wyswietlMacierzSasiedztwa(macierz_sasiedztwa, n);
        wynik = DEC_Euler(macierz_sasiedztwa, n, skierowany);
        cout << "\n[WYNIK DEC]: Graf " << (wynik ? "SPELNIA warunki" : "NIE SPELNIA warunkow") << " na cykl Eulera.\n";
        
    } else if (problem == 2) {
        vector<vector<int>> mat_sasiedztwa(n + 1, vector<int>(n + 1, 0));
        for (auto& k : krawedzie) {
            mat_sasiedztwa[k.first][k.second] = 1;
            if (!skierowany) mat_sasiedztwa[k.second][k.first] = 1;
        }
        
        MacierzGrafu mg(n);
        mg.zbuduj(mat_sasiedztwa, skierowany); 
        
        wynik = DHC_Hamilton(mg, skierowany);
        cout << "\n[WYNIK DHC]: Graf " << (wynik ? "SPELNIA zbadany warunek" : "NIE SPELNIA zbadanego warunku") << " na cykl Hamiltona.\n";
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    cout << "Czas wykonania decyzyjnego: " << duration.count() << " ms\n";
}

void trybEksperymentalny() {
    cout << "\n--- TRYB EKSPERYMENTALNY ---\n";
    cout << "Generowanie danych pomiarowych (srednia z 10 uruchomien i odchylenie standardowe).\n";
    
    vector<int> n_wartosci_DEC = {1000, 1500, 2000, 2500, 3000}; 
    vector<int> n_wartosci_DHC = {100, 200, 300, 400, 500};      
    
    vector<int> s_wartosci = {10, 20, 30, 40, 50, 60, 70, 80, 90}; 

    cout << "           TESTY DEC (Cykl Eulera)           \n";
    for(int n : n_wartosci_DEC) {
        for(int s : s_wartosci) {
            for(bool skierowany : {false, true}) { // Automatycznie testuje oba typy 
                vector<double> czasy;
                double suma_czasu = 0;

                for(int i = 0; i < 10; i++) { // 10 prob 
                    vector<pair<int, int>> krawedzie = generujLosowy(n, s, skierowany);
                    
                    // Budowa macierzy sasiedztwa tylko dla algorytmu DEC
                    vector<vector<int>> macierz_sasiedztwa(n + 1, vector<int>(n + 1, 0));
                    for (auto& k : krawedzie) {
                        macierz_sasiedztwa[k.first][k.second] = 1;
                        if (!skierowany) macierz_sasiedztwa[k.second][k.first] = 1;
                    }

                    auto start = chrono::high_resolution_clock::now();
                    DEC_Euler(macierz_sasiedztwa, n, skierowany); 
                    auto end = chrono::high_resolution_clock::now();
                    
                    double czas = chrono::duration<double, milli>(end - start).count();
                    czasy.push_back(czas);
                    suma_czasu += czas;
                }

                // Obliczanie sredniej i odchylenia standardowego 
                double srednia = suma_czasu / 10.0;
                double wariancja = 0;
                for(double czas : czasy) { wariancja += pow(czas - srednia, 2); }
                double odchylenie = sqrt(wariancja / 10.0); 

                cout << "DEC | " << (skierowany ? "Skierowany   " : "Nieskierowany") 
                     << " | n=" << n << ", s=" << s << "% | Sredni czas: " << srednia << " ms | Odch. std: " << odchylenie << " ms\n";
            }
        }
    }

    cout << "         TESTY DHC (Cykl Hamiltona)          \n";
    for(int n : n_wartosci_DHC) {
        for(int s : s_wartosci) {
            for(bool skierowany : {false, true}) { 
                vector<double> czasy;
                double suma_czasu = 0;

                for(int i = 0; i < 10; i++) { 
                    vector<pair<int, int>> krawedzie = generujLosowy(n, s, skierowany);
                    vector<vector<int>> mat_sasiedztwa(n + 1, vector<int>(n + 1, 0));
                    for (auto& k : krawedzie) {
                        mat_sasiedztwa[k.first][k.second] = 1;
                        if (!skierowany) mat_sasiedztwa[k.second][k.first] = 1;
                    }

                    MacierzGrafu mg(n);
                    mg.zbuduj(mat_sasiedztwa, skierowany);

                    auto start = chrono::high_resolution_clock::now();
                    DHC_Hamilton(mg, skierowany); 
                    auto end = chrono::high_resolution_clock::now();
                    
                    double czas = chrono::duration<double, milli>(end - start).count();
                    czasy.push_back(czas);
                    suma_czasu += czas;
                }

                double srednia = suma_czasu / 10.0;
                double wariancja = 0;
                for(double czas : czasy) { wariancja += pow(czas - srednia, 2); }
                double odchylenie = sqrt(wariancja / 10.0); 

                cout << "DHC | " << (skierowany ? "Skierowany   " : "Nieskierowany") 
                     << " | n=" << n << ", s=" << s << "% | Sredni czas: " << srednia << " ms | Odch. std: " << odchylenie << " ms\n";
            }
        }
    }
}

int main() {
    int opcja;
    while (true) {
        cout << "1. Tryb Demonstracyjny\n";
        cout << "2. Tryb Eksperymentalny (Szkielet)\n";
        cout << "0. Wyjdz\n";
        cout << "Wybierz opcje: ";
        if (!(cin >> opcja)) {
            cout << "Zle wejscie!\n";
            break;
        }

        if (opcja == 1) trybDemonstracyjny();
        else if (opcja == 2) trybEksperymentalny();
        else if (opcja == 0) break;
        else cout << "Nieznana opcja.\n";
    }
    return 0;
}