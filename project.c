#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define ADMIN_USER "diusecurebank"
#define ADMIN_PASS "newbank@#"

// ==========================================
// 1. DATA STRUCTURES
// ==========================================

struct account {
    long long accNo;
    char name[50];
    char username[30];
    char password[30];
    char nid[12];
    char phone[15];
    int pin;
    float balance;
    float loan;
    int isLocked; // SECURITY FLAG: 0 = Active, 1 = Locked
    struct account *next;
};
struct account *head = NULL;

struct loanRequest {
    long long accNo;
    float amount;
    struct loanRequest *next;
};
struct loanRequest *front = NULL, *rear = NULL;

struct transaction {
    char type[50], date[50];
    float amt;
    struct transaction *next;
};
struct transaction *top = NULL;


// ==========================================
// 2. UTILITY FUNCTIONS
// ==========================================

void color(int c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void waitForEnter() {
    color(10);
    printf("\n[Press ENTER key to continue...]\n");
    while (1) {
        int ch = _getch();
        if (ch == 13) break; // 13 is ASCII for Enter (\r)
    }
}

void header() {
    system("cls");
    color(10);
    printf("====================================================\n");
    printf(" ||        DIU SECURE STUDENT BANK SYSTEM        || \n");
    printf("====================================================\n\n");
}


// ==========================================
// 3. CORE MEMORY & QUEUE/STACK OPS (O(1) & O(N))
// ==========================================

struct account* search(long long id) {
    struct account *temp = head;
    while (temp) {
        if (temp->accNo == id) return temp;
        temp = temp->next;
    }
    return NULL;
}

void enqueueLoan(long long id, float amt) {
    struct loanRequest *newNode = (struct loanRequest*)malloc(sizeof(struct loanRequest));
    if (!newNode) return;
    newNode->accNo = id;
    newNode->amount = amt;
    newNode->next = NULL;

    if (rear == NULL) { front = rear = newNode; }
    else { rear->next = newNode; rear = newNode; }
}

void pushTransaction(char type[], float amt, char date[]) {
    struct transaction *newNode = (struct transaction*)malloc(sizeof(struct transaction));
    if (!newNode) return;
    strcpy(newNode->type, type);
    newNode->amt = amt;
    strcpy(newNode->date, date);
    newNode->next = top;
    top = newNode;
}

void clearStack() {
    while (top) {
        struct transaction *temp = top;
        top = top->next;
        free(temp);
    }
}


// ==========================================
// 4. VALIDATION & SECURITY PROTOCOLS
// ==========================================

int usernameExists(char u[]) {
    struct account *temp = head;
    while (temp) { if (strcmp(temp->username, u) == 0) return 1; temp = temp->next; }
    return 0;
}

int nidExists(char nid[]) {
    struct account *temp = head;
    while (temp) { if (strcmp(temp->nid, nid) == 0) return 1; temp = temp->next; }
    return 0;
}

int phoneExists(char phone[]) {
    struct account *temp = head;
    while (temp) { if (strcmp(temp->phone, phone) == 0) return 1; temp = temp->next; }
    return 0;
}

long long generateAccount() {
    long long id;
    do {
        id = 1000000000000LL + (long long)(rand() % 900000000);
        id = id * 10000 + rand() % 10000;
    } while (search(id) != NULL);
    return id;
}

int validNID(char nid[]) {
    if (strlen(nid) != 10) return 0;
    for (int i = 0; i < 10; i++) { if (nid[i] < '0' || nid[i] > '9') return 0; }
    return 1;
}

int validPhone(char ph[]) {
    if (strlen(ph) != 14 || strncmp(ph, "+880", 4) != 0) return 0;
    for (int i = 4; i < 14; i++) { if (ph[i] < '0' || ph[i] > '9') return 0; }
    return 1;
}

// SECURE FILE SAVING
void save() {
    FILE *fp = fopen("bank.txt", "w");
    if (!fp) return;
    struct account *temp = head;
    while (temp) {
        fprintf(fp, "%lld|%s|%s|%s|%s|%s|%d|%.2f|%.2f|%d\n",
        temp->accNo, temp->name, temp->username, temp->password, temp->nid,
         temp->phone, temp->pin, temp->balance, temp->loan, temp->isLocked);
        temp = temp->next;
    }
    fclose(fp);
}

// BACKWARD COMPATIBLE FILE LOADER
void load() {
    FILE *fp = fopen("bank.txt", "r");
    if (!fp) return;
    while (1) {
        struct account *newNode = (struct account*)malloc(sizeof(struct account));
        if (!newNode) break;
        newNode->isLocked = 0; // Default if old file format

        int args = fscanf(fp, "%lld|%49[^|]|%29[^|]|%29[^|]|%11[^|]|%14[^|]|%d|%f|%f|%d\n",
        &newNode->accNo, newNode->name, newNode->username, newNode->password, newNode->nid,
         newNode->phone, &newNode->pin, &newNode->balance, &newNode->loan, &newNode->isLocked);

        if (args >= 9) {
            newNode->next = head;
            head = newNode;
        } else {
            free(newNode);
            break;
        }
    }
    fclose(fp);
}

void logTransaction(long long accNo, const char* type, float amount) {
    FILE *fp = fopen("transactions.txt", "a");
    if (fp) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(fp, "%lld|%s|%.2f|%02d-%02d-%d %02d:%02d:%02d\n",
                accNo, type, amount, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fclose(fp);
    }
}

// SECURE PIN VERIFICATION (MAX 5 ATTEMPTS)
int verifyPin(struct account *acc) {
    if (acc->isLocked) return 0;

    int enteredPin, attempts = 0;
    while (attempts < 5) {
        printf("[+] Enter 4-Digit PIN to authenticate: ");
        if (scanf("%d", &enteredPin) != 1) {
            clearInputBuffer();
        } else if (enteredPin == acc->pin) {
            return 1;
        }
        attempts++;
        color(12); printf("[ERROR] Incorrect PIN! Attempts remaining: %d\n", 5 - attempts); color(10);
    }
    color(12); printf("\n[SECURITY ALERT] Maximum PIN attempts reached! Account LOCKED.\n"); color(10);
    acc->isLocked = 1;
    save();
    return 0;
}


// ==========================================
// 5. CORE BANKING FUNCTIONS (USER)
// ==========================================

void createAccount() {
    header();
    struct account *newNode = (struct account*)malloc(sizeof(struct account));
    if (!newNode) return;

    char passConfirm[30];
    int pinConfirm, otpGenerated, otpEntered, otpAttempts = 0;
    float initialDeposit;

    printf(">> Initializing Secure Registration Protocol...\n");
    printf("[+] Full Name: ");
    scanf(" %[^\n]", newNode->name);

    while (1) {
        printf("[+] NID (10 digit): "); scanf("%11s", newNode->nid);
        if (!validNID(newNode->nid)) { color(12); printf("[ERROR] Invalid NID! Must be exactly 10 digits.\n"); color(10); }
        else if (nidExists(newNode->nid)) { color(12); printf("[ERROR] An account with this NID already exists!\n"); color(10); }
        else break;
    }

    while (1) {
        printf("[+] Phone (+880XXXXXXXXXX): "); scanf("%14s", newNode->phone);
        if (!validPhone(newNode->phone)) { color(12); printf("[ERROR] Invalid Phone Format!\n"); color(10); }
        else if (phoneExists(newNode->phone)) { color(12); printf("[ERROR] Phone Number already exists!\n"); color(10); }
        else break;
    }

    otpGenerated = 100000 + rand() % 900000;
    printf("\n[SYSTEM] Sending Encrypted OTP to %s...\n", newNode->phone);
    Sleep(1000);
    color(14); printf("[SIMULATION] YOUR OTP CODE IS: %d\n", otpGenerated); color(10);

    // MAX 5 OTP ATTEMPTS
    while (otpAttempts < 5) {
        printf("[+] Enter 6-Digit OTP to Verify: ");
        if (scanf("%d", &otpEntered) != 1) clearInputBuffer();
        if (otpEntered == otpGenerated) {
            printf("[SUCCESS] Phone Number Verified!\n\n");
            break;
        } else {
            otpAttempts++;
            color(12); printf("[ERROR] Wrong OTP! Attempts left: %d\n", 5 - otpAttempts); color(10);
        }
    }
    if (otpAttempts >= 5) {
        color(12); printf("\n[SECURITY ALERT] Registration aborted due to multiple failed OTPs.\n"); color(10);
        free(newNode); waitForEnter(); return;
    }

    while (1) {
        printf("[+] Choose Username: "); scanf("%29s", newNode->username);
        if (usernameExists(newNode->username)) { color(12); printf("[ERROR] Username '%s' is taken!\n", newNode->username); color(10); }
        else break;
    }

    while (1) {
        printf("[+] Set Password: "); scanf("%29s", newNode->password);
        printf("[+] Confirm Password: "); scanf("%29s", passConfirm);
        if (strcmp(newNode->password, passConfirm) == 0) break;
        color(12); printf("[ERROR] Passwords do not match!\n"); color(10);
    }

    while (1) {
        printf("[+] Set 4-Digit PIN: ");
        if (scanf("%d", &newNode->pin) != 1) clearInputBuffer();
        printf("[+] Confirm PIN: ");
        if (scanf("%d", &pinConfirm) != 1) clearInputBuffer();
        if (newNode->pin == pinConfirm && newNode->pin >= 1000 && newNode->pin <= 9999) break;
        color(12); printf("[ERROR] Invalid PIN! Must be 4 digits.\n"); color(10);
    }

    printf("\n----------------------------------------------------\n");
    color(14); printf("[NOTICE] System Policy: Min deposit of 300 BDT required.\n"); color(10);
    while (1) {
        printf("[+] Initial Deposit Amount: ");
        if (scanf("%f", &initialDeposit) != 1) { clearInputBuffer(); continue; }
        if (initialDeposit >= 300.0) {
            newNode->balance = initialDeposit;
            printf("[SUCCESS] Deposit of %.2f BDT accepted!\n", initialDeposit);
            break;
        } else { color(12); printf("[ERROR] Deposit must be at least 300 BDT!\n"); color(10); }
    }

    newNode->loan = 0;
    newNode->isLocked = 0;
    newNode->accNo = generateAccount();
    newNode->next = head;
    head = newNode;

    printf("\n[SYSTEM] Account Successfully Encrypted & Saved!\n");
    printf("[SYSTEM] Your New Account No: %lld\n", newNode->accNo);
    logTransaction(newNode->accNo, "Account Opening", initialDeposit);
    save();
    waitForEnter();
}

void editAccount(struct account *acc) {
    int ch;
    while (1) {
        header();
        printf("--- ACCOUNT DETAILS ---\n");
        printf(" Name     : %s\n Username : %s\n NID No   : %s\n Phone    : %s\n", acc->name, acc->username, acc->nid, acc->phone);
        printf("----------------------------------------------------\n");
        printf(" 1. Update Phone Number\n 2. Update Password\n 3. Update 4-Digit PIN\n 0. Back to Main Menu\n\nCommand >> ");

        if (scanf("%d", &ch) != 1) { clearInputBuffer(); continue; }
        if (ch == 0) break;

        if (ch == 1) {
            char tempPhone[15];
            while (1) {
                printf("\n[+] Enter New Phone (+880...): "); scanf("%14s", tempPhone);
                if (!validPhone(tempPhone)) { color(12); printf("[ERROR] Invalid Phone Format!\n"); color(10); }
                else if (phoneExists(tempPhone)) { color(12); printf("[ERROR] Phone number already exists!\n"); color(10); }
                else {
                    strcpy(acc->phone, tempPhone);
                    printf("[SUCCESS] Phone Number Updated!\n"); save(); break;
                }
            }
            waitForEnter();
        }
        else if (ch == 2) {
            char oldPass[30], newPass[30], confirmPass[30];
            int passAttempts = 0;
            while(passAttempts < 5) {
                printf("\n[+] Enter Current Password: "); scanf("%29s", oldPass);
                if (strcmp(oldPass, acc->password) == 0) {
                    while (1) {
                        printf("[+] Enter New Password: "); scanf("%29s", newPass);
                        printf("[+] Confirm New Password: "); scanf("%29s", confirmPass);
                        if (strcmp(newPass, confirmPass) == 0) {
                            strcpy(acc->password, newPass);
                            printf("[SUCCESS] Password Updated Successfully!\n"); save(); goto edit_end;
                        } else { color(12); printf("[ERROR] Passwords do not match!\n"); color(10); }
                    }
                }
                passAttempts++;
                color(12); printf("[ERROR] Incorrect Password! Attempts left: %d\n", 5 - passAttempts); color(10);
            }
            color(12); printf("\n[SECURITY ALERT] Account LOCKED due to multiple failed attempts!\n"); color(10);
            acc->isLocked = 1; save(); return; // Lockout
            edit_end: waitForEnter();
        }
        else if (ch == 3) {
            printf("\n");
            if (verifyPin(acc)) {
                int newPin, confirmPin;
                while (1) {
                    printf("[+] Enter New 4-Digit PIN: "); if (scanf("%d", &newPin) != 1) { clearInputBuffer(); continue; }
                    printf("[+] Confirm New PIN: "); if (scanf("%d", &confirmPin) != 1) { clearInputBuffer(); continue; }

                    if (newPin == confirmPin && newPin >= 1000 && newPin <= 9999) {
                        acc->pin = newPin;
                        printf("[SUCCESS] PIN Updated Successfully!\n"); save(); break;
                    } else { color(12); printf("[ERROR] Invalid PIN or mismatch!\n"); color(10); }
                }
            }
            waitForEnter();
            if(acc->isLocked) return; // Kick out if blocked
        }
    }
}

void checkBalance(struct account *acc) {
    header();
    printf("--- FINANCIAL STATUS ---\nBalance: %.2f BDT\nLoan: %.2f BDT\n", acc->balance, acc->loan);
}

void deposit(struct account *acc) {
    float amt;
    printf("[+] Amount: "); scanf("%f", &amt);
    if (amt > 0) {
        acc->balance += amt; logTransaction(acc->accNo, "Deposit", amt);
        printf("Deposit Successful!\n");
    } else { color(12); printf("[ERROR] Invalid Amount!\n"); color(10); }
}

void withdraw(struct account *acc) {
    float amt;
    printf("[+] Amount to withdraw: "); scanf("%f", &amt);
    if (amt > 0 && amt <= acc->balance) {
        if (!verifyPin(acc)) return;
        acc->balance -= amt; logTransaction(acc->accNo, "Withdraw", amt);
        printf("Withdrawal Successful!\n");
    } else { color(12); printf("Insufficient Balance or Invalid Amount!\n"); color(10); }
}

void transfer(struct account *acc) {
    long long id; float amt;
    printf("[+] Receiver Acc No: "); scanf("%lld", &id);
    struct account *receiver = search(id);

    if (receiver && receiver->accNo != acc->accNo && !receiver->isLocked) {
        printf("[+] Amount to transfer: "); scanf("%f", &amt);
        if (amt > 0 && amt <= acc->balance) {
            if (!verifyPin(acc)) return;
            acc->balance -= amt; receiver->balance += amt;
            logTransaction(acc->accNo, "Transfer Out", amt); logTransaction(receiver->accNo, "Transfer In", amt);
            printf("Fund Transfer Successful!\n");
        } else { color(12); printf("Insufficient Balance!\n"); color(10); }
    } else { color(12); printf("Account Not Found, Invalid or Locked!\n"); color(10); }
}

void externalTransfer(struct account *acc) {
    char bank[50]; long long extAccNo; float amt, fee = 10.00;
    printf("[+] Target Bank Name: "); scanf(" %[^\n]", bank);
    printf("[+] Receiver Account No: "); scanf("%lld", &extAccNo);
    printf("[+] Amount to transfer: "); scanf("%f", &amt);

    if (amt > 0 && (amt + fee) <= acc->balance) {
        color(14); printf("\n[NOTICE] An inter-bank routing fee of %.2f BDT will apply.\n", fee); color(10);
        if (!verifyPin(acc)) return;
        acc->balance -= (amt + fee);
        logTransaction(acc->accNo, "External Transfer", amt); logTransaction(acc->accNo, "Routing Fee", fee);
        printf("\n[SUCCESS] Successfully Routed %.2f BDT to A/C: %lld (%s)!\n", amt, extAccNo, bank);
        printf("Total Deducted: %.2f BDT (Including %.2f BDT Fee)\n", (amt + fee), fee);
    } else { color(12); printf("[ERROR] Invalid Amount or Insufficient Balance!\n"); color(10); }
}

void loanRequest(struct account *acc) {
    header();
    if (acc->balance < 10000) { color(12); printf("Minimum 10,000 BDT balance required for Loan!\n"); color(10); return; }
    float maxEligible = acc->balance * 0.25;
    printf("You are eligible for max loan of: %.2f BDT\n[+] Enter amount to request: ", maxEligible);
    float amt; scanf("%f", &amt);
    if (amt > 0 && amt <= maxEligible) {
        enqueueLoan(acc->accNo, amt);
        printf("\n[SYSTEM] Loan Request Submitted Successfully! Please wait for Admin approval.\n");
    } else { color(12); printf("Requested amount exceeds limit or is invalid!\n"); color(10); }
}

void repayLoan(struct account *acc) {
    header();
    printf("--- LOAN REPAYMENT ---\n");
    if (acc->loan <= 0) { color(14); printf("[INFO] You do not have any outstanding loan to repay.\n"); color(10); return; }

    printf("Outstanding Loan: %.2f BDT\nCurrent Balance : %.2f BDT\n\n[+] Enter amount to repay: ", acc->loan, acc->balance);
    float amt; if (scanf("%f", &amt) != 1) { clearInputBuffer(); return; }

    if (amt <= 0 || amt > acc->loan || amt > acc->balance) { color(12); printf("[ERROR] Invalid Amount / Insufficient balance!\n"); color(10); }
    else {
        if (!verifyPin(acc)) return;
        acc->balance -= amt; acc->loan -= amt;
        logTransaction(acc->accNo, "Loan Repayment", amt);
        printf("\n[SUCCESS] Successfully repaid %.2f BDT!\n", amt);
    }
}

void showHistory(long long id) {
    clearStack();
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history found.\n"); return; }

    char line[256]; int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        long long acc; char type[50], date[50]; float amt;
        if (sscanf(line, "%lld|%49[^|]|%f|%49[^\n]", &acc, type, &amt, date) == 4) {
            if (acc == id) { pushTransaction(type, amt, date); found = 1; }
        }
    }
    fclose(fp);

    header(); printf("--- RECENT TRANSACTION HISTORY (LIFO) ---\n");
    if (!found) { printf("No records found for your account.\n"); }
    else {
        struct transaction *temp = top;
        while (temp) { printf("[%s] %-15s: %.2f BDT\n", temp->date, temp->type, temp->amt); temp = temp->next; }
    }
}

