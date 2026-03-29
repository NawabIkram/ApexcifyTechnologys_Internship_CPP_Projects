#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <iomanip>
#include <limits>

using namespace std;

// ============================================================================
// ENCRYPTION/SECURITY UTILITIES
// ============================================================================

/**
 * Simple password hasher for demonstration purposes
 * In production, use proper cryptographic hashing (bcrypt, SHA-256 with salt)
 */
class PasswordHasher {
private:
    static const int XOR_KEY = 0x5A; // Simple XOR key for demo
    
    string xorEncrypt(const string& input) const {
        string output = input;
        for (char& c : output) {
            c ^= XOR_KEY;
        }
        return output;
    }
    
public:
    string hashPassword(const string& password) const {
        // In production, add salt and use proper hashing algorithm
        return xorEncrypt(password);
    }
    
    bool verifyPassword(const string& plainPassword, const string& hashedPassword) const {
        return hashPassword(plainPassword) == hashedPassword;
    }
};

// ============================================================================
// DOMAIN ENTITIES
// ============================================================================

/**
 * User entity - stores user information
 * Single Responsibility Principle (SRP): Only manages user data
 */
class User {
private:
    string username;
    string passwordHash;
    string role;
    string email;
    time_t registrationDate;
    bool isActive;
    
public:
    User(const string& username, const string& passwordHash, 
         const string& role = "student", const string& email = "")
        : username(username), passwordHash(passwordHash), role(role), 
          email(email), isActive(true) {
        registrationDate = time(nullptr);
    }
    
    // Getters
    string getUsername() const { return username; }
    string getPasswordHash() const { return passwordHash; }
    string getRole() const { return role; }
    string getEmail() const { return email; }
    time_t getRegistrationDate() const { return registrationDate; }
    bool isAccountActive() const { return isActive; }
    
    // Setters with validation
    void setEmail(const string& newEmail) { email = newEmail; }
    void setRole(const string& newRole) { role = newRole; }
    void deactivate() { isActive = false; }
    void activate() { isActive = true; }
    
    // For file storage
    string serialize() const {
        stringstream ss;
        ss << username << "|" 
           << passwordHash << "|"
           << role << "|"
           << email << "|"
           << registrationDate << "|"
           << isActive;
        return ss.str();
    }
    
    static User deserialize(const string& data) {
        stringstream ss(data);
        string username, passwordHash, role, email;
        string regDateStr, isActiveStr;
        
        getline(ss, username, '|');
        getline(ss, passwordHash, '|');
        getline(ss, role, '|');
        getline(ss, email, '|');
        getline(ss, regDateStr, '|');
        getline(ss, isActiveStr, '|');
        
        User user(username, passwordHash, role, email);
        user.registrationDate = stoll(regDateStr);
        user.isActive = (isActiveStr == "1");
        
        return user;
    }
};

// ============================================================================
// REPOSITORY INTERFACE & IMPLEMENTATION
// ============================================================================

/**
 * Interface for user data operations
 * Interface Segregation Principle (ISP): Focused interface for user repository
 */
class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual bool saveUser(const User& user) = 0;
    virtual bool findUser(const string& username, User& outUser) = 0;
    virtual bool userExists(const string& username) = 0;
    virtual bool updateUser(const User& user) = 0;
    virtual vector<User> getAllUsers() = 0;
};

/**
 * File-based user repository implementation
 * Single Responsibility Principle (SRP): Only handles file operations
 */
class FileUserRepository : public IUserRepository {
private:
    string filename;
    PasswordHasher hasher;
    
    void ensureFileExists() {
        ifstream file(filename);
        if (!file.is_open()) {
            ofstream createFile(filename);
            createFile.close();
        }
        file.close();
    }
    
    map<string, User> loadUsers() {
        map<string, User> users;
        ensureFileExists();
        
        ifstream file(filename);
        string line;
        
        while (getline(file, line)) {
            if (!line.empty()) {
                try {
                    User user = User::deserialize(line);
                    users[user.getUsername()] = user;
                } catch (const exception& e) {
                    // Skip corrupted lines
                    cerr << "Warning: Skipping corrupted user data\n";
                }
            }
        }
        file.close();
        return users;
    }
    
