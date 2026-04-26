
// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void totalBalance(FILE *fPtr);   // NEW FUNCTION
void averageBalance(FILE *fPtr);
void lowBalanceWarning(FILE *fPtr);
void interestCalculator(FILE *fPtr);
void searchByName(FILE *fPtr);
void miniBankReport(FILE *fPtr);
int isActiveAccount(struct clientData client);

int main(int argc, char *argv[])
{
    FILE *cfPtr;
    unsigned int choice;

    (void)argc;

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 11)
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
            totalBalance(cfPtr);   // NEW CASE
            break;
        case 6:
            averageBalance(cfPtr);
            break;
        case 7:
            lowBalanceWarning(cfPtr);
            break;
        case 8:
            interestCalculator(cfPtr);
            break;
        case 9:
            searchByName(cfPtr);
            break;
        case 10:
            miniBankReport(cfPtr);
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }

    fclose(cfPtr);
}

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    int result;
    struct clientData client = {0, "", "", 0.0};
    double total = 0.0;
    int count = 0;
    double average = 0.0;

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    }
    else
    {
        rewind(readPtr);
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        while ((result = fread(&client, sizeof(struct clientData), 1, readPtr)) == 1)
        {
            if (isActiveAccount(client))
            {
                fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                        client.acctNum,
                        client.lastName,
                        client.firstName,
                        client.balance);
                total += client.balance;
                count++;
            }
        }
        
        // Add summary section
        fprintf(writePtr, "\n");
        fprintf(writePtr, "================================================\n");
        fprintf(writePtr, "ACCOUNT SUMMARY\n");
        fprintf(writePtr, "================================================\n");
        fprintf(writePtr, "Total number of active accounts: %d\n", count);
        fprintf(writePtr, "Total balance in bank: $%.2f\n", total);
        if (count > 0)
        {
            average = total / count;
            fprintf(writePtr, "Average balance per account: $%.2f\n", average);
        }
        
        fclose(writePtr);
    }
}

// update balance
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    printf("Enter account to update ( 1 - 100 ): ");
    scanf("%u", &account);

    if (account < 1 || account > 100)
    {
        printf("Invalid account number. Please enter a number from 1 to 100.\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (!isActiveAccount(client))
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    {
        printf("%-6u%-16s%-11s%10.2f\n\n",
               client.acctNum,
               client.lastName,
               client.firstName,
               client.balance);

        printf("Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);

        client.balance += transaction;

        printf("%-6u%-16s%-11s%10.2f\n",
               client.acctNum,
               client.lastName,
               client.firstName,
               client.balance);

        /* rewind one record; fseek takes a long offset so cast explicitly
           to avoid overflow warnings on 64-bit systems */
        fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}

// delete record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0};
    unsigned int accountNum;

    printf("Enter account number to delete ( 1 - 100 ): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100)
    {
        printf("Invalid account number. Please enter a number from 1 to 100.\n");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (!isActiveAccount(client))
    {
        printf("Account %u does not exist.\n", accountNum);
    }
    else
    {
        char confirm;

        printf("Are you sure you want to delete account %u? (y/n): ", accountNum);
        scanf(" %c", &confirm);

        if (confirm != 'y' && confirm != 'Y')
        {
            puts("Delete cancelled.");
            return;
        }

        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        puts("Account deleted successfully.");
    }
}

// create record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter new account number ( 1 - 100 ): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100)
    {
        printf("Invalid account number. Please enter a number from 1 to 100.\n");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (isActiveAccount(client))
    {
        printf("Account #%u already contains information.\n", client.acctNum);
    }
    else
    {
        printf("Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;

        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}

// menu
unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\nEnter your choice\n"
           "1 - store a formatted text file of accounts called\n"
           "    \"accounts.txt\" for printing\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - show total balance in bank\n"
           "6 - show average balance\n"
           "7 - low balance warning\n"
           "8 - calculate interest\n"
           "9 - search customer by name\n"
           "10 - mini bank report\n"
           "11 - end program\n? ");

    scanf("%u", &menuChoice);
    return menuChoice;
}

// NEW FUNCTION: TOTAL BALANCE
void totalBalance(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    double total = 0.0;

    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr))
    {
        if (isActiveAccount(client))
        {
            total += client.balance;
        }
    }

    printf("\nTotal balance in bank: %.2f\n", total);
}

// NEW FUNCTION: AVERAGE BALANCE
void averageBalance(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    double total = 0.0;
    int count = 0;
    double average = 0.0;

    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr))
    {
        if (isActiveAccount(client))
        {
            total += client.balance;
            count++;
        }
    }

    if (count > 0)
    {
        average = total / count;
        printf("\nNumber of active accounts: %d\n", count);
        printf("Total balance: %.2f\n", total);
        printf("Average balance per account: %.2f\n", average);
    }
    else
    {
        printf("\nNo active accounts found.\n");
    }
}

int isActiveAccount(struct clientData client)
{
    return client.acctNum >= 1 && client.acctNum <= 100;
}

void lowBalanceWarning(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    double limit;
    int found = 0;

    printf("Enter low balance limit: ");
    scanf("%lf", &limit);

    rewind(fPtr);
    printf("\nAccounts below %.2f\n", limit);
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (isActiveAccount(client) && client.balance < limit)
        {
            printf("%-6u%-16s%-11s%10.2f\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No low balance accounts found.");
    }
}

void interestCalculator(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    double rate;
    double interest;
    int updated = 0;

    printf("Enter interest rate percentage: ");
    scanf("%lf", &rate);

    if (rate <= 0.0)
    {
        puts("Interest rate must be greater than zero.");
        return;
    }

    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (isActiveAccount(client))
        {
            interest = client.balance * rate / 100.0;
            client.balance += interest;

            fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
            fflush(fPtr);
            updated++;
        }
    }

    printf("Interest added to %d account(s).\n", updated);
}

void searchByName(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    char name[15];
    int found = 0;

    printf("Enter first name or last name to search: ");
    scanf("%14s", name);

    rewind(fPtr);
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (isActiveAccount(client) &&
            (strcmp(client.firstName, name) == 0 || strcmp(client.lastName, name) == 0))
        {
            printf("%-6u%-16s%-11s%10.2f\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No account found with that name.");
    }
}

void miniBankReport(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    struct clientData highest = {0, "", "", 0.0};
    struct clientData lowest = {0, "", "", 0.0};
    double total = 0.0;
    double average;
    int count = 0;

    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (isActiveAccount(client))
        {
            if (count == 0 || client.balance > highest.balance)
            {
                highest = client;
            }

            if (count == 0 || client.balance < lowest.balance)
            {
                lowest = client;
            }

            total += client.balance;
            count++;
        }
    }

    if (count == 0)
    {
        puts("No active accounts found.");
        return;
    }

    average = total / count;

    printf("\nMINI BANK REPORT\n");
    printf("================\n");
    printf("Total active accounts: %d\n", count);
    printf("Total balance: %.2f\n", total);
    printf("Average balance: %.2f\n", average);
    printf("Highest balance account: %u %s %s %.2f\n",
           highest.acctNum,
           highest.lastName,
           highest.firstName,
           highest.balance);
    printf("Lowest balance account: %u %s %s %.2f\n",
           lowest.acctNum,
           lowest.lastName,
           lowest.firstName,
           lowest.balance);
}
