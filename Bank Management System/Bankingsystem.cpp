#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>
#include <memory>

using namespace std;

/*
===============================================================================
                        DATE / FORMAT UTILITY CLASS
===============================================================================
*/
class DateUtils {
public:
    static string getCurrentDateTime() {
        time_t now = time(nullptr);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer);
    }

    static string formatDateTime(time_t t) {
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
        return string(buffer);
    }

    static string formatAmount(double amount) {
        stringstream ss;
        ss << "$" << fixed << setprecision(2) << amount;
        return ss.str();
    }
};

/*
===============================================================================
                                ENUMS
===============================================================================
*/
enum class TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER_SENT,
    TRANSFER_RECEIVED,
    ACCOUNT_CREATED,
    INTEREST_CREDITED
};

enum class AccountType {
    SAVINGS,
    CURRENT,
    FIXED_DEPOSIT
};

/*
===============================================================================
                            TRANSACTION CLASS
===============================================================================
*/
class Transaction {
private:
    string transactionId;
    string dateTime;
    TransactionType type;
    double amount;
    double balanceAfter;
    string description;
    string fromAccount;
    string toAccount;

    static int nextId;

    string generateTransactionId() {
        stringstream ss;
        ss << "TXN" << (++nextId);
        return ss.str();
    }

public:
    Transaction(TransactionType t, double amt, double balance, const string& desc)
        : type(t), amount(amt), balanceAfter(balance), description(desc) {
        transactionId = generateTransactionId();
        dateTime = DateUtils::getCurrentDateTime();
        fromAccount = "";
        toAccount = "";
    }

    Transaction(TransactionType t, double amt, double balance, const string& desc,
                const string& from, const string& to)
        : type(t), amount(amt), balanceAfter(balance), description(desc),
          fromAccount(from), toAccount(to) {
        transactionId = generateTransactionId();
        dateTime = DateUtils::getCurrentDateTime();
    }

    string getTransactionId() const { return transactionId; }
    string getDateTime() const { return dateTime; }
    TransactionType getType() const { return type; }
    double getAmount() const { return amount; }
    double getBalanceAfter() const { return balanceAfter; }
    string getDescription() const { return description; }

    string getTypeString() const {
        switch (type) {
            case TransactionType::DEPOSIT: return "DEPOSIT";
            case TransactionType::WITHDRAWAL: return "WITHDRAWAL";
            case TransactionType::TRANSFER_SENT: return "TRANSFER SENT";
            case TransactionType::TRANSFER_RECEIVED: return "TRANSFER RECEIVED";
            case TransactionType::ACCOUNT_CREATED: return "ACCOUNT CREATED";
            case TransactionType::INTEREST_CREDITED: return "INTEREST";
            default: return "UNKNOWN";
        }
    }

    void display() const {
        cout << "  " << left << setw(15) << transactionId
             << setw(22) << dateTime
             << setw(16) << getTypeString()
             << right << setw(12) << fixed << setprecision(2) << amount
             << setw(15) << balanceAfter << "\n";

        if (!description.empty()) {
            cout << "    Note: " << description << "\n";
        }
    }
};

int Transaction::nextId = 0;

/*
===============================================================================
                                ACCOUNT CLASS
===============================================================================
*/
class Account {
private:
    string accountNumber;
    string accountHolderName;
    AccountType accountType;
    double balance;
    bool isActive;
    time_t creationDate;
    vector<Transaction> transactions;

    static int nextAccountNumber;

    string generateAccountNumber() {
        stringstream ss;
        int nextNum = ++nextAccountNumber;
        ss << "ACC" << setfill('0') << setw(4) << nextNum;
        return ss.str();
    }

    void addTransaction(TransactionType type, double amount, double balanceAfter,
                        const string& description) {
        transactions.push_back(Transaction(type, amount, balanceAfter, description));
    }