    void saveUsers(const map<string, User>& users) {
        ofstream file(filename);
        for (const auto& pair : users) {
            file << pair.second.serialize() << "\n";
        }
        file.close();
    }
    
public:
    FileUserRepository(const string& file = "users.dat") : filename(file) {
        ensureFileExists();
    }
    
    bool saveUser(const User& user) override {
        map<string, User> users = loadUsers();
        
        if (users.find(user.getUsername()) != users.end()) {
            return false; // User already exists
        }
        
        users[user.getUsername()] = user;
        saveUsers(users);
        return true;
    }
    
    bool findUser(const string& username, User& outUser) override {
        map<string, User> users = loadUsers();
        auto it = users.find(username);
        
        if (it != users.end()) {
            outUser = it->second;
            return true;
        }
        return false;
    }
    
    bool userExists(const string& username) override {
        map<string, User> users = loadUsers();
        return users.find(username) != users.end();
    }
    
    bool updateUser(const User& user) override {
        map<string, User> users = loadUsers();
        
        if (users.find(user.getUsername()) == users.end()) {
            return false;
        }
        
        users[user.getUsername()] = user;
        saveUsers(users);
        return true;
    }
    
    vector<User> getAllUsers() override {
        vector<User> userList;
        map<string, User> users = loadUsers();
        
        for (const auto& pair : users) {
            userList.push_back(pair.second);
        }
        return userList;
    }
};

// ============================================================================
// VALIDATION SERVICE
// ============================================================================

/**
 * Handles all input validation
 * Single Responsibility Principle (SRP): Only validates inputs
 */
class ValidationService {
private:
    static string trim(const string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        return (start == string::npos) ? "" : str.substr(start, end - start + 1);
    }
    
    static bool isAlphanumeric(const string& str) {
        for (char c : str) {
            if (!isalnum(c) && c != '_' && c != '-') {
                return false;
            }
        }
        return true;
    }
    
public:
    static bool validateUsername(const string& username, string& errorMessage) {
        string trimmed = trim(username);
        
        if (trimmed.empty()) {
            errorMessage = "Username cannot be empty";
            return false;
        }
        
        if (trimmed.length() < 3) {
            errorMessage = "Username must be at least 3 characters long";
            return false;
        }
        
        if (trimmed.length() > 20) {
            errorMessage = "Username must be at most 20 characters long";
            return false;
        }
        
        if (!isAlphanumeric(trimmed)) {
            errorMessage = "Username can only contain letters, numbers, underscore, and hyphen";
            return false;
        }
        
        return true;
    }
    
    static bool validatePassword(const string& password, string& errorMessage) {
        if (password.empty()) {
            errorMessage = "Password cannot be empty";
            return false;
        }
        
        if (password.length() < 6) {
            errorMessage = "Password must be at least 6 characters long";
            return false;
        }
        
        if (password.length() > 50) {
            errorMessage = "Password must be at most 50 characters long";
            return false;
        }
        
        // Check for at least one digit
        bool hasDigit = false;
        // Check for at least one uppercase letter
        bool hasUpper = false;
        // Check for at least one lowercase letter
        bool hasLower = false;
        
        for (char c : password) {
            if (isdigit(c)) hasDigit = true;
            if (isupper(c)) hasUpper = true;
            if (islower(c)) hasLower = true;
        }
        
        if (!hasDigit || !hasUpper || !hasLower) {
            errorMessage = "Password must contain at least one digit, one uppercase, and one lowercase letter";
            return false;
        }
        
        return true;
    }
    
