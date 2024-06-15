#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define PERM 0660
#define KLUCZ_KOM (key_t)1231
#define KLUCZ_SEM (key_t)1232
#define KLUCZ_MEM (key_t)0x10

// --- ZMIENNE GLOBALNE ---
	
int id_sem, id_mem, kom, kon, *adres;
struct m_komunikat msg;
pthread_t watek[3], sloik;

// --- STRUKTURY ---

struct m_komunikat
{
    long typ;
    char znak1, znak2;
    int pozycjaY1, pozycjaX1, kolor1, pozycjaY2, pozycjaX2, kolor2;
};

typedef struct _my_data
{
    short sem_num;
    short sem_op;
    short sem_flg;
} sembuf;

union semun
{
    int val;
    struct semi_ds *buf;
    unsigned short *array;
};

// --- FUNKCJE ---

int usun_sem(int id) // FUNKCJA USUWA ZESTAW SEMAFOROW
{
    union semun unia_sem;

    if (semctl(id, 0, IPC_RMID, unia_sem) == -1)
    {
        perror("\nS. SEM_RMID error ");
        return(-1);
    }

    return(0);
}

int ustaw_sem(int id, int nr, int wart) // FUNKCJA USTAWIAJACA WARTOSC SEMAFORA
{
    union semun unia_sem;
    unia_sem.val = wart;

    if ((semctl(id, nr, SETVAL, unia_sem)) == -1)
    {
        perror("\nS. SETVAL error ");
        return(-1);
    }

    return(0);
}

int sem_oper(int id, short nr, short wart) // FUNKCJA WYKONUJE OPERACJE NA SEMAFORZE
{
    struct sembuf buf_sem;
    buf_sem.sem_num = nr;
    buf_sem.sem_op = wart;
    buf_sem.sem_flg = 0;

    if ((semop(id, &buf_sem, 1)) == -1)
    {
        perror("\nK. SEMOP error ");
        return(-1);
    }

    return(0);
}

void koniec()
{
    msgctl(kom, IPC_RMID, 0);
    if (shmctl(id_mem, IPC_RMID, NULL) == -1)
        perror("\nS. MEM_RMID error ");
    usun_sem(id_sem);
    curs_set(1);
    clear();
    refresh();
    endwin();

    printf("[SERWER]: Wcisnieto Ctrl+C\n");
    exit(0);
}

void debug(int czas)
{
	for(int i = 0; i <= 23; i++) // WYSWIETLANIE PAMIECI WSPOLDZIELONEJ
    {
       for(int j = 0; j <= 79; j++)
          printf("%d", *(adres + i * 80 + j));
       printf("\n");
    }

    sleep(czas);
}

// --- WATKI ---

void *pajak()
{
    // LOSUJEMY POZYCJE PAJAKA
    int pozycjaY = rand() % 3 + 1; // LOSUJE LICZBE Z PRZEDZIALU [1, 3]
    int pozycjaX = rand() % 78 + 1; // LOSUJE LICZBE Z PRZEDZIALU [1, 78]

    if (*(adres + pozycjaY * 80 + pozycjaX) == 0)
    {
        int czyWolne = 0, poprzedniaPozycjaY, przesuniecieY;

        while (true)
        {
            poprzedniaPozycjaY = pozycjaY; // KOPIA WSPOLRZEDNYCH
            //poprzedniaPozycjaX = pozycjaX;

            przesuniecieY = rand() % 3 - 1; // LOSUJE LICZBE Z PRZEDZIALU [-1, 1]
            //przesuniecieX = rand() % 3 - 1;

            pozycjaY += przesuniecieY; // UWZGLEDNIAM PRZENIESIENIE
            //pozycjaX += przesuniecieX;

            int wartosc = *(adres + pozycjaY * 80 + pozycjaX);

            switch (wartosc)
            {
               case 0: // WOLNE MIEJSCE
                   czyWolne = 1;
                   break;
               default: // INNY PRZYPADEK
                   czyWolne = 0;
                   break;
            }

            if (czyWolne)
            {
                sem_oper(id_sem, 0, -1);
                   *(adres + pozycjaY * 80 + pozycjaX) = 4;
                   *(adres + poprzedniaPozycjaY * 80 + pozycjaX) = 0;
                sem_oper(id_sem, 0, 1);

                sem_oper(id_sem, 1, -1);
                   // USUWANIE
                   attron(COLOR_PAIR(8) | A_BOLD);
                   mvprintw(poprzedniaPozycjaY, pozycjaX, " ");
                   attroff(COLOR_PAIR(8) | A_BOLD);
                   // RYSOWANIE
                   attron(COLOR_PAIR(6) | A_BOLD);
                   mvprintw(pozycjaY, pozycjaX, "p");
                   attroff(COLOR_PAIR(6) | A_BOLD);
                   refresh();
                sem_oper(id_sem, 1, 1);
            }

            else
            {
                pozycjaY = poprzedniaPozycjaY;
                //pozycjaX = poprzedniaPozycjaX;
            }

            usleep(250000);
        }
    }
}