void userMenu(struct account *acc) {
    int ch;
    while (1) {
        if (acc->isLocked) {
            color(12); printf("\n[SECURITY ALERT] Account LOCKED. System force logging out...\n"); color(10);
            waitForEnter(); return;
        }
        header();
        printf("Welcome: %s | Account ID: %lld\n----------------------------------------------------\n", acc->name, acc->accNo);
        printf(" 1. Edit Details     2. Check Balance   3. Deposit\n 4. Withdraw         5. DIU Transfer    6. Bank Transfer\n 7. Request Loan     8. Repay Loan      9. History\n 0. Logout\n\nCommand >> ");

        if (scanf("%d", &ch) != 1) { clearInputBuffer(); continue; }
        if (ch == 0) return;

        switch (ch) {
            case 1: editAccount(acc); break;
            case 2: checkBalance(acc); break;
            case 3: deposit(acc); break;
            case 4: withdraw(acc); break;
            case 5: transfer(acc); break;
            case 6: externalTransfer(acc); break;
            case 7: loanRequest(acc); break;
            case 8: repayLoan(acc); break;
            case 9: showHistory(acc->accNo); break;
            default: color(12); printf("Invalid Command!\n"); color(10);
        }
        save(); waitForEnter();
    }
}


// ==========================================
// 6. LOGIN & RECOVERY PROCEDURES
// ==========================================

