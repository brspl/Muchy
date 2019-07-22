# Muchy wyjadające dżem ze słoika

---

## Programowanie systemowe

Wykorzystując mechanizmy systemu GNU/Linux zaprojektować i uruchomić zadany przez prowadzącego zajęcia system w wersji lokalnej.

Zasady działania systemu:

* Wszystkie procesy systemu uruchamiamy na jednym komputerze. Nie korzystamy z funkcji fork oraz potoków.
* Nie wprowadzamy danych do procesów z plików.
* Procesy systemu wyświetlają następujące informacje np.: "N.koniec pracy".
* Przygotować skrypty (dla klientów) pokazujące prawidłowe działanie systemu.

## 1. Proces serwera

Tylko ten proces ma dostęp do klawiatury. Proces ten po zainstalowaniu wszystkich wspólnych mechanizmów IPC oczekuje na sygnał z klawiatury Ctrl+C, po którym usuwa z pamięci komputera wcześniej zainstalowane mechanizmy IPC i kończy pracę. Pozostałe procesy mają zablokowany sygnał Ctrl+c i Ctrl+z, a po stwierdzeniu braku połączenia również kończą pracę.
Program zajmuje się dodatkowo wizualizacją działania aplikacji współbieżnej (czyli serwera jak też klientów). Pracuje on w trybie tekstowym. Proces ten wyświetla na ekranie znaki ASCII w miejscach określonych przez klientów. Format danych dla komunikatu tego procesu ma następującą strukturę:
```
char z1, kol1, x1, y1, z2, kol2, x2, y2
```
Powoduje on wyświetlenie dwóch znaków: z1 w kolorze kol1 na współrzędnych x i y. W razie dowolnej błędnej operacji kończy działanie. W programie konieczne jest synchronizacja dostępu do monitora.