    static bool validateEmail(const string& email, string& errorMessage) {
        string trimmed = trim(email);
        
        if (trimmed.empty()) {
            errorMessage = "Email cannot be empty";
            return false;
        }
        
        // Basic email validation
        size_t atPos = trimmed.find('@');
        size_t dotPos = trimmed.find('.', atPos);
        
        if (atPos == string::npos || dotPos == string::npos || 
            atPos == 0 || dotPos == atPos + 1 || dotPos == trimmed.length() - 1) {
            errorMessage = "Invalid email format";
            return false;
        }
        
        return true;
    }
};

// ============================================================================
// AUTHENTICATION SERVICE
// ============================================================================

/**
 * Manages authentication logic
 * Single Responsibility Principle (SRP): Only handles authentication
 * Dependency Inversion Principle (DIP): Depends on abstraction IUserRepository
 */
class AuthService {
private:
    unique_ptr<IUserRepository> repository;
    PasswordHasher hasher;
    
public:
    AuthService(unique_ptr<IUserRepository> repo) : repository(move(repo)) {}
    
    bool registerUser(const string& username, const string& password, 
                      const string& email, const string& role, string& errorMessage) {
        // Validate inputs
        if (!ValidationService::validateUsername(username, errorMessage)) {
            return false;
        }
        
        if (!ValidationService::validatePassword(password, errorMessage)) {
            return false;
        }
        
        if (!ValidationService::validateEmail(email, errorMessage)) {
            return false;
        }
        
        // Check if user already exists
        if (repository->userExists(username)) {
            errorMessage = "Username already exists. Please choose a different username.";
            return false;
        }
        
        // Hash password and create user
        string hashedPassword = hasher.hashPassword(password);
        User newUser(username, hashedPassword, role, email);
        
        // Save user
        if (repository->saveUser(newUser)) {
            errorMessage = "";
            return true;
        } else {
            errorMessage = "Failed to save user data";
            return false;
        }
    }
    
    bool loginUser(const string& username, const string& password, 
                   User& loggedInUser, string& errorMessage) {
        // Find user
        User user("", "");
        if (!repository->findUser(username, user)) {
            errorMessage = "Username not found";
            return false;
        }
        
        // Check if account is active
        if (!user.isAccountActive()) {
            errorMessage = "Account is deactivated. Please contact administrator.";
            return false;
        }
        
        // Verify password
        if (!hasher.verifyPassword(password, user.getPasswordHash())) {
            errorMessage = "Incorrect password";
            return false;
        }
        
        loggedInUser = user;
        errorMessage = "";
        return true;
    }
    
    bool changePassword(const string& username, const string& oldPassword, 
                        const string& newPassword, string& errorMessage) {
        // Validate new password
        if (!ValidationService::validatePassword(newPassword, errorMessage)) {
            return false;
        }
        
        // Find user
        User user("", "");
        if (!repository->findUser(username, user)) {
            errorMessage = "User not found";
            return false;
        }
        
        // Verify old password
        if (!hasher.verifyPassword(oldPassword, user.getPasswordHash())) {
            errorMessage = "Incorrect current password";
            return false;
        }
        
        // Update password
        string newHashedPassword = hasher.hashPassword(newPassword);
        User updatedUser = user;
        updatedUser = User(username, newHashedPassword, user.getRole(), user.getEmail());
        
        if (repository->updateUser(updatedUser)) {
            errorMessage = "";
            return true;
        } else {
            errorMessage = "Failed to update password";
            return false;
        }
    }
    
    bool isUsernameAvailable(const string& username) {
        return !repository->userExists(username);
    }
};

// ============================================================================
// SESSION MANAGER
// ============================================================================

/**
 * Manages active user session
 * Single Responsibility Principle (SRP): Only handles session management
 */
class SessionManager {
private:
    User currentUser;
    bool isLoggedIn;
    time_t loginTime;
    
public:
    SessionManager() : isLoggedIn(false), loginTime(0) {}
    
    void startSession(const User& user) {
        currentUser = user;
        isLoggedIn = true;
        loginTime = time(nullptr);
    }
    
    void endSession() {
        currentUser = User("", "");
        isLoggedIn = false;
        loginTime = 0;
    }
    
