// Bank-account program with Role-Based Access Control
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

double currentMinBalance = 500.0;
FILE *logFile = NULL;
#define PRINT(...) do { printf(__VA_ARGS__); if(logFile) fprintf(logFile, __VA_ARGS__); } while(0)

struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
    char password[20];
    char createdAt[30];
};

struct Transaction {
    unsigned int acctNum;
    char type[15];
    double amount;
    char date[30];
};

// Prototypes
void clearInputBuffer();
void getCurrentTime(char *buffer, int size);
void createAdmin();
int loginAdmin();
int loginUser(FILE *cfPtr, unsigned int *loggedInAcct);
void recordTransaction(unsigned int acctNum, const char* type, double amount);
int isActiveAccount(struct clientData client);

// Admin functions
void adminMenu(FILE *cfPtr);
void displayAllRecords(FILE *cfPtr);
void searchAccount(FILE *cfPtr);
void addAccount(FILE *cfPtr);
void deleteAccount(FILE *cfPtr);
void exportToFile(FILE *cfPtr);
void accountSummary(FILE *cfPtr);
void viewAllTransactions();
void viewUserCredentials(FILE *cfPtr);
void setMinimumBalance();
void loadConfig();
void saveConfig();

// User functions
void userMenu(FILE *cfPtr, unsigned int acctNum);
void viewBalanceAndDetails(FILE *cfPtr, unsigned int acctNum);
void deposit(FILE *cfPtr, unsigned int acctNum);
void withdraw(FILE *cfPtr, unsigned int acctNum);
void transferFunds(FILE *cfPtr, unsigned int acctNum);
void viewUserTransactions(unsigned int acctNum);

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void getCurrentTime(char *buffer, int size) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

void createAdmin() {
    FILE *adminFile = fopen("admin.txt", "r");
    if (!adminFile) {
        adminFile = fopen("admin.txt", "w");
        if (adminFile) {
            fprintf(adminFile, "AdminID: admin\n");
            fprintf(adminFile, "Password: admin123\n");
            fclose(adminFile);
        }
    } else {
        fclose(adminFile);
    }
}

int loginAdmin() {
    char id[20], pw[20];
    char fileId[20], filePw[20];
    FILE *adminFile = fopen("admin.txt", "r");
    if (!adminFile) {
        printf("Error: admin.txt not found.\n");
        return 0;
    }
    fscanf(adminFile, "AdminID: %19s\n", fileId);
    fscanf(adminFile, "Password: %19s\n", filePw);
    fclose(adminFile);

    printf("\n--- Admin Login ---\n");
    printf("Admin ID: ");
    scanf("%19s", id);
    printf("Password: ");
    scanf("%19s", pw);
    clearInputBuffer();

    if (strcmp(id, fileId) == 0 && strcmp(pw, filePw) == 0) {
        printf("Login successful!\n");
        return 1;
    } else {
        printf("Invalid credentials.\n");
        return 0;
    }
}

