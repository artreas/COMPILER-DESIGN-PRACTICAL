
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20
#define STATES 100
#define ITEMS 100

char lhs[MAX], rhs[MAX][MAX];
char nt[MAX], t[MAX], symbols[MAX];
char first[128][128], follow[128][128];

int n, ntCount = 0, tCount = 0;
int stateCount = 0, symCount = 0;
char start, augmented;

typedef struct
{
    int p, dot;
} Item;

typedef struct
{
    Item a[ITEMS];
    int count;
} State;

State states[STATES];

int trans[STATES][128];
int goTo[STATES][128];
char action[STATES][128][20];

int add(char *s, char c)
{
    if (!strchr(s, c))
    {
        int len = strlen(s);
        s[len] = c;
        s[len + 1] = '\0';
        return 1;
    }
    return 0;
}

int isNT(char c)
{
    return isupper((unsigned char)c);
}

void firstString(char *str, char *result)
{
    result[0] = '\0';

    if (str[0] == '\0')
    {
        add(result, '#');
        return;
    }

    int nullable = 1;

    for (int i = 0; str[i] && nullable; i++)
    {
        char c = str[i];

        if (!isNT(c))
        {
            add(result, c);
            nullable = 0;
        }
        else
        {
            for (int j = 0; first[(int)c][j]; j++)
                if (first[(int)c][j] != '#')
                    add(result, first[(int)c][j]);

            if (!strchr(first[(int)c], '#'))
                nullable = 0;
        }
    }

    if (nullable)
        add(result, '#');
}

void calculateFirst()
{
    int change;

    do
    {
        change = 0;

        for (int i = 1; i <= n; i++)
        {
            if (rhs[i][0] == '\0')
            {
                change |= add(first[(int)lhs[i]], '#');
                continue;
            }

            int nullable = 1;

            for (int j = 0; rhs[i][j] && nullable; j++)
            {
                char c = rhs[i][j];

                if (!isNT(c))
                {
                    change |= add(first[(int)lhs[i]], c);
                    nullable = 0;
                }
                else
                {
                    for (int k = 0; first[(int)c][k]; k++)
                        if (first[(int)c][k] != '#')
                            change |= add(first[(int)lhs[i]],
                                          first[(int)c][k]);

                    if (!strchr(first[(int)c], '#'))
                        nullable = 0;
                }
            }

            if (nullable)
                change |= add(first[(int)lhs[i]], '#');
        }

    } while (change);
}

void calculateFollow()
{
    add(follow[(int)start], '$');

    int change;

    do
    {
        change = 0;

        for (int i = 1; i <= n; i++)
        {
            for (int j = 0; rhs[i][j]; j++)
            {
                char c = rhs[i][j];

                if (!isNT(c))
                    continue;

                char temp[MAX] = "";

                firstString(rhs[i] + j + 1, temp);

                for (int k = 0; temp[k]; k++)
                    if (temp[k] != '#')
                        change |= add(follow[(int)c], temp[k]);

                if (strchr(temp, '#'))
                {
                    for (int k = 0; follow[(int)lhs[i]][k]; k++)
                        change |= add(follow[(int)c],
                                      follow[(int)lhs[i]][k]);
                }
            }
        }

    } while (change);
}

void insert(State *s, int p, int dot)
{
    for (int i = 0; i < s->count; i++)
        if (s->a[i].p == p && s->a[i].dot == dot)
            return;

    s->a[s->count].p = p;
    s->a[s->count].dot = dot;
    s->count++;
}

void closure(State *s)
{
    int change;

    do
    {
        change = 0;

        for (int i = 0; i < s->count; i++)
        {
            int p = s->a[i].p;
            int d = s->a[i].dot;

            if (d < strlen(rhs[p]) && isNT(rhs[p][d]))
            {
                char c = rhs[p][d];

                for (int j = 0; j <= n; j++)
                {
                    if (lhs[j] == c)
                    {
                        int old = s->count;
                        insert(s, j, 0);

                        if (old != s->count)
                            change = 1;
                    }
                }
            }
        }

    } while (change);
}

void goToState(State *s, char c, State *result)
{
    result->count = 0;

    for (int i = 0; i < s->count; i++)
    {
        int p = s->a[i].p;
        int d = s->a[i].dot;

        if (rhs[p][d] == c)
            insert(result, p, d + 1);
    }

    if (result->count)
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
            if (a->a[i].p == b->a[j].p &&
                a->a[i].dot == b->a[j].dot)
                found = 1;

        if (!found)
            return 0;
    }

    return 1;
}

int findState(State *s)
{
    for (int i = 0; i < stateCount; i++)
        if (sameState(s, &states[i]))
            return i;

    return -1;
}