    bool isActive() const {
        return isLoggedIn;
    }
    
    User getCurrentUser() const {
        return currentUser;
    }
    
    string getSessionDuration() const {
        if (!isLoggedIn) return "No active session";
        
        time_t now = time(nullptr);
        double duration = difftime(now, loginTime);
        
        int hours = static_cast<int>(duration) / 3600;
        int minutes = (static_cast<int>(duration) % 3600) / 60;
        int seconds = static_cast<int>(duration) % 60;
        
        stringstream ss;
        ss << hours << "h " << minutes << "m " << seconds << "s";
        return ss.str();
    }
};

// ============================================================================
// DISPLAY MANAGER
// ============================================================================

/**
 * Manages all console output
 * Single Responsibility Principle (SRP): Only handles display logic
 */
class DisplayManager {
private:
    static const int WIDTH = 70;
    
    void printLine(char fill = '=', int width = WIDTH) {
        cout << string(width, fill) << "\n";
    }
    
    void printCentered(const string& text, int width = WIDTH) {
        int padding = (width - text.length()) / 2;
        if (padding > 0) {
            cout << string(padding, ' ') << text << "\n";
        } else {
            cout << text << "\n";
        }
    }
    
public:
    void showWelcome() {
        printLine();
        printCentered("SCHOOL & ACADEMY MANAGEMENT SYSTEM");
        printLine();
        printCentered("Welcome to Secure Authentication System");
        printLine();
        cout << "\n";
    }
    
    void showMainMenu() {
        cout << "\n";
        printLine('-');
        cout << "MAIN MENU\n";
        printLine('-');
        cout << "1. Register New Account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        printLine('-');
        cout << "Enter your choice (1-3): ";
    }
    
    void showUserMenu(const User& user) {
        cout << "\n";
        printLine('-');
        cout << "USER DASHBOARD\n";
        printLine('-');
        cout << "Welcome, " << user.getUsername() << "!\n";
        cout << "Role: " << user.getRole() << "\n";
        cout << "Email: " << user.getEmail() << "\n\n";
        cout << "1. Change Password\n";
        cout << "2. View Account Details\n";
        cout << "3. Logout\n";
        printLine('-');
        cout << "Enter your choice (1-3): ";
    }
    
    void showRegistrationForm() {
        cout << "\n";
        printLine('-');
        cout << "USER REGISTRATION\n";
        printLine('-');
    }
    
    void showLoginForm() {
        cout << "\n";
        printLine('-');
        cout << "USER LOGIN\n";
        printLine('-');
    }
    
    void showAccountDetails(const User& user) {
        cout << "\n";
        printLine('-');
        cout << "ACCOUNT DETAILS\n";
        printLine('-');
        cout << "Username: " << user.getUsername() << "\n";
        cout << "Role: " << user.getRole() << "\n";
        cout << "Email: " << user.getEmail() << "\n";
        cout << "Account Status: " << (user.isAccountActive() ? "Active" : "Inactive") << "\n";
        
        time_t regTime = user.getRegistrationDate();
        cout << "Registered: " << ctime(&regTime);
        printLine('-');
    }
    
    void showSuccess(const string& message) {
        cout << "\n? SUCCESS: " << message << "\n";
    }
    
    void showError(const string& message) {
        cout << "\n? ERROR: " << message << "\n";
    }
    
    void showInfo(const string& message) {
        cout << "\n? INFO: " << message << "\n";
    }
    
    void pressAnyKey() {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
    
    void clearScreen() {
        // Simple clear screen (works on most systems)
        cout << string(50, '\n');
    }
};

// ============================================================================
// APPLICATION CONTROLLER (FACADE PATTERN)
// ============================================================================

/**
 * Main application controller - coordinates all components
 * Facade Pattern: Provides simplified interface to complex subsystem
 */
class SchoolManagementApp {
private:
    unique_ptr<AuthService> authService;
    SessionManager session;
    DisplayManager display;
    