int loginUser(FILE *cfPtr, unsigned int *loggedInAcct) {
    unsigned int acct;
    char pw[20];
    struct clientData client = {0, "", "", 0.0, "", ""};

    printf("\n--- User Login ---\n");
    printf("Account Number: ");
    if (scanf("%u", &acct) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return 0;
    }
    printf("Password: ");
    scanf("%19s", pw);
    clearInputBuffer();

    if (acct < 1 || acct > 100) {
        printf("Invalid account number.\n");
        return 0;
    }

    fseek(cfPtr, (acct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, cfPtr);

    if (isActiveAccount(client)) {
        if (strcmp(pw, client.password) == 0) {
            printf("Login successful! Welcome %s %s.\n", client.firstName, client.lastName);
            *loggedInAcct = acct;
            return 1;
        } else {
            printf("Invalid password.\n");
            return 0;
        }
    } else {
        printf("Account does not exist.\n");
        return 0;
    }
}

void recordTransaction(unsigned int acctNum, const char* type, double amount) {
    FILE *fPtr = fopen("transactions.dat", "ab");
    if (!fPtr) return;
    struct Transaction t;
    t.acctNum = acctNum;
    strncpy(t.type, type, sizeof(t.type)-1);
    t.type[sizeof(t.type)-1] = '\0';
    t.amount = amount;
    getCurrentTime(t.date, sizeof(t.date));
    fwrite(&t, sizeof(struct Transaction), 1, fPtr);
    fclose(fPtr);
}

int isActiveAccount(struct clientData client) {
    return client.acctNum >= 1 && client.acctNum <= 100;
}

// MAIN
int main() {
    FILE *cfPtr;
    createAdmin();
    loadConfig();

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL) {
        if ((cfPtr = fopen("credit.dat", "wb+")) == NULL) {
            printf("Error opening file.\n");
            return 1;
        }
    }

    int choice;
    while(1) {
        printf("\n====== BANK SYSTEM ======\n");
        printf("1. Admin Login\n");
        printf("2. User Login\n");
        printf("3. Exit\n");
        printf("=========================\n");
        printf("Choice: ");
        int scanRes = scanf("%d", &choice);
        if (scanRes == EOF) break;
        if (scanRes != 1) {
            printf("Invalid input.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (choice == 1) {
            if (loginAdmin()) adminMenu(cfPtr);
        } else if (choice == 2) {
            unsigned int acctNum;
            if (loginUser(cfPtr, &acctNum)) userMenu(cfPtr, acctNum);
        } else if (choice == 3) {
            printf("Exiting system...\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
    fclose(cfPtr);
    return 0;
}

void adminMenu(FILE *cfPtr) {
    int choice;
    while(1) {
        printf("\n====== ADMIN BANK SYSTEM ======\n");
        printf("1. View All Accounts\n");
        printf("2. Search Account\n");
        printf("3. Add Account\n");
        printf("4. Delete Account\n");
        printf("5. Account Summary Dashboard\n");
        printf("6. Export to File (accounts.txt)\n");
        printf("7. View Transaction History\n");
        printf("8. View User Credentials\n");
        printf("9. Set Minimum Balance\n");
        printf("10. Logout\n");
        printf("===============================\n");
        printf("Choice: ");
        int scanRes = scanf("%d", &choice);
        if (scanRes == EOF) break;
        if (scanRes != 1) {
            printf("Invalid input.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1: displayAllRecords(cfPtr); break;
            case 2: searchAccount(cfPtr); break;
            case 3: addAccount(cfPtr); break;
            case 4: deleteAccount(cfPtr); break;
            case 5: accountSummary(cfPtr); break;
            case 6: exportToFile(cfPtr); break;
            case 7: viewAllTransactions(); break;
            case 8: viewUserCredentials(cfPtr); break;
            case 9: setMinimumBalance(); break;
            case 10: printf("Logging out...\n"); return;
            default: printf("Invalid choice.\n");
        }
    }
}

void userMenu(FILE *cfPtr, unsigned int acctNum) {
    int choice;
    while(1) {
        printf("\n====== USER PORTAL ======\n");
        printf("1. View Balance & Details\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Transfer Funds\n");
        printf("5. Transaction History\n");
        printf("6. Logout\n");
        printf("=========================\n");
        printf("Choice: ");
        int scanRes = scanf("%d", &choice);
        if (scanRes == EOF) break;
        if (scanRes != 1) {
            printf("Invalid input.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1: viewBalanceAndDetails(cfPtr, acctNum); break;
            case 2: deposit(cfPtr, acctNum); break;
            case 3: withdraw(cfPtr, acctNum); break;
            case 4: transferFunds(cfPtr, acctNum); break;
            case 5: viewUserTransactions(acctNum); break;
            case 6: printf("Logging out...\n"); return;
            default: printf("Invalid choice.\n");
        }
    }
}

// ADMIN FUNCTIONS

void displayAllRecords(FILE *cfPtr) {
    logFile = fopen("accounts.txt", "w");
    struct clientData client = {0, "", "", 0.0, "", ""};
    rewind(cfPtr);
    PRINT("\n%-6s%-11s%-16s%-10s%-22s\n", "Acct", "First Name", "Last Name", "Balance", "Created At");
    PRINT("----------------------------------------------------------------------\n");
    while (fread(&client, sizeof(struct clientData), 1, cfPtr) == 1) {
        if (isActiveAccount(client)) {
            PRINT("%-6u%-11s%-16s%-10.2f%-22s\n", client.acctNum, client.firstName, client.lastName, client.balance, client.createdAt);
        }
    }
    if(logFile) { fclose(logFile); logFile = NULL; }
}

void searchAccount(FILE *cfPtr) {
    char query[20];
    struct clientData client = {0, "", "", 0.0, "", ""};
    int found = 0;

    printf("Enter First/Last name or Account Number to search: ");
    scanf("%19s", query);
    clearInputBuffer();

    logFile = fopen("accounts.txt", "w");
    int isNum = 1;
    for (int i = 0; query[i] != '\0'; i++) {
        if (query[i] < '0' || query[i] > '9') {
            isNum = 0;
            break;
        }
    }
    
    unsigned int acctQuery = 0;
    if (isNum) acctQuery = atoi(query);

    rewind(cfPtr);
    PRINT("\n%-6s%-11s%-16s%-10s\n", "Acct", "First Name", "Last Name", "Balance");
    while (fread(&client, sizeof(struct clientData), 1, cfPtr) == 1) {
        if (isActiveAccount(client)) {
            if ((isNum && client.acctNum == acctQuery) ||
                strcmp(client.firstName, query) == 0 ||
                strcmp(client.lastName, query) == 0) {
                PRINT("%-6u%-11s%-16s%-10.2f\n", client.acctNum, client.firstName, client.lastName, client.balance);
                found = 1;
            }
        }
    }

    if (!found) {
        PRINT("No matching records found.\n");
    }
    if(logFile) { fclose(logFile); logFile = NULL; }
}

void addAccount(FILE *cfPtr) {
    struct clientData client = {0, "", "", 0.0, "", ""};
    unsigned int accountNum;

    printf("Enter new account number ( 1 - 100 ): ");
    if (scanf("%u", &accountNum) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (accountNum < 1 || accountNum > 100) {
        printf("Invalid account number. Please enter a number from 1 to 100.\n");
        return;
    }

    fseek(cfPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, cfPtr);

    if (isActiveAccount(client)) {
        printf("Account #%u already exists.\n", client.acctNum);
    } else {
        printf("Enter First Name: ");
        scanf("%9s", client.firstName);
        clearInputBuffer();
        
        printf("Enter Last Name: ");
        scanf("%14s", client.lastName);
        printf("Enter Initial Deposit (Min $%.2f): ", currentMinBalance);
        if (scanf("%lf", &client.balance) != 1) {
            printf("Invalid amount.\n");
            clearInputBuffer();
            return;
        }
        
        if (client.balance < currentMinBalance) {
            printf("Error: Initial deposit must be at least $%.2f to meet minimum balance requirements.\n", currentMinBalance);
            clearInputBuffer();
            return;
        }
        
        printf("Assign User Password: ");
        scanf("%19s", client.password);
        clearInputBuffer();

        client.acctNum = accountNum;
        getCurrentTime(client.createdAt, sizeof(client.createdAt));

        fseek(cfPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, cfPtr);
        printf("Account #%u successfully created!\n", client.acctNum);
        
        recordTransaction(accountNum, "InitialDeposit", client.balance);
    }
}

void deleteAccount(FILE *cfPtr) {
    struct clientData client = {0, "", "", 0.0, "", ""};
    struct clientData blankClient = {0, "", "", 0.0, "", ""};
    unsigned int accountNum;

    printf("Enter account number to delete ( 1 - 100 ): ");
    if (scanf("%u", &accountNum) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (accountNum < 1 || accountNum > 100) {
        printf("Invalid account number.\n");
        return;
    }

    fseek(cfPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, cfPtr);

    if (!isActiveAccount(client)) {
        printf("Account %u does not exist.\n", accountNum);
    } else {
        char confirm;
        printf("Are you sure you want to delete account %u? (y/n): ", accountNum);
        scanf(" %c", &confirm);
        clearInputBuffer();

        if (confirm == 'y' || confirm == 'Y') {
            fseek(cfPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
            fwrite(&blankClient, sizeof(struct clientData), 1, cfPtr);
            printf("Account deleted successfully.\n");
        } else {
            printf("Delete cancelled.\n");
        }
    }
}

void accountSummary(FILE *cfPtr) {
    logFile = fopen("accounts.txt", "w");
    struct clientData client = {0, "", "", 0.0, "", ""};
    double total = 0.0;
    int count = 0;
    int belowMin = 0;
    rewind(cfPtr);

    while (fread(&client, sizeof(struct clientData), 1, cfPtr) == 1) {
        if (isActiveAccount(client)) {
            total += client.balance;
            count++;
            if (client.balance < currentMinBalance) {
                belowMin++;
            }
        }
    }

    PRINT("\n====== ACCOUNT SUMMARY DASHBOARD ======\n");
    PRINT("Total Active Accounts : %d\n", count);
    PRINT("Total Bank Balance    : $%.2f\n", total);
    if (count > 0) {
        PRINT("Average Balance       : $%.2f\n", total / count);
    }
    PRINT("Accounts < $%.2f    : %d\n", currentMinBalance, belowMin);
    PRINT("=======================================\n");
    if(logFile) { fclose(logFile); logFile = NULL; }
}

void exportToFile(FILE *cfPtr) {
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0, "", ""};
    double total = 0.0;
    int count = 0;

    if ((writePtr = fopen("accounts.txt", "w")) == NULL) {
        printf("File could not be opened.\n");
    } else {
        rewind(cfPtr);
        fprintf(writePtr, "====== BANK SYSTEM EXPORT ======\n");
        fprintf(writePtr, "%-6s%-11s%-16s%-10s%-22s\n", "Acct", "First Name", "Last Name", "Balance", "Created At");
        fprintf(writePtr, "----------------------------------------------------------------------\n");

        while (fread(&client, sizeof(struct clientData), 1, cfPtr) == 1) {
            if (isActiveAccount(client)) {
                fprintf(writePtr, "%-6u%-11s%-16s%-10.2f%-22s\n",
                        client.acctNum, client.firstName, client.lastName, client.balance, client.createdAt);
                total += client.balance;
                count++;
            }
        }

        fprintf(writePtr, "\n====== ACCOUNT SUMMARY ======\n");
        fprintf(writePtr, "Total active accounts: %d\n", count);
        fprintf(writePtr, "Total balance in bank: $%.2f\n", total);
        if (count > 0) {
            fprintf(writePtr, "Average balance: $%.2f\n", total / count);
        }
        fprintf(writePtr, "=============================\n");
        
        fclose(writePtr);
        printf("Data successfully exported to accounts.txt\n");
    }
}

void viewAllTransactions() {
    logFile = fopen("accounts.txt", "w");
    FILE *fPtr = fopen("transactions.dat", "rb");
    if (!fPtr) {
        PRINT("No transaction history available.\n");
        if(logFile) { fclose(logFile); logFile = NULL; }
        return;
    }

    struct Transaction t;
    PRINT("\n====== GLOBAL TRANSACTION HISTORY ======\n");
    PRINT("%-6s %-15s %-10s %-22s\n", "Acct", "Type", "Amount", "Date");
    PRINT("-------------------------------------------------------\n");
    while (fread(&t, sizeof(struct Transaction), 1, fPtr) == 1) {
        PRINT("%-6u %-15s $%-9.2f %-22s\n", t.acctNum, t.type, t.amount, t.date);
    }
    fclose(fPtr);
    if(logFile) { fclose(logFile); logFile = NULL; }
}

void viewUserCredentials(FILE *cfPtr) {
    logFile = fopen("accounts.txt", "w");
    struct clientData client = {0, "", "", 0.0, "", ""};
    rewind(cfPtr);
    PRINT("\n====== USER CREDENTIALS ======\n");
    PRINT("%-10s%-11s%-16s%-15s\n", "User ID", "First Name", "Last Name", "Password");
    PRINT("------------------------------------------------------\n");
    while (fread(&client, sizeof(struct clientData), 1, cfPtr) == 1) {
        if (isActiveAccount(client)) {
            PRINT("%-10u%-11s%-16s%-15s\n", client.acctNum, client.firstName, client.lastName, client.password);
        }
    }
    if(logFile) { fclose(logFile); logFile = NULL; }
}

void loadConfig() {
    FILE *f = fopen("config.txt", "r");
    if(f) {
        fscanf(f, "%lf", &currentMinBalance);
        fclose(f);
    } else {
        saveConfig();
    }
}

void saveConfig() {
    FILE *f = fopen("config.txt", "w");
    if(f) {
        fprintf(f, "%.2f", currentMinBalance);
        fclose(f);
    }
}

void setMinimumBalance() {
    printf("\n====== SET MINIMUM BALANCE ======\n");
    printf("Current minimum balance: $%.2f\n", currentMinBalance);
    printf("Enter new minimum balance: $");
    double newBal;
    if(scanf("%lf", &newBal) == 1 && newBal >= 0) {
        currentMinBalance = newBal;
        saveConfig();
        printf("Minimum balance successfully updated to $%.2f\n", currentMinBalance);
    } else {
        printf("Error: Invalid balance amount.\n");
    }
    clearInputBuffer();
}

// USER FUNCTIONS

void viewBalanceAndDetails(FILE *cfPtr, unsigned int acctNum) {
    logFile = fopen("accounts.txt", "w");
    struct clientData client = {0, "", "", 0.0, "", ""};
    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, cfPtr);

    PRINT("\n====== ACCOUNT DETAILS ======\n");
    PRINT("Account Number : %u\n", client.acctNum);
    PRINT("Name           : %s %s\n", client.firstName, client.lastName);
    PRINT("Balance        : $%.2f\n", client.balance);
    PRINT("Created At     : %s\n", client.createdAt);
    PRINT("=============================\n");
    if(logFile) { fclose(logFile); logFile = NULL; }
}

void deposit(FILE *cfPtr, unsigned int acctNum) {
    struct clientData client = {0, "", "", 0.0, "", ""};
    double amount;

    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, cfPtr);

    printf("Enter amount to deposit: $");
    if (scanf("%lf", &amount) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (amount <= 0) {
        printf("Error: Deposit amount must be positive.\n");
        return;
    }

    client.balance += amount;
    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, cfPtr);

    printf("Successfully deposited $%.2f. New Balance: $%.2f\n", amount, client.balance);
    recordTransaction(acctNum, "Deposit", amount);
}

void withdraw(FILE *cfPtr, unsigned int acctNum) {
    struct clientData client = {0, "", "", 0.0, "", ""};
    double amount;

    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, cfPtr);

    printf("Enter amount to withdraw: $");
    if (scanf("%lf", &amount) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (amount <= 0) {
        printf("Error: Withdrawal amount must be positive.\n");
        return;
    }

    if (client.balance - amount < currentMinBalance) {
        printf("Error: Transaction failed. Minimum balance of $%.2f must be maintained.\n", currentMinBalance);
        return;
    }

    client.balance -= amount;
    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, cfPtr);

    printf("Successfully withdrew $%.2f. New Balance: $%.2f\n", amount, client.balance);
    recordTransaction(acctNum, "Withdraw", amount);
}

void transferFunds(FILE *cfPtr, unsigned int acctNum) {
    struct clientData sender = {0, "", "", 0.0, "", ""};
    struct clientData receiver = {0, "", "", 0.0, "", ""};
    unsigned int destAcct;
    double amount;

    printf("Enter destination account number: ");
    if (scanf("%u", &destAcct) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (destAcct < 1 || destAcct > 100 || destAcct == acctNum) {
        printf("Invalid destination account number.\n");
        return;
    }

    fseek(cfPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&receiver, sizeof(struct clientData), 1, cfPtr);

    if (!isActiveAccount(receiver)) {
        printf("Error: Destination account does not exist.\n");
        return;
    }

    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&sender, sizeof(struct clientData), 1, cfPtr);

    printf("Enter amount to transfer: $");
    if (scanf("%lf", &amount) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (amount <= 0) {
        printf("Error: Transfer amount must be positive.\n");
        return;
    }

    if (sender.balance - amount < currentMinBalance) {
        printf("Error: Transaction failed. Minimum balance of $%.2f must be maintained.\n", currentMinBalance);
        return;
    }

    // Deduct from sender
    sender.balance -= amount;
    fseek(cfPtr, (acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&sender, sizeof(struct clientData), 1, cfPtr);
    recordTransaction(acctNum, "TransferOut", amount);

    // Add to receiver
    receiver.balance += amount;
    fseek(cfPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&receiver, sizeof(struct clientData), 1, cfPtr);
    recordTransaction(destAcct, "TransferIn", amount);

    printf("Successfully transferred $%.2f to Account #%u. New Balance: $%.2f\n", amount, destAcct, sender.balance);
}

void viewUserTransactions(unsigned int acctNum) {
    logFile = fopen("accounts.txt", "w");
    FILE *fPtr = fopen("transactions.dat", "rb");
    if (!fPtr) {
        PRINT("No transaction history available.\n");
        if(logFile) { fclose(logFile); logFile = NULL; }
        return;
    }

    struct Transaction t;
    int found = 0;
    PRINT("\n====== TRANSACTION HISTORY ======\n");
    PRINT("%-15s %-10s %-22s\n", "Type", "Amount", "Date");
    PRINT("------------------------------------------------\n");
    while (fread(&t, sizeof(struct Transaction), 1, fPtr) == 1) {
        if (t.acctNum == acctNum) {
            PRINT("%-15s $%-9.2f %-22s\n", t.type, t.amount, t.date);
            found = 1;
        }
    }
    if (!found) {
        PRINT("No transactions found for this account.\n");
    }
    fclose(fPtr);
    if(logFile) { fclose(logFile); logFile = NULL; }
}
