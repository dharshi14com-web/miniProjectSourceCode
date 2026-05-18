// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, deletes data previously in the file,
// and displays account information in a friendly way.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "credit.dat"
#define TEXT_FILE "accounts.txt"
#define MAX_ACCOUNTS 100
#define LAST_NAME_SIZE 15
#define FIRST_NAME_SIZE 10

struct clientData
{
    unsigned int acctNum;                // account number
    char lastName[LAST_NAME_SIZE];       // account last name
    char firstName[FIRST_NAME_SIZE];     // account first name
    double balance;                      // account balance
};

// prototypes
unsigned int enterChoice(void);
unsigned int getAccountNumber(void);
double getDoubleValue(const char *prompt);
void clearInput(void);
void createDataFileIfNeeded(FILE **fPtr);
void textFile(FILE *readPtr);
void listAccounts(FILE *fPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

int main(void)
{
    FILE *cfPtr;            // credit.dat file pointer
    unsigned int choice;    // user's choice

    createDataFileIfNeeded(&cfPtr);

    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 5:
            listAccounts(cfPtr);
            break;
        default:
            puts("Invalid choice. Please enter a number from 1 to 6.");
            break;
        }
    }

    fclose(cfPtr);
    puts("Goodbye! Thank you for using the account manager.");
    return 0;
}

void createDataFileIfNeeded(FILE **fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};

    *fPtr = fopen(DATA_FILE, "r+b");
    if (*fPtr == NULL)
    {
        *fPtr = fopen(DATA_FILE, "w+b");
        if (*fPtr == NULL)
        {
            fprintf(stderr, "Unable to create or open %s\n", DATA_FILE);
            exit(EXIT_FAILURE);
        }

        for (unsigned int i = 0; i < MAX_ACCOUNTS; ++i)
        {
            fwrite(&blankClient, sizeof(struct clientData), 1, *fPtr);
        }
    }
}

unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    puts("\n=== Account Transaction Menu ===");
    puts("1 - save a formatted accounts report to accounts.txt");
    puts("2 - update an existing account");
    puts("3 - create a new account");
    puts("4 - delete an account");
    puts("5 - list all accounts on screen");
    puts("6 - exit program");
    printf("Select an option (1-6): ");

    if (scanf("%u", &menuChoice) != 1)
    {
        clearInput();
        return 0;
    }

    return menuChoice;
}

unsigned int getAccountNumber(void)
{
    unsigned int account;

    while (1)
    {
        printf("Enter account number (1-%u): ", MAX_ACCOUNTS);
        if (scanf("%u", &account) != 1)
        {
            clearInput();
            puts("Invalid input. Please enter a positive number.");
            continue;
        }

        if (account < 1 || account > MAX_ACCOUNTS)
        {
            printf("Account number must be between 1 and %u.\n", MAX_ACCOUNTS);
            continue;
        }

        clearInput();
        return account;
    }
}

double getDoubleValue(const char *prompt)
{
    double value;

    while (1)
    {
        printf("%s", prompt);
        if (scanf("%lf", &value) != 1)
        {
            clearInput();
            puts("Invalid number. Please try again.");
            continue;
        }

        clearInput();
        return value;
    }
}

void clearInput(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        ;
    }
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    writePtr = fopen(TEXT_FILE, "w");
    if (writePtr == NULL)
    {
        fprintf(stderr, "Unable to create %s\n", TEXT_FILE);
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);
    printf("Report saved to %s successfully.\n", TEXT_FILE);
}

void listAccounts(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int count = 0;

    rewind(fPtr);
    puts("\nActive accounts:");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                   client.balance);
            ++count;
        }
    }

    if (count == 0)
    {
        puts("No accounts are currently stored.");
    }
    else
    {
        printf("\nTotal active accounts: %u\n", count);
    }
}

void updateRecord(FILE *fPtr)
{
    unsigned int account = getAccountNumber();
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Unable to read account information.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", account);
        return;
    }

    printf("Current account:\n");
    printf("%-6u%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    transaction = getDoubleValue("Enter charge (+) or payment (-): ");
    client.balance += transaction;

    fseek(fPtr, -((long)sizeof(struct clientData)), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Updated account:\n");
    printf("%-6u%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
}

void deleteRecord(FILE *fPtr)
{
    unsigned int account = getAccountNumber();
    struct clientData client = {0, "", "", 0.0};
    struct clientData blankClient = {0, "", "", 0.0};
    int ch;

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Unable to read account information.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", account);
        return;
    }

    printf("Delete account %u for %s %s? (y/n): ", client.acctNum, client.firstName, client.lastName);
    ch = getchar();
    clearInput();

    if (ch != 'y' && ch != 'Y')
    {
        puts("Deletion canceled.");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    printf("Account %u has been deleted.\n", account);
}

void newRecord(FILE *fPtr)
{
    unsigned int accountNum = getAccountNumber();
    struct clientData client = {0, "", "", 0.0};

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Unable to read account information.");
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account %u already exists.\n", accountNum);
        return;
    }

    printf("Enter last name (max %d chars): ", LAST_NAME_SIZE - 1);
    scanf("%14s", client.lastName);
    clearInput();

    printf("Enter first name (max %d chars): ", FIRST_NAME_SIZE - 1);
    scanf("%9s", client.firstName);
    clearInput();

    client.balance = getDoubleValue("Enter opening balance: ");
    client.acctNum = accountNum;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Account %u was created for %s %s with balance %.2f.\n",
           client.acctNum, client.firstName, client.lastName, client.balance);
}
