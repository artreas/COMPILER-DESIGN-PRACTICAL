
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20
#define STATES 100

char L[MAX], R[MAX][MAX], NT[MAX], T[MAX];
char F[128][128], FOLLOW[128][128];
int n, ns = 0, ts = 0, count = 0;

typedef struct
{
    int p, d;
} Item;

typedef struct
{
    Item a[100];
    int size;
} State;

State S[STATES];
int trans[STATES][128];

int add(char *s, char c)
{
    if (!strchr(s, c))
    {
        int l = strlen(s);
        s[l] = c;
        s[l + 1] = '\0';
        return 1;
    }
    return 0;
}

int isNT(char c)
{
    return isupper((unsigned char)c);
}

void first(char *str, char *res)
{
    res[0] = '\0';
    int nullable = 1;

    for (int i = 0; str[i] && nullable; i++)
    {
        char c = str[i];

        if (!isNT(c))
        {
            add(res, c);
            nullable = 0;
        }
        else
        {
            for (int j = 0; F[(int)c][j]; j++)
                if (F[(int)c][j] != '#')
                    add(res, F[(int)c][j]);

            if (!strchr(F[(int)c], '#'))
                nullable = 0;
        }
    }

    if (nullable)
        add(res, '#');
}

void findSets()
{
    int change;

    do
    {
        change = 0;

        for (int i = 1; i <= n; i++)
        {
            char temp[MAX] = "";
            first(R[i], temp);

            for (int j = 0; temp[j]; j++)
                change |= add(F[(int)L[i]], temp[j]);
        }
    } while (change);

    add(FOLLOW[(int)L[1]], '$');

    do
    {
        change = 0;

        for (int i = 1; i <= n; i++)
        {
            for (int j = 0; R[i][j]; j++)
            {
                char c = R[i][j];

                if (!isNT(c))
                    continue;

                char temp[MAX] = "";
                first(R[i] + j + 1, temp);

                for (int k = 0; temp[k]; k++)
                    if (temp[k] != '#')
                        change |= add(FOLLOW[(int)c], temp[k]);

                if (strchr(temp, '#'))
                    for (int k = 0; FOLLOW[(int)L[i]][k]; k++)
                        change |= add(FOLLOW[(int)c],
                                      FOLLOW[(int)L[i]][k]);
            }
        }
    } while (change);
}

void insert(State *s, int p, int d)
{
    for (int i = 0; i < s->size; i++)
        if (s->a[i].p == p && s->a[i].d == d)
            return;

    s->a[s->size].p = p;
    s->a[s->size].d = d;
    s->size++;
}

void closure(State *s)
{
    int change;

    do
    {
        change = 0;

        for (int i = 0; i < s->size; i++)
        {
            int p = s->a[i].p;
            int d = s->a[i].d;

            if (d < strlen(R[p]) && isNT(R[p][d]))
            {
                char c = R[p][d];

                for (int j = 0; j <= n; j++)
                    if (L[j] == c)
                    {
                        int old = s->size;
                        insert(s, j, 0);
                        if (old != s->size)
                            change = 1;
                    }
            }
        }
    } while (change);
}

void goTo(State *s, char c, State *y)
{
    y->size = 0;

    for (int i = 0; i < s->size; i++)
    {
        int p = s->a[i].p;
        int d = s->a[i].d;

        if (R[p][d] == c)
            insert(y, p, d + 1);
    }

    if (y->size)
        closure(y);
}

int equal(State *a, State *b)
{
    if (a->size != b->size)
        return 0;

    for (int i = 0; i < a->size; i++)
    {
        int found = 0;

        for (int j = 0; j < b->size; j++)
            if (a->a[i].p == b->a[j].p &&
                a->a[i].d == b->a[j].d)
                found = 1;

        if (!found)
            return 0;
    }

    return 1;
}

int findState(State *x)
{
    for (int i = 0; i < count; i++)
        if (equal(x, &S[i]))
            return i;

    return -1;
}

int main()
{
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (example S=CC):\n");

    for (int i = 1; i <= n; i++)
    {
        char str[MAX];
        scanf("%19s", str);

        L[i] = str[0];

        if (strcmp(str + 2, "#") == 0)
            R[i][0] = '\0';
        else
            strcpy(R[i], str + 2);

        add(NT, L[i]);
    }

    char start = L[1];
    char aug = 'Z';

    while (strchr(NT, aug))
        aug--;

    L[0] = aug;
    R[0][0] = start;
    R[0][1] = '\0';

    add(NT, aug);

    for (int i = 0; i <= n; i++)
        for (int j = 0; R[i][j]; j++)
        {
            if (isNT(R[i][j]))
                add(NT, R[i][j]);
            else
                add(T, R[i][j]);
        }

    add(T, '$');

    ns = strlen(NT);
    ts = strlen(T);

    findSets();

    char symbols[MAX] = "";

    for (int i = 0; i < ns; i++)
        if (NT[i] != aug)
            add(symbols, NT[i]);

    for (int i = 0; i < ts; i++)
        if (T[i] != '$')
            add(symbols, T[i]);

    memset(trans, -1, sizeof(trans));

    S[0].size = 0;
    insert(&S[0], 0, 0);
    closure(&S[0]);
    count = 1;

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; symbols[j]; j++)
        {
            State temp;
            goTo(&S[i], symbols[j], &temp);

            if (!temp.size)
                continue;

            int k = findState(&temp);

            if (k == -1)
            {
                k = count;
                S[count++] = temp;
            }

            trans[i][(int)symbols[j]] = k;
        }
    }

    int conflict = 0;

    for (int i = 0; i < count; i++)
    {
        int actions[128] = {0};

        for (int j = 0; j < S[i].size; j++)
        {
            int p = S[i].a[j].p;
            int d = S[i].a[j].d;

            if (d < strlen(R[p]))
            {
                char c = R[p][d];

                if (!isNT(c))
                    actions[(int)c]++;
            }
            else if (p == 0)
            {
                actions['$']++;
            }
            else
            {
                for (int k = 0; FOLLOW[(int)L[p]][k]; k++)
                    actions[(int)FOLLOW[(int)L[p]][k]]++;
            }
        }

        for (int j = 0; j < 128; j++)
            if (actions[j] > 1)
                conflict = 1;
    }

    if (conflict)
        printf("\nGrammar is NOT SLR(1).\n");
    else
        printf("\nGrammar is SLR(1).\n");

    return 0;
}
