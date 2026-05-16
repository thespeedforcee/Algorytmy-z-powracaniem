#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <queue>
#include <numeric>

using namespace std;

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

// DEC - Cykl Eulera
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

// DHC - Cykl Hamiltona
bool DHC_Hamilton(const vector<vector<int>>& mat, int n, bool skierowany) {
    vector<int> in_deg(n + 1, 0), out_deg(n + 1, 0);
    obliczStopnie(mat, n, skierowany, in_deg, out_deg);

    if (!skierowany) {
        // Twierdzenie Orego (warunek wystarczajacy)
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                if (mat[i][j] == 0) {
                    if (out_deg[i] + out_deg[j] < n) return false;
                }
            }
        }
        return true;
    } else {
        // Twierdzenie Woodalla (warunek wystarczajacy)
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (i != j && mat[i][j] == 0) {
                    if (out_deg[i] + in_deg[j] < n) return false;
                }
            }
        }
        return true;
    }
}

void wyswietlMacierz(const vector<vector<int>>& mat, int n) {
    cout << "\nMacierz grafu:\n";
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            cout << mat[i][j] << " ";
        }
        cout << "\n";
    }
}

vector<vector<int>> wczytajZPliku(int& n, bool& prosty, bool skierowany) {
    string nazwa;
    cout << "Podaj nazwe pliku (np. dane2.txt): ";
    cin >> nazwa;
    ifstream plik(nazwa);
    prosty = true;
    
    if (!plik.is_open()) {
        cout << "[BLAD] Nie udalo sie otworzyc pliku!\n";
        return {};
    }
    
    int m;
    if (!(plik >> n >> m)) {
        cout << "[BLAD] Zly format pliku.\n";
        return {};
    }

    vector<vector<int>> mat(n + 1, vector<int>(n + 1, 0));
    for (int i = 0; i < m; i++) {
        int u, v;
        if (plik >> u >> v) {
            if (u < 1 || v < 1 || u > n || v > n) {
                cout << "[BLAD] Wierzcholki poza zakresem: " << u << " -> " << v << "\n";
                continue;
            }
            if (u == v) prosty = false; // Petla wlasna
            if (mat[u][v] == 1) prosty = false; // Krawedz wielokrotna
            
            mat[u][v] = 1;
            if (!skierowany) mat[v][u] = 1;
        }
    }
    plik.close();
    return mat;
}

vector<vector<int>> generujLosowy(int n, int s_procent, bool skierowany) {
    vector<vector<int>> mat(n + 1, vector<int>(n + 1, 0));
    int max_krawedzi = skierowany ? n * (n - 1) : n * (n - 1) / 2;
    int docelowe_krawedzie = (max_krawedzi * s_procent) / 100;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, n);

    int dodane = 0;
    while (dodane < docelowe_krawedzie) {
        int u = dist(gen);
        int v = dist(gen);
        if (u != v && mat[u][v] == 0) {
            mat[u][v] = 1;
            if (!skierowany) mat[v][u] = 1;
            dodane++;
        }
    }
    return mat;
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
    vector<vector<int>> macierz;
    bool prosty = true;

    if (wejscie == 1) {
        macierz = wczytajZPliku(n, prosty, skierowany);
        if (macierz.empty()) return;
        if (!prosty) cout << "[UWAGA] Graf ma petle wlasne lub krawedzie wielokrotne.\n";
    } else {
        int s;
        cout << "Podaj liczbe wierzcholkow (n): ";
        cin >> n;
        cout << "Podaj nasycenie grafu (w %): ";
        cin >> s;
        macierz = generujLosowy(n, s, skierowany);
    }

    wyswietlMacierz(macierz, n);

    auto start = chrono::high_resolution_clock::now();
    bool wynik = false;
    
    if (problem == 1) {
        wynik = DEC_Euler(macierz, n, skierowany);
        cout << "\n[WYNIK DEC]: Graf " << (wynik ? "SPELNIA" : "NIE SPELNIA") << " warunkow na cykl Eulera.\n";
    } else {
        wynik = DHC_Hamilton(macierz, n, skierowany);
        cout << "\n[WYNIK DHC]: Graf " << (wynik ? "SPELNIA" : "NIE SPELNIA") << " zbadanego warunku wystarczajacego na cykl Hamiltona.\n";
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    cout << "Czas wykonania decyzyjnego: " << duration.count() << " ms\n";
}

void trybEksperymentalny() {
    cout << "\n--- TRYB EKSPERYMENTALNY ---\n";
    cout << "Tutaj program w petli wygeneruje wyniki do tabelek zgodnie z wytycznymi[cite: 24].\n";
    
    vector<int> n_wartosci = {1000, 2000, 3000}; // Przykladowe wartosci, dostosuj do wymogow.
    vector<int> s_wartosci = {10, 50, 90};
    
    cout << "Przyklad dzialania w tle (dane do Excela)...\n";
    for(int n : n_wartosci) {
        for(int s : s_wartosci) {
            double suma_czasu = 0;
            for(int i = 0; i < 10; i++) { // 10 prob
                vector<vector<int>> mat = generujLosowy(n, s, false);
                auto start = chrono::high_resolution_clock::now();
                DEC_Euler(mat, n, false); // Tylko DEC jako przyklad
                auto end = chrono::high_resolution_clock::now();
                suma_czasu += chrono::duration<double, milli>(end - start).count();
            }
            cout << "n=" << n << ", s=" << s << "% | Sredni czas DEC (Nieskierowany): " << suma_czasu/10.0 << " ms\n";
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