void userLogin() {
    header();
    long long id;
    char pass[30];
    int otpGenerated, otpEntered, passAttempts = 0, otpAttempts = 0;

    printf("[+] Account No: "); if (scanf("%lld", &id) != 1) { clearInputBuffer(); return; }
    struct account *acc = search(id);

    if (!acc) { color(12); printf("\n[ERROR] Account not found!\n"); color(10); waitForEnter(); return; }
    if (acc->isLocked) { color(12); printf("\n[SECURITY ALERT] Account is LOCKED! Contact Administration.\n"); color(10); waitForEnter(); return; }

    while (passAttempts < 5) {
        printf("[+] Password: "); scanf("%29s", pass);

        if (strcmp(acc->password, pass) == 0) {
            otpGenerated = 100000 + rand() % 900000;
            printf("\n[SYSTEM] Initiating 2-Step Verification...\n[SYSTEM] Sending OTP to %s...\n", acc->phone);
            Sleep(1000);
            color(14); printf("[SIMULATION] YOUR LOGIN OTP IS: %d\n", otpGenerated); color(10);

            while (otpAttempts < 5) {
                printf("[+] Enter 6-Digit OTP to Login: ");
                if (scanf("%d", &otpEntered) != 1) clearInputBuffer();
                if (otpEntered == otpGenerated) {
                    printf("[SUCCESS] Identity Verified! Logging in...\n");
                    Sleep(500); userMenu(acc); return;
                } else {
                    otpAttempts++;
                    color(12); printf("[ERROR] Wrong OTP! Attempts left: %d\n", 5 - otpAttempts); color(10);
                }
            }
            color(12); printf("\n[SECURITY ALERT] Maximum OTP attempts reached! Account LOCKED.\n"); color(10);
            acc->isLocked = 1; save(); waitForEnter(); return;
        }
        passAttempts++;
        color(12); printf("[ERROR] Wrong Password! Attempts left: %d\n", 5 - passAttempts); color(10);
    }

    color(12); printf("\n[SECURITY ALERT] Account LOCKED due to 5 failed password attempts!\n"); color(10);
    acc->isLocked = 1; save(); waitForEnter();
}