    void addTransferTransaction(TransactionType type, double amount, double balanceAfter,
                                const string& description, const string& from, const string& to) {
        transactions.push_back(Transaction(type, amount, balanceAfter, description, from, to));
    }

public:
    Account(const string& holderName, AccountType type)
        : accountHolderName(holderName), accountType(type), balance(0.0), isActive(true) {
        accountNumber = generateAccountNumber();
        creationDate = time(nullptr);

        string desc = "Account created with type: " + getAccountTypeString();
        addTransaction(TransactionType::ACCOUNT_CREATED, 0.0, balance, desc);
    }

    string getAccountNumber() const { return accountNumber; }
    string getAccountHolderName() const { return accountHolderName; }
    AccountType getAccountType() const { return accountType; }
    double getBalance() const { return balance; }
    bool isAccountActive() const { return isActive; }

    string getAccountTypeString() const {
        switch (accountType) {
            case AccountType::SAVINGS: return "SAVINGS";
            case AccountType::CURRENT: return "CURRENT";
            case AccountType::FIXED_DEPOSIT: return "FIXED DEPOSIT";
            default: return "UNKNOWN";
        }
    }

    bool deposit(double amount, const string& description = "") {
        if (amount <= 0) {
            return false;
        }

        balance += amount;

        string desc = description.empty()
                      ? "Deposit of " + DateUtils::formatAmount(amount)
                      : description;

        addTransaction(TransactionType::DEPOSIT, amount, balance, desc);
        return true;
    }

    bool withdraw(double amount, const string& description = "") {
        if (amount <= 0) {
            return false;
        }

        if (balance < amount) {
            return false;
        }

        // Savings account minimum balance rule
        if (accountType == AccountType::SAVINGS && (balance - amount) < 500.0) {
            return false;
        }

        balance -= amount;

        string desc = description.empty()
                      ? "Withdrawal of " + DateUtils::formatAmount(amount)
                      : description;

        addTransaction(TransactionType::WITHDRAWAL, amount, balance, desc);
        return true;
    }

    bool transfer(Account& toAccount, double amount, const string& description = "") {
        if (amount <= 0) {
            return false;
        }

        if (balance < amount) {
            return false;
        }

        // Savings account minimum balance rule
        if (accountType == AccountType::SAVINGS && (balance - amount) < 500.0) {
            return false;
        }

        balance -= amount;
        toAccount.balance += amount;

        string sentDesc = description.empty()
                          ? "Transfer of " + DateUtils::formatAmount(amount) + " to " + toAccount.getAccountNumber()
                          : description;

        string receivedDesc = description.empty()
                              ? "Transfer received from " + accountNumber
                              : description;

        addTransferTransaction(TransactionType::TRANSFER_SENT, amount, balance,
                               sentDesc, accountNumber, toAccount.getAccountNumber());

        toAccount.addTransferTransaction(TransactionType::TRANSFER_RECEIVED, amount,
                                         toAccount.balance, receivedDesc,
                                         accountNumber, toAccount.getAccountNumber());

        return true;
    }

    void displayTransactions(int limit = 10) const {
        if (transactions.empty()) {
            cout << "\n  No transactions found.\n";
            return;
        }

        if (limit <= 0) {
            limit = 10;
        }

        cout << "\n  --------------------------------------------------------------------------------\n";
        cout << "  TRANSACTION HISTORY (Last " << limit << " transactions)\n";
        cout << "  --------------------------------------------------------------------------------\n";
        cout << "  " << left << setw(15) << "Txn ID"
             << setw(22) << "Date & Time"
             << setw(16) << "Type"
             << right << setw(12) << "Amount"
             << setw(15) << "Balance" << "\n";
        cout << "  --------------------------------------------------------------------------------\n";

        int total = static_cast<int>(transactions.size());
        int start = total - limit;
        if (start < 0) {
            start = 0;
        }

        for (int i = total - 1; i >= start; i--) {
            transactions[static_cast<size_t>(i)].display(); // FIXED warning here
        }

        cout << "  --------------------------------------------------------------------------------\n";
    }

    void displayAccountInfo() const {
        cout << "\n  Account Number:     " << accountNumber << "\n";
        cout << "  Account Holder:     " << accountHolderName << "\n";
        cout << "  Account Type:       " << getAccountTypeString() << "\n";
        cout << "  Current Balance:    " << DateUtils::formatAmount(balance) << "\n";
        cout << "  Account Status:     " << (isActive ? "Active" : "Inactive") << "\n";
        cout << "  Created Date:       " << DateUtils::formatDateTime(creationDate) << "\n";
    }
};