void *zamkniecieSloika()
{
    sem_oper(id_sem, 2, 0);
    sem_oper(id_sem, 0, -1);
       for (int i = 34; i <= 45; i++)
           *(adres + 14 * 80 + i) = 1;
       for (int i = 33; i <= 46; i++)
           *(adres + 15 * 80 + i) = 1;
       for (int i = 33; i <= 46; i++)
           *(adres + 16 * 80 + i) = 1;
    sem_oper(id_sem, 0, 1);
    
    for (int i = 0; i < 3; i++)
        pthread_cancel(watek[i]);

    sem_oper(id_sem, 1, -1);
       attron(COLOR_PAIR(4) | A_BOLD);
       mvprintw(14, 34, "____________"); // RYSOWANIE GORY SLOIKA
       mvprintw(15, 33, "(____________)");
       mvprintw(16, 33, "/            \\");
       attroff(COLOR_PAIR(4) | A_BOLD);
    sem_oper(id_sem, 1, 1);

    // NAPIS NA RAMCE
    sem_oper(id_sem, 1, -1);
       for (int i = 1; i <= 78; i++)
           mvprintw(22, i, " ");
       attron(A_BOLD);
       mvprintw(22, 37, "KONIEC"); // WYSRODKOWANY TEKST
       attroff(A_BOLD);
       refresh();
    sem_oper(id_sem, 1, 1);

    msg.typ = 1; // ZAKONCZENIE KOMUNIKACJI
    if ((msgsnd(kom, &msg, sizeof(msg), 0)) == -1) // WYSLANIE KOMUNIKATU
    {
        perror("\nK.Awaria serwera ");
        exit(-1);
    }
}

// --- MAIN ---

