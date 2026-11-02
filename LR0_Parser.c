
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX 20
#define STATES 100
#define ITEMS 100

char lhs[MAX + 1];
char rhs[MAX + 1][MAX];
char terminals[MAX];
char nonTerminals[MAX];
char symbols[MAX * 2];

int n, tCount = 0, ntCount = 0, symCount = 0;
char startSymbol, augmented;

typedef struct
{
    int p;
    int dot;
} Item;

typedef struct
{
    Item item[ITEMS];
    int count;
} State;

State states[STATES];

int trans[STATES][128];
int stateCount = 0;

char action[STATES][MAX][20];
int goTo[STATES][MAX];

int add(char set[], char c)
{
    if (strchr(set, c) == NULL)
    {
        int len = strlen(set);
        set[len] = c;
        set[len + 1] = '\0';
        return 1;
    }
    return 0;
}

int isNT(char c)
{
    return isupper((unsigned char)c);
}

int getTerminal(char c)
{
    for (int i = 0; i < tCount; i++)
        if (terminals[i] == c)
            return i;
    return -1;
}

int getNT(char c)
{
    for (int i = 0; i < ntCount; i++)
        if (nonTerminals[i] == c)
            return i;
    return -1;
}

int addItem(State *s, int p, int dot)
{
    for (int i = 0; i < s->count; i++)
    {
        if (s->item[i].p == p &&
            s->item[i].dot == dot)
            return 0;
    }

    if (s->count >= ITEMS)
        return 0;

    s->item[s->count].p = p;
    s->item[s->count].dot = dot;
    s->count++;

    return 1;
}

void closure(State *s)
{
    int changed;

    do
    {
        changed = 0;

        for (int i = 0; i < s->count; i++)
        {
            int p = s->item[i].p;
            int dot = s->item[i].dot;

            if (dot < strlen(rhs[p]))
            {
                char c = rhs[p][dot];

                if (isNT(c))
                {
                    for (int j = 0; j <= n; j++)
                    {
                        if (lhs[j] == c)
                            changed |= addItem(s, j, 0);
                    }
                }
            }
        }

    } while (changed);
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
            if (a->item[i].p == b->item[j].p &&
                a->item[i].dot == b->item[j].dot)
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

void goToState(State *s, char c, State *result)
{
    result->count = 0;

    for (int i = 0; i < s->count; i++)
    {
        int p = s->item[i].p;
        int dot = s->item[i].dot;

        if (dot < strlen(rhs[p]) &&
            rhs[p][dot] == c)
        {
            addItem(result, p, dot + 1);
        }
    }

    if (result->count > 0)
        closure(result);
}

int findState(State *s)
{
    for (int i = 0; i < stateCount; i++)
        if (sameState(&states[i], s))
            return i;

    return -1;
}

void buildStates()
{
    for (int i = 0; i < STATES; i++)
        for (int j = 0; j < 128; j++)
            trans[i][j] = -1;

    states[0].count = 0;
    addItem(&states[0], 0, 0);
    closure(&states[0]);

    stateCount = 1;

    for (int i = 0; i < stateCount; i++)
    {
        for (int j = 0; j < symCount; j++)
        {
            char c = symbols[j];

            State temp;
            goToState(&states[i], c, &temp);

            if (temp.count == 0)
                continue;

            int k = findState(&temp);

            if (k == -1)
            {
                if (stateCount >= STATES)
                    return;

                k = stateCount;
                states[stateCount++] = temp;
            }

            trans[i][(int)c] = k;
        }
    }
}

void setAction(int state, int col, char value[])
{
    if (action[state][col][0] == '\0')
        strcpy(action[state][col], value);
    else if (strcmp(action[state][col], value) != 0)
        strcpy(action[state][col], "CONFLICT");
}

void constructTable()
{
    for (int i = 0; i < stateCount; i++)
    {
        for (int j = 0; j < tCount; j++)
            action[i][j][0] = '\0';

        for (int j = 0; j < ntCount; j++)
            goTo[i][j] = -1;
    }

    for (int i = 0; i < stateCount; i++)
    {
        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].item[j].p;
            int dot = states[i].item[j].dot;
            int len = strlen(rhs[p]);

            if (dot < len)
            {
                char c = rhs[p][dot];
                int next = trans[i][(int)c];

                if (!isNT(c))
                {
                    int k = getTerminal(c);
                    char value[20];

                    if (k != -1)
                    {
                        sprintf(value, "s%d", next);
                        setAction(i, k, value);
                    }
                }
                else
                {
                    int k = getNT(c);

                    if (k != -1)
                        goTo[i][k] = next;
                }
            }
            else
            {
                if (p == 0)
                {
                    int k = getTerminal('$');

                    if (k != -1)
                        setAction(i, k, "acc");
                }
                else
                {
                    char value[20];
                    sprintf(value, "r%d", p);

                    for (int k = 0; k < tCount; k++)
                        setAction(i, k, value);
                }
            }
        }
    }
}

void displayItems()
{
    printf("\nCANONICAL LR(0) ITEMS\n");

    for (int i = 0; i < stateCount; i++)
    {
        printf("\nI%d:\n", i);

        for (int j = 0; j < states[i].count; j++)
        {
            int p = states[i].item[j].p;
            int dot = states[i].item[j].dot;

            printf("  %c -> ", lhs[p]);

            int len = strlen(rhs[p]);

            for (int k = 0; k <= len; k++)
            {
                if (k == dot)
                    printf(".");

                if (k < len)
                    printf("%c", rhs[p][k]);
            }

            printf("\n");
        }
    }
}

