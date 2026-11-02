
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

char productions[MAX][MAX];
char nonTerminals[MAX];
char terminals[MAX];
char first[MAX][MAX];
char follow[MAX][MAX];

int table[MAX][MAX];

int n, ntCount = 0, tCount = 0;

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

int getIndex(char c)
{
    for (int i = 0; i < ntCount; i++)
        if (nonTerminals[i] == c)
            return i;
    return -1;
}

int getTerminal(char c)
{
    for (int i = 0; i < tCount; i++)
        if (terminals[i] == c)
            return i;
    return -1;
}

void firstOfString(char str[], char result[])
{
    int nullable = 1;

    for (int i = 0; str[i] != '\0'; i++)
    {
        char c = str[i];

        if (c == '#')
        {
            add(result, '#');
            return;
        }

        if (!isupper(c))
        {
            add(result, c);
            nullable = 0;
            break;
        }

        int k = getIndex(c);

        for (int j = 0; first[k][j] != '\0'; j++)
            if (first[k][j] != '#')
                add(result, first[k][j]);

        if (strchr(first[k], '#') == NULL)
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

        for (int i = 0; i < n; i++)
        {
            int k = getIndex(productions[i][0]);
            char temp[MAX] = "";

            firstOfString(productions[i] + 2, temp);

            for (int j = 0; temp[j] != '\0'; j++)
                changed |= add(first[k], temp[j]);
        }

    } while (changed);
}

void findFollow()
{
    add(follow[0], '$');

    int changed;

    do
    {
        changed = 0;

        for (int i = 0; i < n; i++)
        {
            int a = getIndex(productions[i][0]);
            char *rhs = productions[i] + 2;

            for (int j = 0; rhs[j] != '\0'; j++)
            {
                if (!isupper(rhs[j]))
                    continue;

                int b = getIndex(rhs[j]);
                char temp[MAX] = "";

                firstOfString(rhs + j + 1, temp);

                for (int p = 0; temp[p] != '\0'; p++)
                    if (temp[p] != '#')
                        changed |= add(follow[b], temp[p]);

                if (strchr(temp, '#') != NULL)
                {
                    for (int p = 0; follow[a][p] != '\0'; p++)
                        changed |= add(follow[b], follow[a][p]);
                }
            }
        }

    } while (changed);
}

void constructTable()
{
    for (int i = 0; i < MAX; i++)
        for (int j = 0; j < MAX; j++)
            table[i][j] = -1;

    for (int i = 0; i < n; i++)
    {
        int a = getIndex(productions[i][0]);
        char temp[MAX] = "";

        firstOfString(productions[i] + 2, temp);

        for (int j = 0; temp[j] != '\0'; j++)
        {
            if (temp[j] != '#')
            {
                int k = getTerminal(temp[j]);

                if (k != -1)
                {
                    if (table[a][k] != -1 &&
                        table[a][k] != i)
                        table[a][k] = -2;
                    else
                        table[a][k] = i;
                }
            }
        }

        if (strchr(temp, '#') != NULL)
        {
            for (int j = 0; follow[a][j] != '\0'; j++)
            {
                int k = getTerminal(follow[a][j]);

                if (k != -1)
                {
                    if (table[a][k] != -1 &&
                        table[a][k] != i)
                        table[a][k] = -2;
                    else
                        table[a][k] = i;
                }
            }
        }
    }
}

void displayTable()
{
    printf("\nLL(1) PARSING TABLE\n\n");

    printf("%-12s", "NT");

    for (int i = 0; i < tCount; i++)
        printf("%-12c", terminals[i]);

    printf("\n");

    for (int i = 0; i < ntCount; i++)
    {
        printf("%-12c", nonTerminals[i]);

        for (int j = 0; j < tCount; j++)
        {
            if (table[i][j] == -1)
                printf("%-12s", "-");
            else if (table[i][j] == -2)
                printf("%-12s", "CONFLICT");
            else
                printf("%-12s", productions[table[i][j]]);
        }

        printf("\n");
    }
}

void parseString(char input[])
{
    char stack[100];
    int top = 0, ip = 0;

    stack[top++] = '$';
    stack[top++] = nonTerminals[0];

    if (strcmp(input, "#") == 0)
        input[0] = '\0';

    int len = strlen(input);
    input[len] = '$';
    input[len + 1] = '\0';

    printf("\n%-20s %-20s %-20s\n",
           "STACK", "INPUT", "ACTION");

    while (top > 0)
    {
        char X = stack[top - 1];
        char a = input[ip];

        printf("%-20s ", "");
        for (int i = 0; i < top; i++)
            printf("%c", stack[i]);

        printf("%*s", 20 - top, "");

        printf("%-20s ", input + ip);

        if (X == '$' && a == '$')
        {
            printf("ACCEPT\n");
            return;
        }

        if (!isupper(X))
        {
            if (X == a)
            {
                top--;
                ip++;
                printf("Match %c\n", a);
            }
            else
            {
                printf("REJECT\n");
                return;
            }
        }
        else
        {
            int r = getIndex(X);
            int c = getTerminal(a);

            if (c == -1 || table[r][c] < 0)
            {
                printf("REJECT\n");
                return;
            }

            int p = table[r][c];

            printf("%s\n", productions[p]);

            top--;

            char *rhs = productions[p] + 2;

            if (rhs[0] != '#')
            {
                for (int i = strlen(rhs) - 1; i >= 0; i--)
                    stack[top++] = rhs[i];
            }
        }
    }

    printf("REJECT\n");
}

int main()
{
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (example S=AB):\n");

    for (int i = 0; i < n; i++)
    {
        scanf("%s", productions[i]);

        char A = productions[i][0];

        if (getIndex(A) == -1)
            nonTerminals[ntCount++] = A;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 2; productions[i][j] != '\0'; j++)
        {
            char c = productions[i][j];

            if (!isupper(c) && c != '#')
                add(terminals, c);
        }
    }

    add(terminals, '$');
    tCount = strlen(terminals);

    findFirst();
    findFollow();
    constructTable();

    displayTable();

    int conflict = 0;

    for (int i = 0; i < ntCount; i++)
        for (int j = 0; j < tCount; j++)
            if (table[i][j] == -2)
                conflict = 1;

    if (conflict)
    {
        printf("\nGrammar is NOT LL(1).\n");
        return 0;
    }

    printf("\nGrammar is LL(1) .\n");

    char input[100];

    printf("\nEnter input string (# for epsilon): ");
    scanf("%99s", input);

    parseString(input);

    return 0;
}
