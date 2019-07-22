#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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

// --- MAIN ---

int main(int argc, char *argv[])
{
    struct m_komunikat msg;

    if (argc < 4) // DLA 3 ARGUMENTOW, POZYCJAY, POZYCJAX, KOLOR
    {
        printf("Program wymaga do dzialania trzech argumentow:\n\n1) PozycjaY[1, 23] - wiersz\n2) PozycjaX[1, 78] - kolumna\n");
        printf("3) Kolor[1, 5] - kolor\n[1 - czerwony, 2 - niebieski, 3 - zolty, 4 - bialy, 5 - cyjan]\n\n");
        printf("Przykladowe uzycie: ./client 4 2 3\nW 4 wersie i 2 kolumnie, pojawi sie zolta mucha.\n");
        exit(0);
    }

    if ((kom = msgget(KLUCZ_KOM, 0)) == -1)
    {
        perror("\nK.msgget error ");
        exit(EXIT_FAILURE);
    }

    if ((id_sem = semget(KLUCZ_SEM, 3, 0)) == -1)
    {
        perror("\nK. semget error ");
        exit(EXIT_FAILURE);
    }

    if ((id_mem = shmget(KLUCZ_MEM, 1920 * sizeof(int), 0)) == -1) // 7680 BAJTOW
    {
        perror("\nS. memget error ");
        exit(EXIT_FAILURE);
    }

    if ((adres = (int *)shmat(id_mem, 0, 0)) == ((int *)-1))
    {
        perror("\nS. MEM_SHMAT error ");
        exit(EXIT_FAILURE);
    }

// --- WSZYSTKO JEST W PORZADKU ---

    srand(getpid());
    msg.pozycjaY1 = atoi(argv[1]); // POZYCJA Y POBIERANA Z LINII KOMEND
    msg.pozycjaX1 = atoi(argv[2]); // POZYCJA X POBIERANA Z LINII KOMEND
    msg.kolor1 = atoi(argv[3]); // KOLOR POBIERANY Z LINII KOMEND
    char status = 'n'; // INFORMACJA DLA MNIE, MUCHA ZJADLA DZEM, POLEGLA NA PAJAKU CZY SIE NIE POJAWILA

    if (*(adres + msg.pozycjaY1 * 80 + msg.pozycjaX1) == 0) // SPRAWDZAM CZY MOZE POJAWIC SIE MUCHA
    {
        msg.typ = 2;
        msg.znak1 = 'm';
        msg.znak2 = ' ';
        msg.pozycjaY2 = 0;
        msg.pozycjaX2 = 0;
        msg.kolor2 = 8;

        if ((msgsnd(kom, &msg, sizeof(msg), 0)) == -1) // RYSUJE MUCHE NA EKRANIE
        {
            perror("\nK.Awaria serwera ");
            exit(-1);
        }

        sem_oper(id_sem, 0, -1);
           *(adres + msg.pozycjaY1 * 80 + msg.pozycjaX1) = 2;
        sem_oper(id_sem, 0, 1);

        int pozycjaPoprzedniaY, pozycjaPoprzedniaX;
        int czyWolne = 0; // 1 - WOLNE, 0 - ZAJETE
        int nieZnaleziono = 1; // 1 - NIEZNALEZIONO, 0 - ZNALEZIONO
        int czyUsunac = 1; // 1 - NIE USUWAC, 0 TAK USUNAC

        while (nieZnaleziono && czyUsunac)
        {
            int liczbaLosowaY = rand() % 3 - 1; // LOSUJE LICZBE Z PRZEDZIALU [-1, 1]
            int liczbaLosowaX = rand() % 3 - 1;

            pozycjaPoprzedniaY = msg.pozycjaY1; // KOPIA WSPOLRZEDNYCH
            pozycjaPoprzedniaX = msg.pozycjaX1;

            msg.pozycjaY1 += liczbaLosowaY; // UWZGLEDNIAM PRZENIESIENIE
            msg.pozycjaX1 += liczbaLosowaX;

            int wartosc = *(adres + msg.pozycjaY1 * 80 + msg.pozycjaX1);

            switch (wartosc)
            {
               case 0: // WOLNE MIEJSCE
                   czyWolne = 1;
                   break;
               case 1: // PRZESZKODA
                   czyWolne = 0;
                   break;
               case 2: // MUCHA
                   czyWolne = 0;
                   break;
               case 3: // DZEM
                   czyWolne = 1;
                   nieZnaleziono = 0;
                   break;
               case 4: // PAJAK
                   czyWolne = 1;
                   czyUsunac = 0;
                   break;
            }

            if (czyWolne)
            {
                sem_oper(id_sem, 0, -1);
                   *(adres + msg.pozycjaY1 * 80 + msg.pozycjaX1) = 2; // BLOKUJE NOWE MIEJSCE
                   *(adres + pozycjaPoprzedniaY * 80 + pozycjaPoprzedniaX) = 0; // ZWALNIAM POPRZEDNIA POZYCJE
                   msg.pozycjaY2 = pozycjaPoprzedniaY;
                   msg.pozycjaX2 = pozycjaPoprzedniaX;

                   if (!nieZnaleziono) // MUCHA WCHODZI W DZEM
                   {
                       *(adres + msg.pozycjaY1 * 80 + msg.pozycjaX1) = 0; // ABY MUCHA MOGLA WLECIEC GLEBIEJ
                       msg.znak1 = ' '; // USUWAM MUCHE Z EKRANU
                       status = 'd';
                       sem_oper(id_sem, 2, -1);
                   }

                   if (!czyUsunac) // MUCHA WCHODZI W PAJAKA
                   {
                       *(adres + msg.pozycjaY1 * 80 + msg.pozycjaX1) = 4;
                       msg.kolor1 = 6; // KOLOR ZIELONY
                       msg.znak1 = 'p';
                       status = 'p';
                   }
                sem_oper(id_sem, 0, 1);

                if ((msgsnd(kom, &msg, sizeof(msg), 0)) == -1) // WYSLANIE KOMUNIKATU
                {
                    perror("\nK.Awaria serwera ");
                    exit(EXIT_FAILURE);
                }
            }

            else // WYLOSOWANE MIEJSCE JEST ZAJETE
            {
                msg.pozycjaY1 = pozycjaPoprzedniaY; // PRZYWRACAMY KOPIE WSPOLRZEDNYCH
                msg.pozycjaX1 = pozycjaPoprzedniaX;
            }

            usleep(250000);
        }
    }

    // JEZELI MUCHA NIE MOZE POJAWIC SIE NA MAPIE, TO CLIENT KONCZY PRACE

    printf("[CLIENT]: Koniec pracy! %c\n", status);
    exit(0);
}