int Account::nextAccountNumber = 0;

/*
===============================================================================
                                CUSTOMER CLASS
===============================================================================
*/
class Customer {
private:
    string customerId;
    string name;
    string email;
    string phone;
    string address;
    vector<shared_ptr<Account>> accounts;
    bool isActive;
    time_t registrationDate;

    static int nextCustomerId;

    string generateCustomerId() {
        stringstream ss;
        int nextNum = ++nextCustomerId;
        ss << "CUST" << setfill('0') << setw(4) << nextNum;
        return ss.str();
    }

public:
    Customer(const string& custName, const string& custEmail,
             const string& custPhone, const string& custAddress)
        : name(custName), email(custEmail), phone(custPhone),
          address(custAddress), isActive(true) {
        customerId = generateCustomerId();
        registrationDate = time(nullptr);
    }

    string getCustomerId() const { return customerId; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    string getAddress() const { return address; }
    bool isCustomerActive() const { return isActive; }

    void addAccount(shared_ptr<Account> account) {
        accounts.push_back(account);
    }

    vector<shared_ptr<Account>>& getAccounts() {
        return accounts;
    }

    shared_ptr<Account> getAccount(const string& accountNumber) {
        for (size_t i = 0; i < accounts.size(); i++) {
            if (accounts[i]->getAccountNumber() == accountNumber) {
                return accounts[i];
            }
        }
        return nullptr;
    }

    void displayCustomerInfo() const {
        cout << "\n  Customer ID:        " << customerId << "\n";
        cout << "  Name:               " << name << "\n";
        cout << "  Email:              " << email << "\n";
        cout << "  Phone:              " << phone << "\n";
        cout << "  Address:            " << address << "\n";
        cout << "  Status:             " << (isActive ? "Active" : "Inactive") << "\n";
        cout << "  Registration Date:  " << DateUtils::formatDateTime(registrationDate) << "\n";

        if (accounts.empty()) {
            cout << "  No accounts found.\n";
        } else {
            cout << "\n  Associated Accounts:\n";
            for (size_t i = 0; i < accounts.size(); i++) {
                cout << "    - " << accounts[i]->getAccountNumber()
                     << " (" << accounts[i]->getAccountTypeString()
                     << ") - Balance: " << DateUtils::formatAmount(accounts[i]->getBalance()) << "\n";
            }
        }
    }
};

int Customer::nextCustomerId = 0;

/*
===============================================================================
                            BANKING SYSTEM CLASS
===============================================================================
*/
class BankingSystem {
private:
    map<string, shared_ptr<Customer>> customers;
    map<string, shared_ptr<Account>> accounts;

public:
    // FIX: return created customer ID
    string createCustomer(const string& name, const string& email,
                          const string& phone, const string& address) {
        shared_ptr<Customer> customer = make_shared<Customer>(name, email, phone, address);
        string id = customer->getCustomerId();
        customers[id] = customer;
        return id;
    }

    shared_ptr<Customer> findCustomer(const string& customerId) {
        map<string, shared_ptr<Customer>>::iterator it = customers.find(customerId);
        if (it != customers.end()) {
            return it->second;
        }
        return nullptr;
    }

    vector<shared_ptr<Customer>> getAllCustomers() {
        vector<shared_ptr<Customer>> customerList;
        map<string, shared_ptr<Customer>>::iterator it;

        for (it = customers.begin(); it != customers.end(); ++it) {
            customerList.push_back(it->second);
        }

        return customerList;
    }

    // FIX: return created account number
    string createAccount(const string& customerId, AccountType type) {
        shared_ptr<Customer> customer = findCustomer(customerId);
        if (!customer) {
            return "";
        }

        shared_ptr<Account> account = make_shared<Account>(customer->getName(), type);
        customer->addAccount(account);
        accounts[account->getAccountNumber()] = account;

        return account->getAccountNumber();
    }

