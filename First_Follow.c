
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

char productions[MAX][MAX];
char nonTerminals[MAX];
char first[MAX][MAX];
char follow[MAX][MAX];

int n, ntCount = 0;

int isNonTerminal(char c)
{
    return isupper(c);
}

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

void findFirst(int index)
{
    char A = nonTerminals[index];

    for (int i = 0; i < n; i++)
    {
        if (productions[i][0] != A)
            continue;

        char *rhs = productions[i] + 2;
        int epsilonFound = 1;

        for (int j = 0; rhs[j] != '\0'; j++)
        {
            char X = rhs[j];

            if (X == '#')
            {
                add(first[index], '#');
                epsilonFound = 0;
                break;
            }

            if (!isNonTerminal(X))
            {
                add(first[index], X);
                epsilonFound = 0;
                break;
            }

            int k = getIndex(X);

            if (strlen(first[k]) == 0)
                findFirst(k);

            for (int p = 0; first[k][p] != '\0'; p++)
                if (first[k][p] != '#')
                    add(first[index], first[k][p]);

            if (strchr(first[k], '#') == NULL)
            {
                epsilonFound = 0;
                break;
            }
        }

        if (epsilonFound)
            add(first[index], '#');
    }
}

void firstOfString(char str[], char result[])
{
    int epsilonFound = 1;

    for (int i = 0; str[i] != '\0'; i++)
    {
        char X = str[i];

        if (X == '#')
        {
            add(result, '#');
            return;
        }

        if (!isNonTerminal(X))
        {
            add(result, X);
            epsilonFound = 0;
            break;
        }

        int k = getIndex(X);

        for (int j = 0; first[k][j] != '\0'; j++)
            if (first[k][j] != '#')
                add(result, first[k][j]);

        if (strchr(first[k], '#') == NULL)
        {
            epsilonFound = 0;
            break;
        }
    }

    if (epsilonFound)
        add(result, '#');
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
                if (!isNonTerminal(rhs[j]))
                    continue;

                int b = getIndex(rhs[j]);
                char beta[MAX] = "";

                strcpy(beta, rhs + j + 1);

                char temp[MAX] = "";
                firstOfString(beta, temp);

                for (int p = 0; temp[p] != '\0'; p++)
                    if (temp[p] != '#')
                        changed |= add(follow[b], temp[p]);

                if (strchr(temp, '#') != NULL || beta[0] == '\0')
                    for (int p = 0; follow[a][p] != '\0'; p++)
                        changed |= add(follow[b], follow[a][p]);
            }
        }

    } while (changed);
}

int main()
{
    printf("Enter number of productions: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        scanf("%s", productions[i]);

        char A = productions[i][0];

        if (getIndex(A) == -1)
            nonTerminals[ntCount++] = A;
    }

    for (int i = 0; i < ntCount; i++)
        findFirst(i);

    findFollow();

    printf("\nFIRST sets:\n");

    for (int i = 0; i < ntCount; i++)
    {
        printf("FIRST(%c) = { ", nonTerminals[i]);

        for (int j = 0; first[i][j] != '\0'; j++)
        {
            printf("%c", first[i][j]);

            if (first[i][j + 1] != '\0')
                printf(", ");
        }

        printf(" }\n");
    }

    printf("\nFOLLOW sets:\n");

    for (int i = 0; i < ntCount; i++)
    {
        printf("FOLLOW(%c) = { ", nonTerminals[i]);

        for (int j = 0; follow[i][j] != '\0'; j++)
        {
            printf("%c", follow[i][j]);

            if (follow[i][j + 1] != '\0')
                printf(", ");
        }

        printf(" }\n");
    }
    return 0;
}