    string getInput(const string& prompt) {
        string input;
        cout << prompt;
        getline(cin, input);
        return input;
    }
    
    void handleRegistration() {
        display.showRegistrationForm();
        
        string username, password, confirmPassword, email, role;
        
        username = getInput("Username (3-20 chars, alphanumeric): ");
        password = getInput("Password (min 6 chars, with digit, upper & lower): ");
        confirmPassword = getInput("Confirm Password: ");
        
        if (password != confirmPassword) {
            display.showError("Passwords do not match!");
            return;
        }
        
        email = getInput("Email: ");
        
        cout << "Role (student/teacher/admin) [student]: ";
        getline(cin, role);
        if (role.empty()) role = "student";
        
        string errorMessage;
        if (authService->registerUser(username, password, email, role, errorMessage)) {
            display.showSuccess("Registration successful! You can now login.");
        } else {
            display.showError(errorMessage);
        }
    }
    
    void handleLogin() {
        display.showLoginForm();
        
        string username, password;
        username = getInput("Username: ");
        password = getInput("Password: ");
        
        User loggedInUser("", "");
        string errorMessage;
        
        if (authService->loginUser(username, password, loggedInUser, errorMessage)) {
            session.startSession(loggedInUser);
            display.showSuccess("Login successful!");
            handleUserSession();
        } else {
            display.showError(errorMessage);
        }
    }
    
    void handleChangePassword() {
        if (!session.isActive()) {
            display.showError("No active session!");
            return;
        }
        
        User currentUser = session.getCurrentUser();
        cout << "\nChange Password for: " << currentUser.getUsername() << "\n";
        
        string oldPassword, newPassword, confirmPassword;
        oldPassword = getInput("Current Password: ");
        newPassword = getInput("New Password: ");
        confirmPassword = getInput("Confirm New Password: ");
        
        if (newPassword != confirmPassword) {
            display.showError("New passwords do not match!");
            return;
        }
        
        string errorMessage;
        if (authService->changePassword(currentUser.getUsername(), oldPassword, 
                                        newPassword, errorMessage)) {
            display.showSuccess("Password changed successfully!");
        } else {
            display.showError(errorMessage);
        }
    }
    
    void handleViewAccount() {
        if (!session.isActive()) {
            display.showError("No active session!");
            return;
        }
        
        display.showAccountDetails(session.getCurrentUser());
    }
    
    void handleLogout() {
        if (session.isActive()) {
            session.endSession();
            display.showSuccess("Logged out successfully!");
        } else {
            display.showInfo("No active session to logout");
        }
    }
    
    void handleUserSession() {
        bool sessionActive = true;
        
        while (sessionActive && session.isActive()) {
            display.showUserMenu(session.getCurrentUser());
            
            int choice;
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            switch (choice) {
                case 1:
                    handleChangePassword();
                    break;
                case 2:
                    handleViewAccount();
                    break;
                case 3:
                    handleLogout();
                    sessionActive = false;
                    break;
                default:
                    display.showError("Invalid choice. Please try again.");
            }
            
            if (sessionActive && session.isActive()) {
                display.pressAnyKey();
                display.clearScreen();
            }
        }
    }
    
public:
    SchoolManagementApp() {
        auto repository = make_unique<FileUserRepository>("school_users.dat");
        authService = make_unique<AuthService>(move(repository));
    }
    
    void run() {
        display.showWelcome();
        
        bool running = true;
        while (running) {
            display.showMainMenu();
            
            int choice;
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            switch (choice) {
                case 1:
                    handleRegistration();
                    break;
                case 2:
                    handleLogin();
                    break;
                case 3:
                    running = false;
                    display.showInfo("Thank you for using School Management System!");
                    break;
                default:
                    display.showError("Invalid choice. Please select 1-3.");
            }
            
            if (running) {
                display.pressAnyKey();
                display.clearScreen();
            }
        }
    }
};

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main() {
    try {
        SchoolManagementApp app;
        app.run();
    } catch (const exception& e) {
        cerr << "\nFatal error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
