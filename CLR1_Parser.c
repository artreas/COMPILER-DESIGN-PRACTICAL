
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX 30
#define STATES 100
#define SIZE 128
#define ITEMS 2000

typedef struct
{
    int prod, dot;
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
int totalStates = 0, n;
char ACTION[STATES][SIZE][16];
int GOTO[STATES][SIZE];

char aug;

int isNT(char c)
{
    return isupper((unsigned char)c);
}

int contains(char s[], char c)
{
    return strchr(s, c) != NULL;
}

void add(char s[], char c)
{
    if (!contains(s, c))
    {
        int len = strlen(s);
        s[len] = c;
        s[len + 1] = '\0';
    }
}

void firstSeq(char s[], char result[])
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

        if (!contains(FIRST[(int)c], '#'))
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
    int changed;

    do
    {
        changed = 0;

        for (int i = 1; i <= n; i++)
        {
            char temp[SIZE];
            firstSeq(R[i], temp);

            for (int j = 0; temp[j]; j++)
            {
                if (!contains(FIRST[(int)L[i]], temp[j]))
                {
                    add(FIRST[(int)L[i]], temp[j]);
                    changed = 1;
                }
            }
        }
    } while (changed);
}

void addItem(State *s, int p, int d, char look)
{
    for (int i = 0; i < s->count; i++)
    {
        if (s->item[i].prod == p &&
            s->item[i].dot == d &&
            s->item[i].look == look)
            return;
    }

    if (s->count < ITEMS)
    {
        s->item[s->count].prod = p;
        s->item[s->count].dot = d;
        s->item[s->count].look = look;
        s->count++;
    }
}

void closure(State *s)
{
    int changed;

    do
    {
        changed = 0;

        for (int i = 0; i < s->count; i++)
        {
            int p = s->item[i].prod;
            int d = s->item[i].dot;
            char look = s->item[i].look;

            char B = R[p][d];

            if (B == '\0' || B == '#')
                continue;

            if (isNT(B))
            {
                char sequence[MAX];

                strcpy(sequence, R[p] + d + 1);

                int len = strlen(sequence);
                sequence[len] = look;
                sequence[len + 1] = '\0';

                char result[SIZE];
                firstSeq(sequence, result);

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
                                    changed = 1;
                            }
                        }
                    }
                }
            }
        }
    } while (changed);
}

void goTo(State *source, char symbol, State *result)
{
    result->count = 0;

    for (int i = 0; i < source->count; i++)
    {
        int p = source->item[i].prod;
        int d = source->item[i].dot;
        char look = source->item[i].look;

        if (R[p][d] == symbol)
            addItem(result, p, d + 1, look);
    }

    closure(result);
}

int sameState(State *a, State *b)
{
    if (a->count != b->count)
        return 0;

    for (int i = 0; i < a->count; i++)
    {
        int found = 0;

        for (int j = 0; j < b->count; j++)
        {
            if (a->item[i].prod == b->item[j].prod &&
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
    for (int i = 0; i < totalStates; i++)
    {
        if (sameState(&states[i], s))
            return i;
    }

    return -1;
}

void buildStates()
{
    totalStates = 1;
    states[0].count = 0;

    addItem(&states[0], 0, 0, '$');
    closure(&states[0]);

    for (int i = 0; i < STATES; i++)
        for (int j = 0; j < SIZE; j++)
            trans[i][j] = -1;

    for (int i = 0; i < totalStates; i++)
    {
        char symbols[SIZE] = "";

        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].item[j].prod;
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
                if (totalStates >= STATES)
                {
                    printf("Too many states.\n");
                    exit(1);
                }

                id = totalStates;
                states[totalStates++] = temp;
            }

            trans[i][(int)c] = id;
        }
    }
}

int addAction(int state, char symbol, char value[])
{
    char *cell = ACTION[state][(int)symbol];

    if (cell[0] == '\0')
    {
        strcpy(cell, value);
        return 1;
    }

    if (strcmp(cell, value) != 0)
        return 0;

    return 1;
}

int buildTable()
{
    int valid = 1;

    for (int i = 0; i < STATES; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            ACTION[i][j][0] = '\0';
            GOTO[i][j] = -1;
        }
    }

    for (int i = 0; i < totalStates; i++)
    {

        for (int j = 0; T[j]; j++)
        {
            char c = T[j];
            int next = trans[i][(int)c];

            if (next != -1)
            {
                char value[16];
                sprintf(value, "s%d", next);

                if (!addAction(i, c, value))
                    valid = 0;
            }
        }

        for (int j = 0; NT[j]; j++)
        {
            char c = NT[j];
            GOTO[i][(int)c] = trans[i][(int)c];
        }

        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].item[j].prod;
            int d = states[i].item[j].dot;
            char look = states[i].item[j].look;

            if (R[p][d] != '\0')
                continue;

            if (p == 0 && look == '$')
            {
                if (!addAction(i, '$', "acc"))
                    valid = 0;
            }
            else if (p != 0)
            {
                char value[16];
                sprintf(value, "r%d", p);

                if (!addAction(i, look, value))
                    valid = 0;
            }
        }
    }

    return valid;
}

void displayTable()
{
    printf("\nCLR(1) PARSING TABLE\n\n");

    printf("%-10s", "STATE");

    for (int i = 0; T[i]; i++)
        printf("%-10c", T[i]);

    printf("| ");

    for (int i = 0; NT[i]; i++)
        printf("%-10c", NT[i]);

    printf("\n");

    for (int i = 0; i < totalStates; i++)
    {
        printf("%-10d", i);

        for (int j = 0; T[j]; j++)
            printf("%-10s", ACTION[i][(int)T[j]]);

        printf("| ");

        for (int j = 0; NT[j]; j++)
        {
            int x = GOTO[i][(int)NT[j]];

            if (x == -1)
                printf("%-10s", "");
            else
                printf("%-10d", x);
        }

        printf("\n");
    }
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

        scanf("%s", line);

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

    aug = 'Z';

    while (contains(NT, aug))
        aug--;

    L[0] = aug;
    R[0][0] = L[1];
    R[0][1] = '\0';

    for (int i = 1; i <= n; i++)
    {
        for (int j = 0; R[i][j]; j++)
        {
            char c = R[i][j];

            if (c != '#' && !isNT(c))
                add(T, c);
        }
    }

    add(T, '$');

    findFirst();
    buildStates();

    if (!buildTable())
    {
        printf("\nGrammar is NOT CLR(1).\n");
        printf("CLR(1) parsing table contains conflicts.\n");
        return 0;
    }

    printf("\nGrammar is CLR(1).\n");

    displayTable();

    return 0;
}