void forgotPassword() {
    header();
    long long id; int otpGenerated, otpEntered, otpAttempts = 0;
    printf(">> ACCOUNT RECOVERY SYSTEM\n[+] Enter your Account No: ");
    if (scanf("%lld", &id) != 1) { clearInputBuffer(); return; }

    struct account *acc = search(id);
    if (acc) {
        if(acc->isLocked) { color(12); printf("\n[ERROR] Account is LOCKED! Recovery disabled. Contact Admin.\n"); color(10); waitForEnter();
        return; }

        printf("\n[SYSTEM] Account Found!\n[+] Account Name: %s\n", acc->name);
        char phoneInput[15];
        printf("[+] Enter Registered Phone Number (+880...): "); scanf("%14s", phoneInput);

        if (strcmp(acc->phone, phoneInput) == 0) {
            otpGenerated = 100000 + rand() % 900000;
            printf("\n[SYSTEM] Sending Verification OTP to %s...\n", acc->phone);
            Sleep(1000); color(14); printf("[SIMULATION] YOUR RECOVERY OTP IS: %d\n", otpGenerated); color(10);

            while (otpAttempts < 5) {
                printf("[+] Enter 6-Digit OTP to Verify: ");
                if (scanf("%d", &otpEntered) != 1) clearInputBuffer();
                if (otpEntered == otpGenerated) {
                    printf("[SUCCESS] Identity Verified!\n\n");
                    char newPass[30], confirmPass[30];
                    while (1) {
                        printf("[+] Enter New Password: "); scanf("%29s", newPass);
                        printf("[+] Confirm New Password: "); scanf("%29s", confirmPass);
                        if (strcmp(newPass, confirmPass) == 0) {
                            strcpy(acc->password, newPass); printf("\n[SUCCESS] Password Reset Successfully!\n");
                            save(); goto recovery_end;
                        } else { color(12); printf("[ERROR] Passwords do not match!\n\n"); color(10); }
                    }
                } else { otpAttempts++; color(12); printf("[ERROR] Wrong OTP! Attempts left: %d\n", 5 - otpAttempts); color(10); }
            }
            color(12); printf("\n[SECURITY ALERT] Maximum OTP attempts reached! Account LOCKED.\n"); color(10);
            acc->isLocked = 1; save();
        } else { color(12); printf("\n[ERROR] Phone Number does not match!\n"); color(10); }
    } else { color(12); printf("\n[ERROR] Account not found!\n"); color(10); }
    recovery_end: waitForEnter();
}


