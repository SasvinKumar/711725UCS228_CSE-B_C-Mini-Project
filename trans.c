



// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.

#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[])
{
    FILE *cfPtr;
    unsigned int choice;

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 7)   // CHANGED from 6 to 7
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

        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n",
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
    scanf("%d", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    {
        printf("%-6d%-16s%-11s%10.2f\n\n",
               client.acctNum,
               client.lastName,
               client.firstName,
               client.balance);

        printf("Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);

        client.balance += transaction;

        printf("%-6d%-16s%-11s%10.2f\n",
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
    scanf("%d", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    }
    else
    {
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    }
}

// create record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
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
           "7 - end program\n? ");

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
        if (client.acctNum != 0)
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
        if (client.acctNum != 0)
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