#include <stdio.h>

#define DTABSIZE 60

struct tsd_ent{
    int ts_quantum;
    int ts_tqexp;
    int ts_slpret;
};


struct tsd_ent tsd_tab[DTABSIZE] = {
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50}, // 10

{160, 0, 51},
{160, 1, 51},
{160, 2, 51},
{160, 3, 51},
{160, 4, 51},
{160, 5, 51},
{160, 6, 51},
{160, 7, 51},
{160, 8, 51},
{160, 9, 51}, // 20

{120, 10, 52},
{120, 11, 52},
{120, 12, 52},
{120, 13, 52},
{120, 14, 52},
{120, 15, 52},
{120, 16, 52},
{120, 17, 52},
{120, 18, 52},
{120, 19, 52}, // 30

{80, 20, 53},
{80, 21, 53},
{80, 22, 53},
{80, 23, 53},
{80, 24, 53},
{80, 25, 54},
{80, 26, 54},
{80, 27, 54},
{80, 28, 54},
{80, 29, 54}, // 40

{40, 30, 55},
{40, 31, 55},
{40, 32, 55},
{40, 33, 55},
{40, 34, 55},
{40, 35, 56},
{40, 36, 57},
{40, 37, 58},
{40, 38, 58},
{40, 39, 58}, // 50

{40, 40, 58},
{40, 41, 58},
{40, 42, 58},
{40, 43, 58},
{40, 44, 58},
{40, 45, 58},
{40, 46, 58},
{40, 47, 58},
{40, 48, 58},
{20, 49, 59}
};

int main(){
    int LEVEL;
    printf("ts_quantum\tts_tqexp\tts_slpret\n");
    for(LEVEL=0; LEVEL < DTABSIZE; LEVEL++){
        printf("%d\t\t%d\t\t%d\n", tsd_tab[LEVEL].ts_quantum, tsd_tab[LEVEL].ts_tqexp, tsd_tab[LEVEL].ts_slpret);
    }
    return 0;
}