// ==========================================
// 7. ADMIN FUNCTIONS
// ==========================================

void adminMenu() {
    int ch;
    while (1) {
        header();
        printf(">> ROOT PRIVILEGES GRANTED\n");
        printf("1. View All Database Accounts\n2. Manage Pending Loan Requests (Queue)\n3. View Total Bank Vault\n4. Process Monthly Loan EMI (8.33%% Auto)\n5. Delete an Account\n6. Unlock Blocked Account\n0. Logout\n\nCommand >> ");
        if (scanf("%d", &ch) != 1) { clearInputBuffer(); continue; }
        if (ch == 0) return;

        if (ch == 1) {
            header(); struct account *temp = head;
            printf("%-15s | %-20s | %-10s | %-10s | %-8s\n", "Account No", "Name", "Balance", "Loan", "Status");
            printf("---------------------------------------------------------------------------\n");
            while (temp) {
                printf("%-15lld | %-20s | %-10.2f | %-10.2f | %-8s\n", temp->accNo, temp->name, temp->balance, temp->loan, temp->isLocked ? "LOCKED" : "ACTIVE");
                temp = temp->next;
            }
            waitForEnter();
        }
        else if (ch == 2) {
            header(); printf("--- PENDING LOAN REQUESTS (FIFO) ---\n");
            if (front == NULL) { printf("No pending loan requests in the queue.\n"); }
            else {
                struct loanRequest *req = front; struct account *acc = search(req->accNo);
                if (acc && !acc->isLocked) {
                    printf("Next Request -> Account: %lld | Name: %s | Amount: %.2f BDT\n\nAction: [1] Approve  [2] Reject  [0] Skip\nCommand >> ", req->accNo, acc->name, req->amount);
                    int action; scanf("%d", &action);
                    if (action == 1) {
                        acc->balance += req->amount; acc->loan += req->amount;
                        logTransaction(acc->accNo, "Loan Approved", req->amount); save();
                        printf("\n[SUCCESS] Loan Approved & Credited!\n");
                    } else if (action == 2) { printf("\n[SYSTEM] Loan Request Rejected.\n"); }

                    if (action == 1 || action == 2) {
                        front = front->next; if (front == NULL) rear = NULL; free(req);
                    }
                } else {
                    front = front->next; if (front == NULL) rear = NULL; free(req);
                    printf("\n[SYSTEM] Invalid/Locked Account request automatically skipped.\n");
                }
            }
            waitForEnter();
        }
        else if (ch == 3) {
            header(); struct account *temp = head; float totalVault = 0.0, totalLoans = 0.0;
            while (temp) { totalVault += temp->balance; totalLoans += temp->loan; temp = temp->next; }
            printf("--- TOTAL BANK FINANCIALS ---\nTotal Vault Balance : %.2f BDT\nTotal Issued Loans  : %.2f BDT\n", totalVault, totalLoans);
            waitForEnter();
        }
        else if (ch == 4) {
            header(); printf("--- MONTHLY LOAN DEDUCTION (8.33%%) ---\n");
            struct account *temp = head; int count = 0;
            while (temp) {
                if (temp->loan > 0) {
                    float emi = temp->loan * 0.0833;
                    if (emi > temp->balance) emi = temp->balance;
                    if (emi > 0) {
                        temp->balance -= emi; temp->loan -= emi;
                        logTransaction(temp->accNo, "Monthly EMI Auto-Deduct", emi);
                        printf("Account: %lld | Deducted: %.2f BDT | Remaining Loan: %.2f BDT\n", temp->accNo, emi, temp->loan); count++;
                    } else { printf("Account: %lld | EMI Failed (0 Balance available)\n", temp->accNo); }
                }
                temp = temp->next;
            }
            if (count > 0) save(); else printf("No active loans eligible for EMI deduction.\n");
            printf("\n[SUCCESS] Monthly EMI cycle processed.\n"); waitForEnter();
        }
        else if (ch == 5) {
            header(); long long delId; char pass[30]; int passAttempts = 0;
            printf("--- DELETE ACCOUNT ---\nEnter Account No to Delete: ");
            if (scanf("%lld", &delId) != 1) { clearInputBuffer(); continue; }
            struct account *target = search(delId);

            if (!target) { color(12); printf("\n[ERROR] Account not found!\n"); color(10); }
            else {
                printf("\nTarget Account Found: %s\nCurrent Balance: %.2f BDT\n", target->name, target->balance);
                while (passAttempts < 5) {
                    printf("\n[+] Enter Admin Password to permanently delete: "); scanf("%29s", pass);
                    if (strcmp(pass, ADMIN_PASS) == 0) {
                        struct account *curr = head, *prev = NULL;
                        while (curr != NULL && curr->accNo != delId) { prev = curr; curr = curr->next; }
                        if (curr != NULL) {
                            if (prev == NULL) head = curr->next; else prev->next = curr->next;
                            free(curr); save();
                            color(12); printf("\n[SUCCESS] Account Deleted Permanently!\n"); color(10);
                        }
                        goto del_end;
                    }
                    passAttempts++; color(12); printf("[ERROR] Authentication Failed! Attempts left: %d\n", 5 - passAttempts); color(10);
                }
                color(12); printf("\n[SECURITY ALERT] Maximum attempts reached. Deletion aborted.\n"); color(10);
            }
            del_end: waitForEnter();
        }
        else if (ch == 6) { // NEW FEATURE: UNLOCK ACCOUNT
            header(); long long unlockId;
            printf("--- UNLOCK ACCOUNT ---\nEnter Account No to Unlock: ");
            if (scanf("%lld", &unlockId) == 1) {
                struct account *target = search(unlockId);
                if(target) {
                    if(target->isLocked) {
                        target->isLocked = 0; save();
                        printf("\n[SUCCESS] Account %lld successfully UNLOCKED.\n", unlockId);
                    } else { printf("\n[INFO] Account is already ACTIVE.\n"); }
                } else { color(12); printf("\n[ERROR] Account not found!\n"); color(10); }
            } else { clearInputBuffer(); }
            waitForEnter();
        }
    }
}