int main(int argc, char *argv[])
{
    signal(SIGINT, koniec);

    if ((id_sem = semget(KLUCZ_SEM, 3, PERM | IPC_CREAT)) == -1)
    {
        perror("\nS. SEM_CREAT error ");
        koniec();
    }

    if ((id_mem = shmget(KLUCZ_MEM, 1920 * sizeof(int), PERM | IPC_CREAT)) == -1) // USTAWIAM WARTOSC PAMIECI 24 * 80 = 1920
    {
        perror("\nS. MEM_CREAT error ");
        koniec();
    }

    if ((adres = (int *)shmat(id_mem, 0, 0)) == ((int *)-1))
    {
        perror("\nS. MEM_SHMAT error ");
        koniec();
    }

    if ((kom = msgget(KLUCZ_KOM, PERM | IPC_CREAT)) == -1)
    {
        perror("\nS.msgget error ");
        koniec();
    }

    ustaw_sem(id_sem, 0, 1); // PAMIEC WSPOLDZIELONA
    ustaw_sem(id_sem, 1, 1); // DRUKOWANIE NA EKRANIE
    ustaw_sem(id_sem, 2, 12); // LICZNIK DEKREMENTUJACY

    /*

    LEGENDA:
       0 - TEREN WOLNY
       1 - ELEMENT ZABLOKOWANY
       2 - MUCHA
       3 - DZEM
       4 - PAJAK

    */

    sem_oper(id_sem, 0, -1);
       for (int i = 0; i <= 23; i++) // CALY OBSZAR MAPY NA 1
       {
           for (int j = 0; j <= 79; j++)
               *(adres + i * 80 + j) = 1;
       }

       for (int i = 1; i <= 20; i++) // OBSZAR DO PORUSZANIA SIE MUCH NA 0
       {
           for (int j = 1; j <= 78; j++)
               *(adres + i * 80 + j) = 0;
       }

       for (int i = 32; i <= 47; i++) // DOL SLOIKA NA 1
           *(adres + 21 * 80 + i) = 1;

       for (int i = 17; i <= 20; i++) // LEWA STRONA SLOIKA NA 1
           *(adres + i * 80 + 33) = 1;

       for (int i = 17; i <= 20; i++) // PRAWA STRONA SLOIKA NA 1
           *(adres + i * 80 + 46) = 1;

       for (int i = 34; i <= 45; i++) // OBSZAR DZEMU NA 3
           *(adres + 20 * 80 + i) = 3;

    sem_oper(id_sem, 0, 1);

	//debug(5);

// --- RYSOWANIE PLANSZY ---

    initscr();
    curs_set(0); // WYLACZENIE KURSORA
    start_color();
    clear();

    // KOLORY
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_GREEN, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(8, COLOR_BLACK, COLOR_BLACK);

    // OKNO
    WINDOW *okno = newwin(24, 80, 0, 0);
    wborder(okno, '#', '#', '#', '#', '+', '+', '+', '+');
    refresh();
    wrefresh(okno);

    // SLOIK
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(17, 33, "[            ]"); // WYSRODKOWANY SLOIK
    mvprintw(18, 33, "[            ]");
    mvprintw(19, 33, "[            ]");
    mvprintw(20, 33, "[            ]");
    attroff(COLOR_PAIR(4) | A_BOLD);

    // ZAWARTOSC SLOIKA
    attron(COLOR_PAIR(7) | A_BOLD);
    mvprintw(20, 34, "            ");
    attroff(COLOR_PAIR(7) | A_BOLD);

    // RAMKA
    for (int i = 1; i <= 78; i++)
        mvprintw(21, i, "#");
    mvprintw(21, 0, "+");
    mvprintw(21, 79, "+");
    attron(A_BOLD);
    mvprintw(22, 24, "MUCHY WYJADAJACE DZEM ZE SLOIKA!"); // WYSRODKOWANY TEKST
    attroff(A_BOLD);
    refresh();

// --- TWORZENIE WATKOW ---

    srand(time(NULL));
    pthread_create(&watek[0], NULL, pajak, NULL);
    pthread_create(&watek[1], NULL, pajak, NULL);
    pthread_create(&watek[2], NULL, pajak, NULL);
    pthread_create(&sloik, NULL, zamkniecieSloika, NULL);

// --- KONIEC RYSOWANIA PLANSZY ---

    kon = 1;
    while (kon)
    {
        if ((msgrcv(kom, &msg, sizeof(msg), 0, 0)) == -1)
        {
            perror("\nS.msgrcv error ");
            kon = 0;
        }

        else
        {
            switch (msg.typ)
            {
               case 1:
                   kon = 0;
                   break;

               case 2:
                   sem_oper(id_sem, 1, -1);
                      // USUWANIE
                      attron(COLOR_PAIR(msg.kolor2) | A_BOLD);
                      mvprintw(msg.pozycjaY2, msg.pozycjaX2, "%c", msg.znak2);
                      attroff(COLOR_PAIR(msg.kolor2) | A_BOLD);
                      // RYSOWANIE
                      attron(COLOR_PAIR(msg.kolor1) | A_BOLD);
                      mvprintw(msg.pozycjaY1, msg.pozycjaX1, "%c", msg.znak1);
                      attroff(COLOR_PAIR(msg.kolor1) | A_BOLD);
                      refresh();
                   sem_oper(id_sem, 1, 1);
                   break;
            }
        }
    }

    sleep(15);
    msgctl(kom, IPC_RMID, 0);
    pthread_cancel(sloik);
    curs_set(1);
    clear();
    refresh();
    endwin();

    printf("[SERWER]: Koniec pracy!\n");
    return (0);
}
