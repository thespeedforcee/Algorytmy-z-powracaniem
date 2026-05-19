#include<iostream>
#include<vector>
#include<fstream>
#include<chrono>
#include<limits>
#include <cstdlib>
#include <ctime>
#include<queue>
#include<cmath>
#include<random>
using namespace std;

int licznik_backtrackow = 0;
vector<int> znaleziona_sciezka;
vector<bool> odwiedzone;

struct MacierzGrafu {
    int n;
    // Macierz o wymiarach (n+1) x (n+5).
    // Kolumny n+1, n+2, n+3, n+4 to początki list LN, LP, LB, LC.
    vector<vector<int>> macierz; 

    MacierzGrafu(int wierzcholki) {
        n = wierzcholki;
        macierz.assign(n + 1, vector<int>(n + 5, 0));
    }
    //buduje z macierzy sasiedztwa macierz grafu
    void zbuduj(const vector<vector<int>>& mat_sasiedztwa, bool skierowany) {
        for (int i = 1; i <= n; i++) {
            vector<int> ln, lp, lb, lc;
            
            for (int j = 1; j <= n; j++) {
                bool krawedz_w_przod = (mat_sasiedztwa[i][j] == 1);
                bool krawedz_w_tyl = (mat_sasiedztwa[j][i] == 1);

                if (skierowany) {
                    if (krawedz_w_przod && krawedz_w_tyl) lc.push_back(j);       // Cykl 0-1 (wzajemne)
                    else if (krawedz_w_przod && !krawedz_w_tyl) ln.push_back(j); // Tylko in
                    else if (!krawedz_w_przod && krawedz_w_tyl) lp.push_back(j); // Tylko out
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
                    macierz[i][start_col] = list[0]; //glowa listy wskazuje na pierwszy element
                    for (size_t k = 0; k < list.size(); k++) {
                        int curr = list[k];
                        // Jeśli to ostatni element, wskazuje na siebie
                        int next_val = (k + 1 < list.size()) ? list[k + 1] : curr;
                        // Mnożymy przez znak (ujemne dla LB) i dodajemy offset, by móc odróżnić z jakiej listy czytamy
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
            start = i; //szuka 1 wierzcholka
            break;
        }
    }
    if (start == 0) return true; // Graf pusty (bez krawedzi) trywialnie spelnia warunek

    vector<bool> odwiedzone(n + 1, false); //bfs
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
    
    // Sprawdzamy twierdzenia dla każdej pary (u, v) braku krawedzi
    for (int u = 1; u <= n; u++) {
        int out_u, in_u;
        mg.getDegrees(u, out_u, in_u); // wyciagniecie st u

        int curr = mg.macierz[u][n + 3]; // zaczynamy od LB zeby nie iterowac po calej tab
        
        while (curr != 0) {
            int v = curr; // v jest wierzchołkiem z którym u nie ma krawędzi
            
            int out_v, in_v;
            mg.getDegrees(v, out_v, in_v); // wyciagniecie st v

            if (!skierowany) {
                // Twierdzenie Orego, graf hamiltonowski
                if (out_u + out_v < n) return false;
            } else {
                // Twierdzenie Woodalla, wychodzace u + wchodzace v >= n
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

int wczytajLiczbe(int min, int max) {
    int wybor;
    while (true) {
        if (cin >> wybor && wybor >= min && wybor <= max) {
            return wybor; 
        }
        cout << "Bledny wybor! Podaj liczbe od " << min << " do " << max << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void wczytajplik(string nazwa_pliku, vector<vector<int>>& mac, int& n, int& m, bool czy_skier,int rodzaj) {
    ifstream plik(nazwa_pliku);
    
    if (!plik.good()) {
        cout << "Błąd! Nie można otworzyć pliku." << endl;
        return;
    }
    if (!(plik >> n >> m)) {
        cout << "Blad! Plik jest pusty lub ma zly format." << endl;
        n = 0; return;
    }
    if(rodzaj==1)
    {
        mac.assign(n+1, vector<int>(n+1, 0)); //macierz sasiedztwa nxn wypelniona zerami
    
        for (int i = 0; i < m; i++) {
            int u, v;
            if(plik >> u >> v){
                if (u >= 1 && v >= 1 && u <= n && v <= n && u != v) {
                    mac[u][v] = 1;
                    if (!czy_skier) {
                        mac[v][u] = 1; //dla nieskierowanego
                    }
                } else {
                    cout << "[Ostrzezenie] Zignorowano bledna krawedz w pliku: " << u << " - " << v << endl;
                }
            }
        }
    }else if (rodzaj==2){
        mac.assign(n+1, vector<int>(n + 5, 0)); //wektor nxn+4 bo te 4 kolumny to lista nastepnikow poprzendikow , braku incydencji i cykli
        int Ln = n+1; //bo od 0 do n sa kolumny z wierzcholkami
        for (int i = 0; i < m; i++) {
            int u, v;
            if(plik >> u >> v){
                if (u >= 1 && v >= 1 && u <= n && v <= n && u != v) {
                    if (mac[u][Ln] == 0) {
                        mac[u][Ln] = v;
                        mac[u][v] = v; //znak stopu (wtedy gdy zrobimy petle to sie zatrzymuje)
                    } else {
                        int obecny = mac[u][Ln];
                        while (mac[u][obecny] != obecny) {
                            obecny = mac[u][obecny];
                        }
                        mac[u][obecny] = v;
                        mac[u][v] = v;
                    }
                    if (!czy_skier) {
                        if (mac[v][Ln] == 0) {
                            mac[v][Ln] = u;
                            mac[v][u] = u;
                        } else {
                            int obecny = mac[v][Ln];
                            while (mac[v][obecny] != obecny) obecny = mac[v][obecny];
                            mac[v][obecny] = u;
                            mac[v][u] = u;
                        }
                    }
                }else{
                    cout << "[Ostrzezenie] Zignorowano bledna krawedz w pliku: " << u << " - " << v << endl;
                }  
            }
        }
    }
    plik.close();
}
vector<int> sasiedzi(int wierzcholek,vector<vector<int>>&mac,int n)
{
    vector<int> s;
    int kolumna_startowa = n+1; //zaczynam od kolumny Ln o ind n+1
    
    int wartosc = mac[wierzcholek][kolumna_startowa];
    
    if (wartosc == 0) {
        return s; //pusty wektor jesli nie ma sasiadow
    }
    
    s.push_back(wartosc);
    int obecna_kolumna = wartosc;
    
    while (true) {
        wartosc = mac[wierzcholek][obecna_kolumna];
        
        if (wartosc == obecna_kolumna) {
            break; //warunek stopu wrocilo na start
        } else {
            s.push_back(wartosc);
            obecna_kolumna = wartosc;
        }
    }
    
    return s;
}
bool SEC(int wierzcholek,vector<vector<int>>& mac,int n,int m,bool czyskier,int odwiedzone_kr,int start) //m - liczba krawedzi grafu
{
    if(wierzcholek==start && odwiedzone_kr==m)
    {
        znaleziona_sciezka.push_back(wierzcholek);
        return true; // da sie przejsc reszte grafu i wrocic na start
    }
    for(int i=1;i<=n;i++)
    {
        if(mac[wierzcholek][i]==1)
        {
            mac[wierzcholek][i] = 0; //odcinam sciezke zeby przypadkiem nie wrocic
            if(!czyskier){mac[i][wierzcholek]=0;}

            if(SEC(i,mac,n,m,czyskier,odwiedzone_kr+1,start)) //czy true
            {
                znaleziona_sciezka.push_back(wierzcholek);
                return true;
            }

            licznik_backtrackow++;
            mac[wierzcholek][i] = 1;
            if(!czyskier)
            {
                mac[i][wierzcholek] = 1;
            }
        }
    }
    return false;
}

vector<int> pobierz_nastepnikow(int wierzcholek, const MacierzGrafu& mg) {
    vector<int> s;
    int n = mg.n;
    
    // Z listy LN (Lista Następników / Krawędzie skierowane w przód)
    int curr = mg.macierz[wierzcholek][n + 1];
    while (curr != 0) {
        s.push_back(curr);
        int next_val = mg.macierz[wierzcholek][curr];
        if (next_val == curr) break; // Warunek stopu
        curr = next_val;
    }
    
    // Z listy LC (Lista Cykli / Krawędzie wzajemnie skierowane)
    curr = mg.macierz[wierzcholek][n + 4];
    while (curr != 0) {
        s.push_back(curr);
        int next_val = mg.macierz[wierzcholek][curr] - 2 * n; // Zdejecie offset z konstruktora
        if (next_val == curr) break;
        curr = next_val;
    }
    
    return s;
}

bool SHC(int wierzcholek,const MacierzGrafu& mg,int start)
{
    odwiedzone[wierzcholek] = true;
    znaleziona_sciezka.push_back(wierzcholek);
    int n = mg.n;

    // Mam N wierzchołków na trasie? Super, ale czy mogę wrócić do bazy?
    if (znaleziona_sciezka.size() == n) {
        vector<int> sasiedzi_ostatniego = pobierz_nastepnikow(wierzcholek, mg);;
        
        for (int i = 0; i < sasiedzi_ostatniego.size(); i++) {
            if (sasiedzi_ostatniego[i] == start) {
                znaleziona_sciezka.push_back(start); // Zamykam cykl
                return true; // znalezione
            }
        }
    }

    // 2. Szukam drogi dalej
    vector<int> s = pobierz_nastepnikow(wierzcholek, mg);
    
    for (int i = 0; i < s.size(); i++) {
        int sasiad = s[i];
        
        // Zabezpieczenie: idę tylko do nieodwiedzonych!
        if (!odwiedzone[sasiad]) { 
            
            // Rekurencja
            if (SHC(sasiad, mg, start)) {
                return true; 
            }
        }
    }
    
    // 3. BACKTRACKING (Ślepy zaułek)
    licznik_backtrackow++;
    odwiedzone[wierzcholek] = false; // Zmywam krzyż
    znaleziona_sciezka.pop_back();   // Wycofuję się
    
    return false;
}
void wypiszMacierz(const vector<vector<int>>& mac, int n, int rodzaj) {
    
    if(rodzaj==1)
    {
        cout << "\nmacierz sąsiedztwa:" << endl;
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                cout << mac[i][j] << "\t"; // \t żeby kolumny były równe
            }       
            cout << endl;
        }
        cout<< endl;
    }else{
        cout << "\nmacierz grafu:" << endl;
        
        cout << "i\\j\t";
        for (int j = 1; j <= n + 4; j++) {
            if (j <= n) cout << j << "\t";
            else if (j == n + 1) cout << "LN\t";
            else if (j == n + 2) cout << "LP\t";
            else if (j == n + 3) cout << "LB\t";
            else if (j == n + 4) cout << "LC\t";
        }
        cout << endl;
        
        for (int i = 1; i <= n; i++) {
            cout << i << " |\t"; 
            for (int j = 1; j <= n + 4; j++) {
                cout << mac[i][j] << "\t";
            }
            cout << endl;
        }
    } 
}
void generujGraf(vector<vector<int>>& mac, int& n, int& m, bool czy_skier, int rodzaj) {
    cout << "Podaj liczbe wierzcholkow (n): ";
    n = wczytajLiczbe(3, 1000);
    m = 0;
    if (rodzaj == 1) { //inicjuje pusta macierz
        mac.assign(n+1, vector<int>(n+1, 0));
    } else {
        mac.assign(n+1, vector<int>(n + 5, 0));
    }
    
    int Ln = n+1;
    srand(time(0));
    for (int u = 1; u <= n; u++) {
        for (int v = 1; v <= n; v++) {
            if (u == v) continue; //bez petli wlasnych
            if (!czy_skier && u > v) continue; //zeb nie nadpisywac bo w nieskierowanym ta druga krawedz z b do a automatycznie powstaje
            if (rand() % 2==1) {
                m++;
                
                if (rodzaj == 1) { //macierz sasiedztwa
                    mac[u][v] = 1;
                    if (!czy_skier) mac[v][u] = 1;
                } else {
                    if (mac[u][Ln] == 0) { //kiedy jeszzce nie ma wpisanego sasiada
                        mac[u][Ln] = v;
                        mac[u][v] = v;
                    } else {
                        int obecny = mac[u][Ln];
                        while (mac[u][obecny] != obecny) obecny = mac[u][obecny]; //dopoki sie nie zapetli to przechodze przez sasiadow
                        mac[u][obecny] = v;
                        mac[u][v] = v;
                    }
                    if (!czy_skier) { //dla nieskierowanego tez w druga strone
                        if (mac[v][Ln] == 0) {
                            mac[v][Ln] = u;
                            mac[v][u] = u;
                        } else {
                            int obecny = mac[v][Ln];
                            while (mac[v][obecny] != obecny) obecny = mac[v][obecny];
                            mac[v][obecny] = u;
                            mac[v][u] = u;
                        }
                    }
                }
            }
        }
    }
}
int main() {
    vector<vector<int>> macierz;
    int n, m, r;
    bool skierowany;

    cout << "wybor problemu" << endl;
    cout << "1. Cykl Eulera" << endl;
    cout << "2. Cykl Hamiltona" << endl;
    cout << "Wybor: ";
    r = wczytajLiczbe(1, 2);

    cout << "\nwersja problemu" << endl;
    cout << "1. Wersja Decyzyjna (DEC - Euler / DHC - Hamilton)" << endl;
    cout << "2. Wersja Przeszukiwania (SEC - Euler / SHC - Hamilton)" << endl;
    cout << "Wybor: ";
    int wersja = wczytajLiczbe(1, 2);

    cout << "\ntyp grafu" << endl;
    cout << "1. Nieskierowany" << endl;
    cout << "2. Skierowany" << endl;
    cout << "Wybor: ";
    int typ = wczytajLiczbe(1, 2);
    skierowany = (typ == 2); //jesli skierowany to true

    cout << "\nZrodlo danych" << endl;
    cout << "1. Wczytaj graf z pliku (graf.txt)" << endl;
    cout << "2. Wygeneruj graf losowo" << endl;
    cout << "Wybor: ";
    int zrodlo = wczytajLiczbe(1, 2);

    int rodzaj_dla_generatora;
    if(wersja==1)
    {
        rodzaj_dla_generatora = 1;
    }else{
        rodzaj_dla_generatora = r;
    }

    if (zrodlo == 1) {
        string nazwa_pliku;
        cout << "Podaj nazwe pliku (np. graf.txt): ";
        cin >> nazwa_pliku;
        wczytajplik(nazwa_pliku, macierz, n, m, skierowany, rodzaj_dla_generatora);
        if (n == 0) return 0; 
    } else {
        generujGraf(macierz, n, m, skierowany, rodzaj_dla_generatora);
    }

    wypiszMacierz(macierz, n, rodzaj_dla_generatora);

    licznik_backtrackow = 0;
    znaleziona_sciezka.clear();
    odwiedzone.assign(n+1, false);
    bool test = false;

    if (wersja == 1) {
        cout << "Algorytm decyzyjny (Twierdzenia)" << endl;
    }else {
        cout << "Przeszukiwanie z powrotami" << endl;
    }

    MacierzGrafu mg(n);
    if (r == 2) { 
        mg.zbuduj(macierz, skierowany); 
        macierz.clear(); 
        macierz.shrink_to_fit();
    }

    auto start_czasu = chrono::high_resolution_clock::now();
    
    if(wersja==1)
    {
        if (r == 1) {
            test = DEC_Euler(macierz, n, skierowany);}
        else{test = DHC_Hamilton(mg, skierowany);}

    }else if(wersja == 2)
    {
        if (r == 1) {
            test = SEC(1, macierz, n, m, skierowany, 0, 1); 
        }else {
            test = SHC(1, mg, 1);
        }
    }
    
    auto koniec_czasu = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> czas_wykonania = koniec_czasu - start_czasu;

    cout << "wyniki:" << endl;
    if (wersja == 1) {
        if (test) {
            if (r == 1) cout << "Cykl eulera znaleziony! (Warunek konieczny i wystarczajacy spelniony)" << endl;
            else cout << "Graf ma cykl hamiltona. (Spelniony warunek wystarczajacy z tw. Orego/Woodalla)" << endl;
        } else {
            if (r == 1) cout << "Brak cyklu eulera. (Graf nie spelnia warunku decyzyjnego)" << endl;
            else cout << "Graf nie spelnia warunku wystarczajacego. (Moze miec cykl Hamiltona, ale twierdzenie tego nie gwarantuje)" << endl;
        }
    } else { 
        if (test) {
            cout << "ZNALEZIONO CYKL!" << endl;
            cout << "Dlugosc trasy: " << znaleziona_sciezka.size() << endl;
            cout << "Sciezka: ";
            
            if (r == 1) { 
                for (int i = znaleziona_sciezka.size() - 1; i >= 0; i--) {
                    cout << znaleziona_sciezka[i] << " "; 
                }
            } else {
                for (int i = 0; i < znaleziona_sciezka.size(); i++) {
                    cout << znaleziona_sciezka[i] << " ";
                }
            }
            cout << endl;
        } else {
            cout << "Brak cyklu w tym grafie." << endl;
        }
    }
    
    cout << "Liczba backtrackow (cofniec): " << licznik_backtrackow << endl;
    cout << "Czas wykonania: " << czas_wykonania.count() << " ms\n" << endl;

    return 0;
}