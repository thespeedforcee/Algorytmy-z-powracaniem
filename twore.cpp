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
    vector<vector<int>> macierz;   // Podstawowa macierz
    vector<vector<int>> LN;        // Lista nastepnikow 
    vector<vector<int>> LP;        // Lista poprzednikow
    vector<pair<int, int>> LB;     // Lista braku incydencji
    vector<int> LC;                // Lista cykli 0-1
    vector<int> in_deg;            // Stopnie wchodzace
    vector<int> out_deg;           // Stopnie wychodzace

    MacierzGrafu(int wierzcholki) {
        n = wierzcholki;
        macierz.assign(n + 1, vector<int>(n + 1, 0));
        LN.resize(n + 1);
        LP.resize(n + 1);
        in_deg.assign(n + 1, 0);
        out_deg.assign(n + 1, 0);
    }

    // Funkcja ktor buduje listy na podstawie zapelnionej macierzy
    void zbudujListy(bool skierowany) {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (macierz[i][j] == 1) {
                    LN[i].push_back(j);
                    LP[j].push_back(i);
                    out_deg[i]++;
                    in_deg[j]++;
                    if (i == j) LC.push_back(i); // Petla wlasna
                } else if (i != j) {
                    if (skierowany || j > i) {
                        LB.push_back({i, j});
                    }
                }
            }
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
    if (!skierowany) {
        // Twierdzenie Orego - iteracja po liscie braku incydencji
        for (const auto& para : mg.LB) {
            int u = para.first;
            int v = para.second;
            if (mg.out_deg[u] + mg.out_deg[v] < mg.n) return false;
        }
        return true;
    } else {
        // Twierdzenie Woodalla - iteracja po brakujacych kraw lb
        for (const auto& para : mg.LB) {
            int u = para.first;
            int v = para.second;
            if (mg.out_deg[u] + mg.in_deg[v] < mg.n) return false;
        }
        return true;
    }
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
    cout << "\n--- REPREZENTACJA: MACIERZ SASIEDZTWA (DLA EULERA) ---\n";
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            cout << mat[i][j] << " ";
        }
        cout << "\n";
    }
}

void wyswietlMacierzGrafu(const MacierzGrafu& mg) {
    cout << "\n--- REPREZENTACJA: MACIERZ GRAFU (DLA HAMILTONA) ---\n";
    cout << "Podstawowa macierz:\n";
    for (int i = 1; i <= mg.n; i++) {
        for (int j = 1; j <= mg.n; j++) {
            cout << mg.macierz[i][j] << " ";
        }
        cout << "\n";
    }
    
    cout << "\nLista Nastepnikow (LN):\n";
    for (int i = 1; i <= mg.n; i++) {
        cout << i << " -> ";
        for (int v : mg.LN[i]) cout << v << " ";
        cout << "\n";
    }

    cout << "\nLista Poprzednikow (LP):\n";
    for (int i = 1; i <= mg.n; i++) {
        cout << i << " -> ";
        for (int v : mg.LP[i]) cout << v << " ";
        cout << "\n";
    }

    cout << "\nLista Braku Incydencji (LB) [wykorzystana w algorytmie DHC]:\n";
    for (const auto& para : mg.LB) {
        cout << para.first << " -> " << para.second << "\n";
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
        cout << "\n[WYNIK DEC]: Graf " << (wynik ? "SPELNIA" : "NIE SPELNIA") << " warunkow na cykl Eulera.\n";
        
    } else if (problem == 2) {
        MacierzGrafu mg(n);
        for (auto& k : krawedzie) {
            mg.macierz[k.first][k.second] = 1;
            if (!skierowany) mg.macierz[k.second][k.first] = 1;
        }
        mg.zbudujListy(skierowany); 
        wyswietlMacierzGrafu(mg);
        cout << "\nZbudowano strukture Macierzy Grafu.\nZnaleziono " << mg.LB.size() << " brakujacych krawedzi (Lista LB).\n";
        wynik = DHC_Hamilton(mg, skierowany);
        cout << "\n[WYNIK DHC]: Graf " << (wynik ? "SPELNIA" : "NIE SPELNIA") << " zbadanego warunku na cykl Hamiltona.\n";
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
                    
                    // Budowa Macierzy Grafu ze strukturą LN, LP i LB specjalnie pod DHC
                    MacierzGrafu mg(n);
                    for (auto& k : krawedzie) {
                        mg.macierz[k.first][k.second] = 1;
                        if (!skierowany) mg.macierz[k.second][k.first] = 1;
                    }
                    mg.zbudujListy(skierowany);

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