void adminLogin() {
    char u[30], p[30]; int attempts = 0;
    header();
    while(attempts < 5) {
        printf("Admin User: "); scanf("%29s", u);
        printf("Admin Pass: "); scanf("%29s", p);
        if (strcmp(u, ADMIN_USER) == 0 && strcmp(p, ADMIN_PASS) == 0) { adminMenu(); return; }

        attempts++;
        color(12); printf("\n[ERROR] UNAUTHORIZED ACCESS! Attempts left: %d\n\n", 5 - attempts); color(10);
    }
    color(12); printf("\n[SECURITY ALERT] Admin Terminal Locked due to 5 failed attempts!\n"); color(10);
    waitForEnter();
}


// ==========================================
// 8. MAIN ENTRY
// ==========================================

int main() {
    color(10); srand(time(NULL));
    load(); // Load data seamlessly
    int ch;

    while (1) {
        header();
        printf(" 1. System Login Options\n 2. Register New Identity\n 3. Exit System\n\nCommand >> ");
        if (scanf("%d", &ch) != 1) { clearInputBuffer(); continue; }

        if (ch == 3) { printf("\nDisconnecting... Goodbye.\n"); break; }
        else if (ch == 1) {
            header();
            printf(" 1. User Terminal\n 2. Root Admin Terminal\n 3. Forgot Password\n 0. Go Back\n\nCommand >> ");
            int option;
            if (scanf("%d", &option) == 1) {
                if (option == 1) userLogin();
                else if (option == 2) adminLogin();
                else if (option == 3) forgotPassword();
            } else { clearInputBuffer(); }
        }
        else if (ch == 2) createAccount();
    }
    return 0;
}