    shared_ptr<Account> findAccount(const string& accountNumber) {
        map<string, shared_ptr<Account>>::iterator it = accounts.find(accountNumber);
        if (it != accounts.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool deposit(const string& accountNumber, double amount, const string& description = "") {
        shared_ptr<Account> account = findAccount(accountNumber);
        if (!account || !account->isAccountActive()) {
            return false;
        }
        return account->deposit(amount, description);
    }

    bool withdraw(const string& accountNumber, double amount, const string& description = "") {
        shared_ptr<Account> account = findAccount(accountNumber);
        if (!account || !account->isAccountActive()) {
            return false;
        }
        return account->withdraw(amount, description);
    }

    bool transfer(const string& fromAccountNumber, const string& toAccountNumber,
                  double amount, const string& description = "") {
        shared_ptr<Account> fromAccount = findAccount(fromAccountNumber);
        shared_ptr<Account> toAccount = findAccount(toAccountNumber);

        if (!fromAccount || !toAccount) {
            return false;
        }

        if (!fromAccount->isAccountActive() || !toAccount->isAccountActive()) {
            return false;
        }

        return fromAccount->transfer(*toAccount, amount, description);
    }

    void displayAccountStatement(const string& accountNumber, int transactionLimit = 10) {
        shared_ptr<Account> account = findAccount(accountNumber);
        if (!account) {
            cout << "\n  Account not found!\n";
            return;
        }

        cout << "\n  ============================================================\n";
        cout << "  ACCOUNT STATEMENT\n";
        cout << "  ============================================================\n";

        account->displayAccountInfo();
        account->displayTransactions(transactionLimit);
    }

    void displayCustomerDetails(const string& customerId) {
        shared_ptr<Customer> customer = findCustomer(customerId);
        if (!customer) {
            cout << "\n  Customer not found!\n";
            return;
        }

        cout << "\n  ============================================================\n";
        cout << "  CUSTOMER DETAILS\n";
        cout << "  ============================================================\n";

        customer->displayCustomerInfo();
    }

    void initializeDemoData() {
        // Create customers
        string cust1 = createCustomer("John Doe", "john@email.com", "1234567890", "123 Main St");
        string cust2 = createCustomer("Jane Smith", "jane@email.com", "0987654321", "456 Oak Ave");

        // Create accounts
        string acc1 = createAccount(cust1, AccountType::SAVINGS);
        string acc2 = createAccount(cust1, AccountType::CURRENT);
        string acc3 = createAccount(cust2, AccountType::SAVINGS);

        // Initial transactions
        if (acc1 != "") {
            deposit(acc1, 10000, "Initial deposit");
            withdraw(acc1, 500, "ATM withdrawal");
            deposit(acc1, 2000, "Salary credit");
        }

        if (acc2 != "") {
            deposit(acc2, 5000, "Initial deposit");
        }

        if (acc3 != "") {
            deposit(acc3, 8000, "Initial deposit");
        }

        // Transfer
        if (acc1 != "" && acc3 != "") {
            transfer(acc1, acc3, 1500, "Payment for services");
        }
    }
};

/*
===============================================================================
                            DISPLAY MANAGER CLASS
===============================================================================
*/
class DisplayManager {
public:
    void showWelcome() {
        cout << "\n";
        cout << "============================================================\n";
        cout << "             WELCOME TO BANKING SYSTEM                      \n";
        cout << "============================================================\n";
    }

    void showMainMenu() {
        cout << "\n";
        cout << "============================================================\n";
        cout << "                      MAIN MENU                             \n";
        cout << "============================================================\n";
        cout << "1. Create New Customer\n";
        cout << "2. Create New Account\n";
        cout << "3. Deposit Money\n";
        cout << "4. Withdraw Money\n";
        cout << "5. Transfer Money\n";
        cout << "6. View Account Statement\n";
        cout << "7. View Customer Details\n";
        cout << "8. List All Customers\n";
        cout << "9. Initialize Demo Data\n";
        cout << "0. Exit\n";
        cout << "------------------------------------------------------------\n";
        cout << "Enter your choice: ";
    }

    void showSuccess(const string& message) {
        cout << "\n[SUCCESS] " << message << "\n";
    }

    void showError(const string& message) {
        cout << "\n[ERROR] " << message << "\n";
    }

    void showInfo(const string& message) {
        cout << "\n[INFO] " << message << "\n";
    }

    void pressAnyKey() {
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void clearScreen() {
        for (int i = 0; i < 50; i++) {
            cout << "\n";
        }
    }

    string getInput(const string& prompt) {
        string input;
        cout << prompt;
        getline(cin, input);
        return input;
    }

    double getAmount(const string& prompt) {
        double amount;

        while (true) {
            cout << prompt;
            cin >> amount;

            if (cin.fail() || amount <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                showError("Invalid amount! Enter a positive number.");
            } else {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return amount;
            }
        }
    }

    int getChoice() {
        int choice;

        while (true) {
            cin >> choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                showError("Invalid input! Enter a number.");
                cout << "Enter again: ";
            } else {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return choice;
            }
        }
    }
};

/*
===============================================================================
                            BANK APPLICATION CLASS
===============================================================================
*/
class BankApplication {
private:
    BankingSystem bank;
    DisplayManager display;

    void createCustomer() {
        cout << "\n--- CREATE NEW CUSTOMER ---\n";

        string name = display.getInput("Full Name: ");
        string email = display.getInput("Email: ");
        string phone = display.getInput("Phone Number: ");
        string address = display.getInput("Address: ");

        if (name.empty() || email.empty() || phone.empty() || address.empty()) {
            display.showError("All fields are required!");
            return;
        }

        // FIX: get actual generated ID from system
        string customerId = bank.createCustomer(name, email, phone, address);

        if (customerId != "") {
            display.showSuccess("Customer created successfully!");
            display.showInfo("Your Customer ID is: " + customerId);
        } else {
            display.showError("Failed to create customer!");
        }
    }

    void createAccount() {
        cout << "\n--- CREATE NEW ACCOUNT ---\n";

        string customerId = display.getInput("Customer ID: ");

        cout << "Account Type:\n";
        cout << "1. Savings (Minimum balance $500)\n";
        cout << "2. Current\n";
        cout << "3. Fixed Deposit\n";
        cout << "Enter choice: ";

        int typeChoice = display.getChoice();

        AccountType type;

        switch (typeChoice) {
            case 1: type = AccountType::SAVINGS; break;
            case 2: type = AccountType::CURRENT; break;
            case 3: type = AccountType::FIXED_DEPOSIT; break;
            default:
                display.showError("Invalid account type!");
                return;
        }

        // FIX: get actual generated account number
        string accountNumber = bank.createAccount(customerId, type);

        if (accountNumber != "") {
            display.showSuccess("Account created successfully!");
            display.showInfo("New Account Number: " + accountNumber);
        } else {
            display.showError("Failed to create account! Customer not found.");
        }
    }

    void depositMoney() {
        cout << "\n--- DEPOSIT MONEY ---\n";

        string accountNumber = display.getInput("Account Number: ");
        double amount = display.getAmount("Amount to deposit: $");
        string description = display.getInput("Description (optional): ");

        if (bank.deposit(accountNumber, amount, description)) {
            display.showSuccess("Deposit successful!");

            shared_ptr<Account> account = bank.findAccount(accountNumber);
            if (account) {
                cout << "New balance: " << DateUtils::formatAmount(account->getBalance()) << "\n";
            }
        } else {
            display.showError("Deposit failed! Check account number or amount.");
        }
    }

    void withdrawMoney() {
        cout << "\n--- WITHDRAW MONEY ---\n";

        string accountNumber = display.getInput("Account Number: ");
        double amount = display.getAmount("Amount to withdraw: $");
        string description = display.getInput("Description (optional): ");

        if (bank.withdraw(accountNumber, amount, description)) {
            display.showSuccess("Withdrawal successful!");

            shared_ptr<Account> account = bank.findAccount(accountNumber);
            if (account) {
                cout << "New balance: " << DateUtils::formatAmount(account->getBalance()) << "\n";
            }
        } else {
            display.showError("Withdrawal failed! Check balance, minimum balance ($500 for savings), or account status.");
        }
    }

    void transferMoney() {
        cout << "\n--- TRANSFER MONEY ---\n";

        string fromAccount = display.getInput("From Account Number: ");
        string toAccount = display.getInput("To Account Number: ");
        double amount = display.getAmount("Amount to transfer: $");
        string description = display.getInput("Description (optional): ");

        if (bank.transfer(fromAccount, toAccount, amount, description)) {
            display.showSuccess("Transfer successful!");

            shared_ptr<Account> account = bank.findAccount(fromAccount);
            if (account) {
                cout << "Sender New Balance: " << DateUtils::formatAmount(account->getBalance()) << "\n";
            }
        } else {
            display.showError("Transfer failed! Check account numbers, balance, or minimum balance.");
        }
    }

    void viewStatement() {
        cout << "\n--- VIEW ACCOUNT STATEMENT ---\n";

        string accountNumber = display.getInput("Account Number (e.g. ACC0001): ");

        cout << "Number of transactions to show (default 10): ";
        int transactions = display.getChoice();

        if (transactions <= 0) {
            transactions = 10;
        }

        bank.displayAccountStatement(accountNumber, transactions);
    }

    void viewCustomerDetails() {
        cout << "\n--- VIEW CUSTOMER DETAILS ---\n";

        string customerId = display.getInput("Customer ID (e.g. CUST0001): ");
        bank.displayCustomerDetails(customerId);
    }

    void listAllCustomers() {
        cout << "\n--- ALL CUSTOMERS ---\n";

        vector<shared_ptr<Customer>> customers = bank.getAllCustomers();

        if (customers.empty()) {
            display.showInfo("No customers found.");
            return;
        }

        cout << "\n  ============================================================\n";
        cout << "  CUSTOMER LIST\n";
        cout << "  ============================================================\n";

        for (size_t i = 0; i < customers.size(); i++) {
            cout << "\n  Customer ID: " << customers[i]->getCustomerId() << "\n";
            cout << "  Name: " << customers[i]->getName() << "\n";
            cout << "  Email: " << customers[i]->getEmail() << "\n";
            cout << "  Phone: " << customers[i]->getPhone() << "\n";
            cout << "  Accounts: " << customers[i]->getAccounts().size() << "\n";
            cout << "  ------------------------------------------------------------\n";
        }
    }

    void initDemoData() {
        bank.initializeDemoData();

        display.showSuccess("Demo data initialized!");
        display.showInfo("Demo Customers:");
        display.showInfo("  CUST0001 - John Doe");
        display.showInfo("  CUST0002 - Jane Smith");

        display.showInfo("Demo Accounts:");
        display.showInfo("  ACC0001 - John Doe (Savings)");
        display.showInfo("  ACC0002 - John Doe (Current)");
        display.showInfo("  ACC0003 - Jane Smith (Savings)");
        display.showInfo("Use exact IDs above while testing.");
    }

public:
    void run() {
        bool running = true;

        while (running) {
            display.clearScreen();
            display.showWelcome();
            display.showMainMenu();

            int choice = display.getChoice();

            switch (choice) {
                case 1:
                    createCustomer();
                    break;
                case 2:
                    createAccount();
                    break;
                case 3:
                    depositMoney();
                    break;
                case 4:
                    withdrawMoney();
                    break;
                case 5:
                    transferMoney();
                    break;
                case 6:
                    viewStatement();
                    break;
                case 7:
                    viewCustomerDetails();
                    break;
                case 8:
                    listAllCustomers();
                    break;
                case 9:
                    initDemoData();
                    break;
                case 0:
                    running = false;
                    display.showInfo("Thank you for using our Banking System!");
                    break;
                default:
                    display.showError("Invalid choice! Please select 0-9.");
            }

            if (running && choice >= 1 && choice <= 9) {
                display.pressAnyKey();
            }
        }
    }
};

/*
===============================================================================
                                    MAIN
===============================================================================
*/
int main() {
    try {
        BankApplication app;
        app.run();
    }
    catch (const exception& e) {
        cerr << "\nFatal error: " << e.what() << endl;
        return 1;
    }

    return 0;
}