void buildStates()
{
    memset(trans, -1, sizeof(trans));

    states[0].count = 0;
    insert(&states[0], 0, 0);
    closure(&states[0]);

    stateCount = 1;

    for (int i = 0; i < stateCount; i++)
    {
        for (int j = 0; j < symCount; j++)
        {
            State temp;

            goToState(&states[i], symbols[j], &temp);

            if (!temp.count)
                continue;

            int k = findState(&temp);

            if (k == -1)
            {
                if (stateCount >= STATES)
                    return;

                k = stateCount;
                states[stateCount++] = temp;
            }

            trans[i][(int)symbols[j]] = k;
        }
    }
}

void setAction(int state, char c, char value[])
{
    if (action[state][(int)c][0] == '\0')
        strcpy(action[state][(int)c], value);
    else if (strcmp(action[state][(int)c], value) != 0)
        strcpy(action[state][(int)c], "CONFLICT");
}

void buildTable()
{
    for (int i = 0; i < stateCount; i++)
    {
        for (int c = 0; c < 128; c++)
        {
            action[i][c][0] = '\0';
            goTo[i][c] = -1;
        }
    }

    for (int i = 0; i < stateCount; i++)
    {
        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].a[j].p;
            int d = states[i].a[j].dot;
            int len = strlen(rhs[p]);

            if (d < len)
            {
                char c = rhs[p][d];
                int next = trans[i][(int)c];

                if (isNT(c))
                    goTo[i][(int)c] = next;
                else
                {
                    char value[20];
                    sprintf(value, "s%d", next);
                    setAction(i, c, value);
                }
            }
            else if (p == 0)
            {
                setAction(i, '$', "acc");
            }
            else
            {
                char value[20];
                sprintf(value, "r%d", p);

                for (int k = 0; follow[(int)lhs[p]][k]; k++)
                    setAction(i, follow[(int)lhs[p]][k], value);
            }
        }
    }
}

void displayTable()
{
    printf("\nSLR(1) PARSING TABLE\n\n");

    printf("%-8s", "STATE");
    printf("ACTION");

    for (int i = 0; i < tCount; i++)
        printf("%-10c", t[i]);

    printf("GOTO");

    for (int i = 0; i < ntCount; i++)
        if (nt[i] != augmented)
            printf("%-10c", nt[i]);

    printf("\n");

    for (int i = 0; i < stateCount; i++)
    {
        printf("I%-7d", i);

        for (int j = 0; j < tCount; j++)
        {
            if (action[i][(int)t[j]][0] == '\0')
                printf("%-10s", "-");
            else
                printf("%-10s", action[i][(int)t[j]]);
        }

        for (int j = 0; j < ntCount; j++)
        {
            if (nt[j] == augmented)
                continue;

            if (goTo[i][(int)nt[j]] == -1)
                printf("%-10s", "-");
            else
                printf("%-10d", goTo[i][(int)nt[j]]);
        }

        printf("\n");
    }
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

        lhs[i] = str[0];

        if (strcmp(str + 2, "#") == 0)
            rhs[i][0] = '\0';
        else
            strcpy(rhs[i], str + 2);

        add(nt, lhs[i]);
    }

    start = lhs[1];

    augmented = 'Z';

    while (strchr(nt, augmented))
    {
        augmented--;

        if (augmented < 'A')
        {
            printf("Too many non-terminals\n");
            return 0;
        }
    }

    lhs[0] = augmented;
    rhs[0][0] = start;
    rhs[0][1] = '\0';

    add(nt, augmented);

    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; rhs[i][j]; j++)
        {
            if (isNT(rhs[i][j]))
                add(nt, rhs[i][j]);
            else
                add(t, rhs[i][j]);
        }
    }

    add(t, '$');

    ntCount = strlen(nt);
    tCount = strlen(t);

    for (int i = 0; i < ntCount; i++)
        if (nt[i] != augmented)
            add(symbols, nt[i]);

    for (int i = 0; i < tCount; i++)
        if (t[i] != '$')
            add(symbols, t[i]);

    symCount = strlen(symbols);

    calculateFirst();
    calculateFollow();

    printf("\nFOLLOW SETS:\n");

    for (int i = 0; i < ntCount; i++)
    {
        if (nt[i] == augmented)
            continue;

        printf("FOLLOW(%c) = { ", nt[i]);

        for (int j = 0; follow[(int)nt[i]][j]; j++)
            printf("%c ", follow[(int)nt[i]][j]);

        printf("}\n");
    }

    buildStates();
    buildTable();
    displayTable();

    int conflict = 0;

    for (int i = 0; i < stateCount; i++)
        for (int j = 0; j < tCount; j++)
            if (strcmp(action[i][(int)t[j]], "CONFLICT") == 0)
                conflict = 1;

    if (conflict)
        printf("\nGrammar is NOT SLR(1).\n");
    else
        printf("\nGrammar is SLR(1).\n");

    return 0;
}
