# -rocnikovyprojekt
## Popis algoritmu

Cielom projektu je najst vsetky validne Z_k-flow riesenia pre zadany graf pomocou backtracking algoritmu.

Algoritmus funguje nasledovne:
- Pre kazdu hranu v grafe skusa vsetky mozne hodnoty od 1 do k-1 (tzn. nikdy 0).
- Kazdemu rebru priradzujeme umyselne orientaciu u -> v, co umoznuje jednoznacne vypocitat vstupne (inf) a vystupne (outf) toky pre kazdy vrchol.
- Na konci sa kontroluje, ci je pre kazdy vrchol v grafe splnena podmienka rovnosti vstupnych a vystupnych tokov (mod k).
- Ak je podmienka splnena, tok je povazovany za validny a ulozeny medzi vysledky.

Tato orientacia je fixovana na zaciatku a nema vplyv na existenciu nikde nuloveho toku, ale umoznuje efektivne sledovat podmienku kontinuity.
