
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 30
#define STATES 100
#define SIZE 128
#define ITEMS 1000

typedef struct
{
    int p, dot;
    char look;
} Item;

typedef struct
{
    Item item[ITEMS];
    int count;
} State;

char L[MAX], R[MAX][MAX];
char NT[SIZE] = "", T[SIZE] = "";
char FIRST[SIZE][SIZE];

State states[STATES];
int trans[STATES][SIZE];
int n, total = 0;

int isNT(char c)
{
    return isupper((unsigned char)c);
}

int has(char s[], char c)
{
    return strchr(s, c) != NULL;
}

void add(char s[], char c)
{
    if (!has(s, c))
    {
        int len = strlen(s);
        s[len] = c;
        s[len + 1] = '\0';
    }
}

void first(char s[], char result[])
{
    result[0] = '\0';

    if (s[0] == '\0')
    {
        add(result, '#');
        return;
    }

    int nullable = 1;

    for (int i = 0; s[i]; i++)
    {
        char c = s[i];

        if (c == '#')
            continue;

        if (!isNT(c))
        {
            add(result, c);
            nullable = 0;
            break;
        }

        for (int j = 0; FIRST[(int)c][j]; j++)
        {
            if (FIRST[(int)c][j] != '#')
                add(result, FIRST[(int)c][j]);
        }

        if (!has(FIRST[(int)c], '#'))
        {
            nullable = 0;
            break;
        }
    }

    if (nullable)
        add(result, '#');
}

void findFirst()
{
    int change;

    do
    {
        change = 0;

        for (int i = 1; i <= n; i++)
        {
            char temp[SIZE];
            first(R[i], temp);

            for (int j = 0; temp[j]; j++)
            {
                if (!has(FIRST[(int)L[i]], temp[j]))
                {
                    add(FIRST[(int)L[i]], temp[j]);
                    change = 1;
                }
            }
        }
    } while (change);
}

void addItem(State *s, int p, int d, char look)
{
    for (int i = 0; i < s->count; i++)
    {
        if (s->item[i].p == p &&
            s->item[i].dot == d &&
            s->item[i].look == look)
            return;
    }

    if (s->count < ITEMS)
    {
        s->item[s->count++] = (Item){p, d, look};
    }
}

void closure(State *s)
{
    int change;

    do
    {
        change = 0;

        for (int i = 0; i < s->count; i++)
        {
            int p = s->item[i].p;
            int d = s->item[i].dot;
            char B = R[p][d];

            if (!isNT(B))
                continue;

            char seq[MAX];
            strcpy(seq, R[p] + d + 1);

            int len = strlen(seq);
            seq[len] = s->item[i].look;
            seq[len + 1] = '\0';

            char result[SIZE];
            first(seq, result);

            for (int j = 1; j <= n; j++)
            {
                if (L[j] == B)
                {
                    for (int k = 0; result[k]; k++)
                    {
                        if (result[k] != '#')
                        {
                            int old = s->count;
                            addItem(s, j, 0, result[k]);

                            if (s->count > old)
                                change = 1;
                        }
                    }
                }
            }
        }
    } while (change);
}

void goTo(State *a, char c, State *b)
{
    b->count = 0;

    for (int i = 0; i < a->count; i++)
    {
        int p = a->item[i].p;
        int d = a->item[i].dot;

        if (R[p][d] == c)
            addItem(b, p, d + 1, a->item[i].look);
    }

    closure(b);
}

int same(State *a, State *b)
{
    if (a->count != b->count)
        return 0;

    for (int i = 0; i < a->count; i++)
    {
        int found = 0;

        for (int j = 0; j < b->count; j++)
        {
            if (a->item[i].p == b->item[j].p &&
                a->item[i].dot == b->item[j].dot &&
                a->item[i].look == b->item[j].look)
            {
                found = 1;
                break;
            }
        }

        if (!found)
            return 0;
    }

    return 1;
}

int findState(State *s)
{
    for (int i = 0; i < total; i++)
    {
        if (same(&states[i], s))
            return i;
    }

    return -1;
}

void buildStates()
{
    total = 1;
    states[0].count = 0;

    addItem(&states[0], 0, 0, '$');
    closure(&states[0]);

    for (int i = 0; i < STATES; i++)
        for (int j = 0; j < SIZE; j++)
            trans[i][j] = -1;

    for (int i = 0; i < total; i++)
    {
        char symbols[SIZE] = "";

        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].item[j].p;
            int d = states[i].item[j].dot;
            char c = R[p][d];

            if (c != '\0' && c != '#')
                add(symbols, c);
        }

        for (int j = 0; symbols[j]; j++)
        {
            char c = symbols[j];

            State temp;
            goTo(&states[i], c, &temp);

            int id = findState(&temp);

            if (id == -1)
            {
                if (total >= STATES)
                    return;

                id = total;
                states[total++] = temp;
            }

            trans[i][(int)c] = id;
        }
    }
}

int checkCLR()
{
    int actions[SIZE];

    for (int i = 0; i < total; i++)
    {
        memset(actions, 0, sizeof(actions));

        for (int j = 0; j < strlen(T); j++)
        {
            char c = T[j];

            if (trans[i][(int)c] != -1)
                actions[(int)c]++;
        }

        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].item[j].p;
            int d = states[i].item[j].dot;
            char look = states[i].item[j].look;

            if (R[p][d] == '\0')
            {
                if (p == 0 && look == '$')
                    actions['$']++;
                else if (p != 0)
                    actions[(int)look]++;
            }
        }

        for (int j = 0; j < SIZE; j++)
        {
            if (actions[j] > 1)
                return 0;
        }
    }

    return 1;
}

int main()
{
    printf("Enter number of productions: ");
    scanf("%d", &n);

    if (n <= 0 || n >= MAX)
    {
        printf("Invalid number of productions.\n");
        return 0;
    }

    printf("\nEnter productions (example S=CC):\n");

    for (int i = 1; i <= n; i++)
    {
        char line[100];
        scanf("%99s", line);

        if (strlen(line) < 3 || line[1] != '=')
        {
            printf("Invalid production format.\n");
            return 0;
        }

        L[i] = line[0];

        if (strcmp(line + 2, "#") == 0)
            strcpy(R[i], "");
        else
            strcpy(R[i], line + 2);

        add(NT, L[i]);
    }

    char aug = 'Z';

    while (has(NT, aug))
        aug--;

    L[0] = aug;
    strcpy(R[0], (char[]){L[1], '\0'});

    for (int i = 1; i <= n; i++)
    {
        for (int j = 0; R[i][j]; j++)
        {
            char c = R[i][j];

            if (!isNT(c))
                add(T, c);
        }
    }

    add(T, '$');

    findFirst();
    buildStates();

    if (checkCLR())
        printf("\nGrammar is in CLR(1).\n");
    else
        printf("\nGrammar is NOT in CLR(1).\n");

    return 0;
}