void displayTable()
{
    printf("\nLR(0) PARSING TABLE\n\n");

    printf("%-8s", "STATE");

    for (int i = 0; i < tCount; i++)
        printf("%-12s", i == 0 ? terminals + i : terminals + i);

    for (int i = 0; i < ntCount; i++)
        printf("%-12c", nonTerminals[i]);

    printf("\n");

    printf("%-8s", "");

    for (int i = 0; i < tCount; i++)
        printf("%-12s", "");

    for (int i = 0; i < ntCount; i++)
        printf("%-12s", "");

    printf("\n");

    for (int i = 0; i < stateCount; i++)
    {
        printf("I%-7d", i);

        for (int j = 0; j < tCount; j++)
        {
            if (action[i][j][0] == '\0')
                printf("%-12s", "-");
            else
                printf("%-12s", action[i][j]);
        }

        for (int j = 0; j < ntCount; j++)
        {
            if (goTo[i][j] == -1)
                printf("%-12s", "-");
            else
            {
                char value[20];
                sprintf(value, "%d", goTo[i][j]);
                printf("%-12s", value);
            }
        }

        printf("\n");
    }
}

void parseString(char input[])
{
    int stack[200];
    int top = 0, ip = 0;

    stack[top] = 0;

    if (strcmp(input, "#") == 0)
        input[0] = '\0';

    int len = strlen(input);
    input[len] = '$';
    input[len + 1] = '\0';

    printf("\n%-25s %-20s %-20s\n",
           "STACK", "INPUT", "ACTION");

    while (top < 198)
    {
        int state = stack[top];
        char c = input[ip];

        int k = getTerminal(c);

        if (k == -1)
        {
            printf("REJECT: Invalid input symbol\n");
            return;
        }

        printf("I");
        for (int i = 0; i <= top; i++)
            printf("%d ", stack[i]);

        printf("%*s", 18 - top * 2, "");
        printf("%-20s ", input + ip);

        char *act = action[state][k];

        if (strcmp(act, "acc") == 0)
        {
            printf("ACCEPT\n");
            return;
        }

        if (act[0] == 's')
        {
            int next = atoi(act + 1);

            printf("SHIFT %d\n", next);

            stack[++top] = next;
            ip++;
        }
        else if (act[0] == 'r')
        {
            int p = atoi(act + 1);
            int count = strlen(rhs[p]);

            printf("REDUCE %c -> %s\n",
                   lhs[p],
                   count ? rhs[p] : "#");

            top -= count;

            int current = stack[top];
            int k = getNT(lhs[p]);

            if (k == -1 || goTo[current][k] == -1)
            {
                printf("REJECT\n");
                return;
            }

            stack[++top] = goTo[current][k];
        }
        else
        {
            printf("REJECT\n");
            return;
        }
    }

    printf("REJECT\n");
}

int main()
{
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (example S=CC):\n");

    for (int i = 1; i <= n; i++)
    {
        char production[MAX];

        scanf("%19s", production);

        lhs[i] = production[0];

        if (strcmp(production + 2, "#") == 0)
            rhs[i][0] = '\0';
        else
            strcpy(rhs[i], production + 2);

        add(nonTerminals, lhs[i]);
    }

    startSymbol = lhs[1];

    augmented = 'Z';

    while (strchr(nonTerminals, augmented) != NULL)
    {
        augmented--;

        if (augmented < 'A')
        {
            printf("Too many non-terminals\n");
            return 0;
        }
    }

    lhs[0] = augmented;
    rhs[0][0] = startSymbol;
    rhs[0][1] = '\0';

    add(nonTerminals, augmented);

    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; rhs[i][j] != '\0'; j++)
        {
            char c = rhs[i][j];

            if (isNT(c))
                add(nonTerminals, c);
            else
                add(terminals, c);
        }
    }

    add(terminals, '$');

    ntCount = strlen(nonTerminals);
    tCount = strlen(terminals);

    for (int i = 0; i < ntCount; i++)
        add(symbols, nonTerminals[i]);

    for (int i = 0; i < tCount; i++)
        if (terminals[i] != '$')
            add(symbols, terminals[i]);

    symCount = strlen(symbols);

    buildStates();

    constructTable();

    displayItems();

    printf("\nACTION                         GOTO\n");

    printf("Terminals: ");
    for (int i = 0; i < tCount; i++)
        printf("%c ", terminals[i]);

    printf("\nNon-terminals: ");
    for (int i = 0; i < ntCount; i++)
        if (nonTerminals[i] != augmented)
            printf("%c ", nonTerminals[i]);

    displayTable();

    int conflict = 0;

    for (int i = 0; i < stateCount; i++)
        for (int j = 0; j < tCount; j++)
            if (strcmp(action[i][j], "CONFLICT") == 0)
                conflict = 1;

    if (conflict)
    {
        printf("\nGrammar is NOT LR(0). Conflicts found.\n");
        return 0;
    }

    printf("\nGrammar is LR(0).\n");

    char input[100];

    printf("\nEnter input string (# for epsilon): ");
    scanf("%99s", input);

    parseString(input);

    return 0;
}
