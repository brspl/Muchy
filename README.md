# Muchy wyjadające dżem ze słoika

---

## Programowanie systemowe

Wykorzystując mechanizmy systemu GNU/Linux zaprojektować i uruchomić zadany przez prowadzącego zajęcia system
w wersji lokalnej.

Zasady działania systemu:

* Wszystkie procesy systemu uruchamiamy na jednym komputerze. Nie korzystamy z funkcji fork oraz potoków.
* Nie wprowadzamy danych do procesów z plików.
* Procesy systemu wyświetlają następujące informacje np.: "N.koniec pracy".
* Przygotować skrypty (dla klientów) pokazujące prawidłowe działanie systemu.

## 1. Proces serwera

Tylko ten proces ma dostęp do klawiatury. Proces ten po zainstalowaniu wszystkich wspólnych mechanizmów IPC oczekuje na sygnał z klawiatury Ctrl+C, po którym usuwa z pamięci komputera wcześniej zainstalowane mechanizmy IPC i kończy pracę. Pozostałe procesy mają zablokowany sygnał Ctrl+C i Ctrl+Z, a po stwierdzeniu braku połączenia również kończą pracę.
Program zajmuje się dodatkowo wizualizacją działania aplikacji współbieżnej (czyli serwera jak też klientów). Pracuje on
w trybie tekstowym. Proces ten wyświetla na ekranie znaki ASCII w miejscach określonych przez klientów. Format danych dla komunikatu tego procesu ma następującą strukturę:
```
char z1, kol1, x1, y1, z2, kol2, x2, y2
```
Powoduje on wyświetlenie dwóch znaków: z1 w kolorze kol1 na współrzędnych x i y. W razie dowolnej błędnej operacji kończy działanie. W programie konieczne jest synchronizacja dostępu do monitora.

## 2. Proces klienta

Przykładowy sposób powołania procesu klienta "ppp"
```
ppp A 30 1 2 &
```
gdzie:
* [A] - jeden znak ASCII 'A..Z' lub 'a..z' nazwa klienta
* [30] - czas opóźnienia startu procesu [0..40] * 1 sekundę
* [1] - skąd zaczyna się proces klienta
* [2] - czas przemieszczania się (po kratce) [1..30] * 0,2 sekundy
Do wyliczenia czasu używamy funkcję sleep() i usleep().

## 3. Proces drugiego klienta

Proces ten wykonuje specyficzną czynność po dojściu licznika dekrementującego (semafora) do zera. Kiedy muchy (klienci) zjedzą cały dżem ze słoika (13 pól), następuje zamknięcie słoika i zakończenie działania serwera. Muchy mogą zostać zjedzone przez pająki (wątki), które poruszają się po osi Y.

System powinien być tworzony etapami:
* Wizualizacja początkowa aplikacji (serwer).
* Tworzenie i usuwanie kolejek komunikatów (serwer) i przesłanie danych z jednego klienta, wizualizacja klienta na serwerze.
* Wielu klientów łączy się z serwerem i wizualizuje efekty działania.
* Wykorzystywanie pamięci współdzielonej przez klientów do sprawdzania wolnego miejsca na ekranie.
* Utworzenie sekcji krytycznych, aby klienci nie zajmowali w tym czasie tej samej przestrzeni na ekranie.
* Wykorzystanie semafora jako licznika dekrementującego (po dojściu do zera odblokowuje się pewne zadanie zablokowane operacją zero).
* Wykorzystanie wątków - serwer tworzy i wizualizuje wątki na ekranie - wyświetlanie (dostęp do monitora) powinno być zsynchronizowane z semaforem.

## 4. Wymagania

Do prawidłowego działania, wymagane są programy: gcc, make i ncurses. Całaść testowana na Ubuntu:
```
apt-get install gcc make libncurses5-dev libncursesw5-dev
```
## 5. Sposób użycia

Po instalacji niezbędnego oprogramowania, należy skompilować pliki. W tym celu udajemy się do katalogu.
Należy wpisać jedną komendę:
```
make
```
Pliki:
* skrypt
* reset

należy dać prawo do uruchamiania się.
```
chmod +x skrypt reset
```
Skrypt zawiera 20 klientów, którzy pojawią się na ekranie. Reset czyści pozostałości po programie, kiedy zostanie wyłączony
w niedozwolony sposób.

Serwer uruchamiamy poleceniem:
```
./serwer
```
W osobnym terminalu należy wywołać skrypt, w celu zilustrowaniu zadania.
```
./skrypt
```
## 6. Wizualizacja działania programu

Zachowanie działania programu, przedstawia poniższy gif.

![Program Muchy](https://media.giphy.com/media/m9R7vSW88ahJwGXstD/giphy.gif)

## 7. TODO

Można przepisać kod i wykorzystać funkcje.
