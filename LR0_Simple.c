
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20
#define STATES 100
#define ITEMS 100

char lhs[MAX], rhs[MAX][MAX];
char terminals[MAX], nonterminals[MAX], symbols[MAX];
int n, nt = 0, t = 0, sc = 0, count = 0;

typedef struct
{
    int p, d;
} Item;

typedef struct
{
    Item a[ITEMS];
    int size;
} State;

State s[STATES];
int trans[STATES][128];

int add(char *str, char c)
{
    if (!strchr(str, c))
    {
        int len = strlen(str);
        str[len] = c;
        str[len + 1] = '\0';
        return 1;
    }
    return 0;
}

int isNT(char c)
{
    return isupper((unsigned char)c);
}

void insert(State *x, int p, int d)
{
    for (int i = 0; i < x->size; i++)
        if (x->a[i].p == p && x->a[i].d == d)
            return;

    x->a[x->size].p = p;
    x->a[x->size].d = d;
    x->size++;
}

void closure(State *x)
{
    int change;

    do
    {
        change = 0;

        for (int i = 0; i < x->size; i++)
        {
            int p = x->a[i].p;
            int d = x->a[i].d;

            if (d < strlen(rhs[p]) && isNT(rhs[p][d]))
            {
                char c = rhs[p][d];

                for (int j = 0; j <= n; j++)
                    if (lhs[j] == c)
                    {
                        int old = x->size;
                        insert(x, j, 0);
                        if (old != x->size)
                            change = 1;
                    }
            }
        }
    } while (change);
}

void goTo(State *x, char c, State *y)
{
    y->size = 0;

    for (int i = 0; i < x->size; i++)
    {
        int p = x->a[i].p;
        int d = x->a[i].d;

        if (rhs[p][d] == c)
            insert(y, p, d + 1);
    }

    if (y->size)
        closure(y);
}

int equal(State *x, State *y)
{
    if (x->size != y->size)
        return 0;

    for (int i = 0; i < x->size; i++)
    {
        int found = 0;

        for (int j = 0; j < y->size; j++)
            if (x->a[i].p == y->a[j].p &&
                x->a[i].d == y->a[j].d)
                found = 1;

        if (!found)
            return 0;
    }

    return 1;
}

int find(State *x)
{
    for (int i = 0; i < count; i++)
        if (equal(x, &s[i]))
            return i;

    return -1;
}

int main()
{
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions:\n");

    for (int i = 1; i <= n; i++)
    {
        char str[MAX];
        scanf("%19s", str);

        lhs[i] = str[0];

        if (strcmp(str + 2, "#") == 0)
            rhs[i][0] = '\0';
        else
            strcpy(rhs[i], str + 2);

        add(nonterminals, lhs[i]);
    }

    lhs[0] = 'Z';
    rhs[0][0] = lhs[1];
    rhs[0][1] = '\0';

    add(nonterminals, 'Z');

    for (int i = 0; i <= n; i++)
        for (int j = 0; rhs[i][j]; j++)
        {
            char c = rhs[i][j];

            if (isNT(c))
                add(nonterminals, c);
            else
                add(terminals, c);
        }

    add(terminals, '$');

    nt = strlen(nonterminals);
    t = strlen(terminals);

    for (int i = 0; i < nt; i++)
        add(symbols, nonterminals[i]);

    for (int i = 0; i < t; i++)
        if (terminals[i] != '$')
            add(symbols, terminals[i]);

    sc = strlen(symbols);

    memset(trans, -1, sizeof(trans));

    s[0].size = 0;
    insert(&s[0], 0, 0);
    closure(&s[0]);

    count = 1;

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < sc; j++)
        {
            State temp;
            goTo(&s[i], symbols[j], &temp);

            if (!temp.size)
                continue;

            int k = find(&temp);

            if (k == -1)
            {
                k = count;
                s[count++] = temp;
            }

            trans[i][(int)symbols[j]] = k;
        }
    }

    int conflict = 0;

    for (int i = 0; i < count; i++)
    {
        int actions[MAX] = {0};

        for (int j = 0; j < s[i].size; j++)
        {
            int p = s[i].a[j].p;
            int d = s[i].a[j].d;
            int len = strlen(rhs[p]);

            if (d < len)
            {
                char c = rhs[p][d];

                if (!isNT(c))
                {
                    int k = strchr(terminals, c) - terminals;
                    actions[k]++;
                }
            }
            else if (p == 0)
            {
                actions[strchr(terminals, '$') - terminals]++;
            }
            else
            {
                for (int k = 0; k < t; k++)
                    actions[k]++;
            }
        }

        for (int j = 0; j < t; j++)
            if (actions[j] > 1)
                conflict = 1;
    }

    if (conflict)
        printf("\nGrammar is NOT LR(0).\n");
    else
        printf("\nGrammar is LR(0).\n");

    return 0;
}
