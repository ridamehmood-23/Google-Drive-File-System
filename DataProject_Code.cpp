#include<iostream>
#include<string>
#include<ctime>
#include<cstdlib>
//......................................................................................................................//
using namespace std;
//***********************************************************************************************************************//
const int totalusers = 100; //total users 
const int totalfiles = 15; //total files 
const int max_users = 80; //maximum users 
const int max_files = 10; //maximum files 
const int HASH_TABLE_SIZE = 100; //size for hash table
//***********************************************************************************************************************//
void set_color(int code)
{
    cout << "\033[" << code << "m";
}
enum class UserRole { VIEWER, EDITOR, ADMIN }; //for access 
//***********************************************************************************************************************//
// compress a string using RLE 
string compressRLE(const string& input)
{
    string compressed;
    if (input.empty())
    {
        return compressed;
    }
    char current = input[0];
    int count = 1;

    for (size_t i = 1; i < input.length(); ++i)
    {
        if (input[i] == current && count < 9)
        {
            count++;
        }
        else
        {
            compressed += to_string(count) + current;
            current = input[i];
            count = 1;
        }
    }
    compressed += to_string(count) + current;
    return compressed;
}
// Decompress using RLE
string decompressRLE(const string& input)
{
    string decompressed;
    for (size_t i = 0; i < input.length(); i += 2)
    {
        if (i + 1 >= input.length())
        {
            break;
        }
        int count = input[i] - '0'; //char to digit 
        char c = input[i + 1];
        for (int j = 0; j < count; ++j)
        {
            decompressed += c;
        }
    }
    return decompressed;
}
//***********************************************************************************************************************//
struct User
{
    string username;
    string password;
    string securityQuestion;
    string securityAnswer;
    UserRole role = UserRole::VIEWER; // Default initialization
};
//***********************************************************************************************************************//
class PermissionGraph  //implement using  graphs 
{
private:
    bool permissions[max_users][max_users][3]; // [from][to][read, write, execute]
    string usernames[max_users];
    UserRole roles[max_users];
    int userCount;
    //find index for user
    int findIndex(string username)
    {
        for (int i = 0; i < userCount; i++)
        {
            if (usernames[i] == username)
                return i;
        }
        return -1;
    }
public:
    PermissionGraph()    //non parametrized constructor 
    {
        userCount = 0;
        for (int i = 0; i < max_users; i++)
        {
            for (int j = 0; j < max_users; j++)
            {
                permissions[i][j][0] = false; // read
                permissions[i][j][1] = false; // write
                permissions[i][j][2] = false; // execute
            }
            roles[i] = UserRole::VIEWER; // Initialize roles array
        }
    }
    //add user for that role
    void addUser(string username, UserRole role)
    {
        if (findIndex(username) == -1 && userCount < max_users)
        {
            usernames[userCount] = username;
            roles[userCount] = role;
            // Set  permissions based on role
            int idx = userCount++;
            if (role == UserRole::ADMIN)
            {
                permissions[idx][idx][0] = true; // read
                permissions[idx][idx][1] = true; // write
                permissions[idx][idx][2] = true; // execute
            }
            else if (role == UserRole::EDITOR)
            {
                permissions[idx][idx][0] = true; // read
                permissions[idx][idx][1] = true; // write
                permissions[idx][idx][2] = false; // no execute
            }
            else // VIEWER
            {
                permissions[idx][idx][0] = true; // read
                permissions[idx][idx][1] = false; // no write
                permissions[idx][idx][2] = false; // no execute
            }
        }
    }
    // give permission
    bool hasPermission(string username, int caseNumber)
    {
        int idx = findIndex(username);
        if (idx == -1) return false;

        UserRole role = roles[idx];
        if (role == UserRole::ADMIN)
        {
            return true; // Admin has access to all cases
        }
        else if (role == UserRole::EDITOR)
        {
            // fpr editor  cases 1 to 13, 18
            return (caseNumber >= 1 && caseNumber <= 13) || caseNumber == 18;
        }
        else
        {
            // for viewer cases 1, 2, 3, 6, 4, 7, 0
            return caseNumber == 1 || caseNumber == 2 || caseNumber == 3 || caseNumber == 13 ||
                caseNumber == 8 || caseNumber == 4 || caseNumber == 7 || caseNumber == 0;
        }
    }
    //grant permission for that 
    void grantPermission(string adminUser, string toUser, bool read, bool write, bool execute)
    {
        int adminIdx = findIndex(adminUser);
        int toIdx = findIndex(toUser);
        if (adminIdx == -1 || toIdx == -1)
        {
            set_color(31);
            cout << "Invalid users." << endl;
            set_color(0);
            return;
        }
        if (roles[adminIdx] != UserRole::ADMIN)
        {
            set_color(31);
            cout << "Only admins can modify permissions." << endl;
            set_color(0);
            return;
        }
        permissions[adminIdx][toIdx][0] = read;
        permissions[adminIdx][toIdx][1] = write;
        permissions[adminIdx][toIdx][2] = execute;
        set_color(32);
        cout << "Permissions updated for " << toUser << endl;
        set_color(0);
    }
};
//***********************************************************************************************************************//
class UserManagement //implmenet using graph and stack
{
private:
    User users[totalusers]; //array of users 
    int userCount;
    string loggedInUsers[totalusers]; //stack-like array for logged-in users
    int loggedInCount;
public:
    UserManagement() //non-parametrized constructor 
    {
        userCount = 0;
        loggedInCount = 0;
        for (int i = 0; i < totalusers; i++)
        {
            loggedInUsers[i] = "";
        }
    }
    //sign up for user also ask role in sign up 
    bool signup(string username, string password, string question, string answer, UserRole role)
    {
        if (userCount >= totalusers)
        {
            set_color(31);
            cout << "User limit reached." << endl;
            return false;
        }
        users[userCount].username = username;
        users[userCount].password = password;
        users[userCount].securityQuestion = question;
        users[userCount].securityAnswer = answer;
        users[userCount].role = role; // Set role
        userCount++;
        set_color(32);
        cout << "Account created successfully." << endl;
        set_color(0);
        return true;
    }
    UserRole getUserRole(string username) // to get user role
    {
        for (int i = 0; i < userCount; i++)
        {
            if (users[i].username == username)
            {
                return users[i].role;
            }
        }
        return UserRole::VIEWER;  //if not any then default is viewer
    }
    bool login(string username, string password)  //login function
    {
        for (int i = 0; i < userCount; i++)
        {
            if (users[i].username == username && users[i].password == password)
            {
                //functionality add just for same user names 
               /* for (int j = 0; j < loggedInCount; j++)
                {
                    if (loggedInUsers[j] == username)
                    {
                        set_color(31);
                        cout << "User " << username << " is already logged in." << endl;
                        set_color(0);
                        return false;
                    }
                }*/
                if (loggedInCount < totalusers)
                {
                    loggedInUsers[loggedInCount++] = username;
                    set_color(32);
                    cout << "Login successful." << endl;
                    set_color(0);
                    return true;
                }
                else
                {
                    set_color(31);
                    cout << "Logged-in user limit reached." << endl;
                    set_color(0);
                    return false;
                }
            }
        }
        set_color(31);
        cout << "Invalid username or password." << endl;
        set_color(0);
        return false;
    }
    bool recoverPassword(string username, string answer, string newPassword) //password recover
    {
        bool isLogged = false;
        for (int i = 0; i < loggedInCount; i++)
        {
            if (loggedInUsers[i] == username)
            {
                isLogged = true;
                break;
            }
        }
        if (!isLogged)
        {
            set_color(31);
            cout << "User not logged in so no recovery of password is possible." << endl;
            set_color(0);
            return false;
        }
        for (int i = 0; i < userCount; i++)
        {
            if (users[i].username == username && users[i].securityAnswer == answer)
            {
                users[i].password = newPassword;
                set_color(32);
                cout << "Password updated successfully." << endl;
                set_color(0);
                return true;
            }
        }
        set_color(31);
        cout << "Invalid username or security answer." << endl;
        set_color(0);
        return false;
    }
    void showSecurityQuestion(string username) //show the security question 
    {
        for (int i = 0; i < userCount; i++)
        {
            if (users[i].username == username)
            {
                set_color(35);
                cout << "Security Question: " << users[i].securityQuestion << endl;
                set_color(0);
                return;
            }
        }
        set_color(31);
        cout << "Username not found." << endl;
        set_color(0);
    }
    void logout()  //log out 
    {
        if (loggedInCount == 0)
        {
            set_color(31);
            cout << "User not logged in so no logout." << endl;
            set_color(0);
        }
        else
        {
            set_color(32);
            cout << "User " << loggedInUsers[loggedInCount - 1] << " logged out successfully." << endl;
            loggedInUsers[loggedInCount - 1] = "";
            loggedInCount--;
            set_color(0);
        }
    }
    bool isLoggedIn() //check for log in 
    {
        return loggedInCount > 0;
    }
    string getCurrentUser()  //return the current user 
    {
        if (loggedInCount > 0)
            return loggedInUsers[loggedInCount - 1]; //return the most recently logged-in user
        return "";
    }
};
//***********************************************************************************************************************//
struct FileVersion //implement using stack
{
    string content; // for storing the compress content
    FileVersion* next;
    FileVersion(string c)  //parametrized constructor 
    {
        content = compressRLE(c); // compress content at the time of creation of file 
        next = nullptr;
    }
};
//***********************************************************************************************************************//
struct File //implmenet using stack and queue 
{
    string name;
    FileVersion* head_version;
    File(string fileName, string content) //paramzterized constructor 
    {
        name = fileName;
        head_version = new FileVersion(content);
    }
    void updateContent(string newContent) //update content 
    {
        FileVersion* newVersion = new FileVersion(newContent);
        newVersion->next = head_version;
        head_version = newVersion;
        set_color(32);
        cout << "File updated." << endl;
        set_color(0);
    }
    void showLatestContent() //show latest content
    {
        if (head_version) //check if not null
        {
            set_color(34);
            cout << "Latest Content of '" << name << "': " << decompressRLE(head_version->content) << endl;
            set_color(0);
        }
        else
        {
            set_color(33);
            cout << "No content." << endl;
            set_color(0);
        }
    }
    void showAllVersions() //show all versions of file 
    {
        FileVersion* temp = head_version;
        int version = 1;
        while (temp != nullptr)
        {
            set_color(34);
            cout << "Version " << version << ": " << decompressRLE(temp->content) << endl;
            temp = temp->next;
            version++;
        }
    }
    void rollbackVersion() //roll back to previous version
    {
        if (head_version != nullptr && head_version->next != nullptr)
        {
            FileVersion* temp = head_version;
            head_version = head_version->next;
            delete temp;
            set_color(32);
            cout << "Rolled back to previous version." << endl;
            set_color(0);
        }
        else
        {
            set_color(31);
            cout << "No previous version to rollback." << endl;
            set_color(0);
        }
    }
    // File searching function using queue
    void searchFile(File* files[], int fileCount, string fileName)
    {
        int queue[max_files];
        int front = 0, rear = 0;
        bool found = false;
        for (int i = 0; i < fileCount; i++) //enqueue files 
        {
            queue[rear++] = i;
        }
        while (front < rear) //display if found 
        {
            int curr = queue[front++];
            if (files[curr]->name == fileName)
            {
                set_color(32);
                cout << "File found: " << files[curr]->name << endl;
                files[curr]->showLatestContent();
                set_color(0);
                found = true;
                break;
            }
        }
        if (!found) //in case of file not found 
        {
            set_color(31);
            cout << "File '" << fileName << "' not found." << endl;
            set_color(0);
        }
    }
};
//***********************************************************************************************************************//
struct FileMetadata     //for hash table
{
    string name;
    int size;
    string type;
    string owner;
    File* filePtr; // Pointer to the actual File
    FileMetadata* next; // For collision resolution (chaining)
    FileMetadata(string n, int s, string t, string o, File* f)
    {
        name = n;
        size = s;
        type = t;
        owner = o;
        filePtr = f;
        next = nullptr;
    }
};
//***********************************************************************************************************************//
class FileHashTable
{
private:
    FileMetadata* table[HASH_TABLE_SIZE];
    // Simple hash function for file name
    int hashFunction(string name)
    {
        int sum = 0;
        for (size_t i = 0; i < name.length(); i++)
        {
            sum += name[i];
        }
        return sum % HASH_TABLE_SIZE;
    }
public:
    FileHashTable()
    {
        for (int i = 0; i < HASH_TABLE_SIZE; i++)
        {
            table[i] = nullptr;
        }
    }
    // Insert file metadata into hash table
    bool insert(string name, int size, string type, string owner, File* file)
    {
        int index = hashFunction(name);
        FileMetadata* current = table[index];
        // Check for duplicate file
        while (current != nullptr)
        {
            if (current->name == name)
            {
                set_color(31);
                cout << "Duplicate file name: " << name << endl;
                set_color(0);
                return false;
            }
            current = current->next;
        }
        // Insert new metadata
        FileMetadata* newNode = new FileMetadata(name, size, type, owner, file);
        newNode->next = table[index];
        table[index] = newNode;
        return true;
    }
    // Search for file metadata by name
    File* search(string name)
    {
        int index = hashFunction(name);
        FileMetadata* current = table[index];
        while (current != nullptr)
        {
            if (current->name == name)
            {
                return current->filePtr;
            }
            current = current->next;
        }
        return nullptr;
    }
    // Remove file metadata from hash table
    void remove(string name)
    {
        int index = hashFunction(name);
        FileMetadata* current = table[index];
        FileMetadata* prev = nullptr;
        while (current != nullptr)
        {
            if (current->name == name)
            {
                if (prev == nullptr)
                {
                    table[index] = current->next;
                }
                else
                {
                    prev->next = current->next;
                }
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }
    // Display all metadata in hash table
    void display()
    {
        set_color(34);
        cout << "File Metadata in Hash Table:" << endl;
        set_color(0);
        for (int i = 0; i < HASH_TABLE_SIZE; i++)
        {
            FileMetadata* current = table[i];
            while (current != nullptr)
            {
                set_color(35);
                cout << "File: " << current->name << endl;
                cout << " Size: " << current->size << endl;
                cout << " Type: " << current->type << endl;
                cout << " Owner: " << current->owner << endl;
                set_color(0);
                current = current->next;
            }
        }
    }
};
//***********************************************************************************************************************//
struct RecycleNode  //implment using stack
{
    File* deletedFile;
    RecycleNode* next;
    RecycleNode(File* f) //parametrized constructor 
    {
        deletedFile = f;
        next = nullptr;
    }
};
//***********************************************************************************************************************//
class RecycleBin //implement using stack
{
public:
    File* stack[totalusers] = {}; //stack for users 
    int top;
    RecycleBin()
    {
        top = -1;
    }
    void push(File* file) //push function 
    {
        if (top < totalusers - 1)
        {
            stack[++top] = file;
        }
    }
    File* pop()  //pop function
    {
        if (top >= 0)
        {
            return stack[top--];
        }
        return nullptr;
    }
    bool isEmpty()  //check stack empty or not 
    {
        return top == -1;
    }
    void display()
    {
        if (isEmpty())
        {
            set_color(31);
            cout << "Recycle bin is empty." << endl;
            set_color(0);
        }
        else
        {
            for (int i = top; i >= 0; i--)
            {
                set_color(35);
                cout << "Deleted File: " << stack[i]->name << endl;
                set_color(0);
            }
        }
    }
};
//***********************************************************************************************************************//
class RecentFiles //implement using  loops  and conditions
{
private:
    File* recentFiles[totalfiles];
    int count;
public:
    RecentFiles()  //non prametrized construtor 
    {
        count = 0;
        for (int i = 0; i < totalfiles; i++)
        {
            recentFiles[i] = nullptr;
        }
    }
    bool isFileRecent(string fileName) //check if file is in recent or not 
    {
        for (int i = 0; i < count; i++)
        {
            if (recentFiles[i] != nullptr && recentFiles[i]->name == fileName)
            {
                return true;
            }
        }
        return false;
    }
    void addFile(File* file) //add file 
    {
        for (int i = 0; i < count; i++)
        {
            if (recentFiles[i] == file)
            {
                for (int j = i; j < count - 1; j++)
                {
                    recentFiles[j] = recentFiles[j + 1];
                }
                recentFiles[count - 1] = nullptr;
                count--;
                break;
            }
        }
        if (count < totalfiles)
        {
            recentFiles[count++] = file; //add that file
        }
        else
        {
            for (int i = 1; i < totalfiles; i++)
            {
                recentFiles[i - 1] = recentFiles[i];
            }
            recentFiles[totalfiles - 1] = file;
        }
    }
    void removeFile(File* file) //remove file 
    {
        for (int i = 0; i < count; i++)
        {
            if (recentFiles[i] == file)
            {
                for (int j = i; j < count - 1; j++)
                {
                    recentFiles[j] = recentFiles[j + 1];
                }
                recentFiles[count - 1] = nullptr;
                count--;
                break;
            }
        }
    }
    void showRecentFiles() //show recent files
    {
        if (count == 0)
        {
            set_color(31);
            cout << "No recent files." << endl;
            set_color(0);
        }
        else
        {
            set_color(33);
            cout << "Recent Files:" << endl;
            set_color(0);
            for (int i = 0; i < count; i++)
            {
                if (recentFiles[i] != nullptr)
                {
                    set_color(34);
                    cout << i + 1 << ". " << recentFiles[i]->name << endl;
                    set_color(0);
                }
            }
        }
    }
};
//************************************************************************************************************************//
class FileSharing //implmenet using graphs 
{
private:
    bool adj[max_users][max_users]; // adjacency matrix
    string usernames[max_users];
    int userCount;
    int findIndex(string username) //find index of user
    {
        for (int i = 0; i < userCount; i++)
        {
            if (usernames[i] == username)
                return i;
        }
        return -1;
    }
public:
    FileSharing() //non parametrized constructor 
    {
        userCount = 0;
        for (int i = 0; i < max_users; i++)
            for (int j = 0; j < max_users; j++)
                adj[i][j] = false;
    }
    void addUser(string username) //add user  
    {
        if (findIndex(username) == -1 && userCount < max_users)
        {
            usernames[userCount++] = username;
        }
    }
    //share file 
    void share_file(string fromUser, string toUser1, string toUser2, string fileName, File* files[], int fileCount, RecycleBin& bin)
    {
        bool fileExists = false;
        File* orignal_file = nullptr;
        for (int i = 0; i < fileCount; i++)
        {
            if (files[i]->name == fileName)
            {
                fileExists = true;
                orignal_file = files[i];
                break;
            }
        }
        if (!fileExists)
        {
            set_color(31);
            cout << "File '" << fileName << "' does not exist." << endl;
            set_color(0);
            return;
        }
        bool inRecycleBin = false;
        for (int i = 0; i <= bin.top; i++)
        {
            if (bin.stack[i]->name == fileName)
            {
                inRecycleBin = true;
                break;
            }
        }
        if (inRecycleBin)
        {
            set_color(31);
            cout << "File '" << fileName << "' is in the recycle bin so no sharing is possible." << endl;
            set_color(0);
            return;
        }
        int from = findIndex(fromUser);
        int user1 = findIndex(toUser1);
        int user2 = findIndex(toUser2);
        if (from == -1 || user1 == -1 || user2 == -1)
        {
            set_color(31);
            cout << "Invalid users." << endl;
            set_color(0);
            return;
        }
        adj[from][user1] = true;
        adj[from][user2] = true;
        set_color(34);
        cout << fromUser << " shared file '" << fileName << "' with " << toUser1 << " and " << toUser2 << endl;
    }
    void bfs(string startUser) //bfs traversal 
    {
        int start = findIndex(startUser);
        if (start == -1)
        {
            set_color(31);
            cout << "User not found." << endl;
            set_color(0);
            return;
        }
        bool visited[max_users] = {};
        int queue[max_users], front = 0, rear = 0;
        queue[rear++] = start;
        visited[start] = true;
        set_color(35);
        cout << "BFS from " << startUser << ": ";
        while (front < rear)
        {
            int curr = queue[front++];
            cout << usernames[curr] << " ";
            for (int i = 0; i < userCount; i++)
            {
                if (adj[curr][i] && !visited[i])
                {
                    queue[rear++] = i;
                    visited[i] = true;
                }
            }
        }
        cout << endl;
    }
    void dfs_cal(int curr, bool visited[], int userCount)  //DFD helper funtion 
    {
        visited[curr] = true;
        cout << usernames[curr] << " ";
        set_color(0);
        for (int i = 0; i < userCount; i++)
        {
            if (adj[curr][i] && !visited[i])
            {
                dfs_cal(i, visited, userCount);
            }
        }
    }
    void dfs(string startUser)  //DFS 
    {
        int start = findIndex(startUser);
        if (start == -1)
        {
            set_color(31);
            cout << "User not found." << endl;
            set_color(0);
            return;
        }
        bool visited[max_users] = {};
        set_color(35);
        cout << "DFS from " << startUser << ": ";
        dfs_cal(start, visited, userCount);
        cout << endl;
        set_color(0);
    }
};
//**************************************************************************************************************************//
struct SyncTask //implement using queue 
{
    string fileName;
    string operation;
    string content;
    time_t timestamp = 0; // Initialize timestamp
};
//*************************************************************************************************************************//
class CloudSync
{
private:
    SyncTask queue[totalfiles];
    int front, rear;
    bool isOnline; //for network 
public:
    CloudSync()
    {
        front = rear = 0;
        isOnline = true; //default online 
    }
    void enqueueTask(string fileName, string operation, string content) //enqueue the task 
    {
        if (rear < totalfiles)
        {
            SyncTask task;
            task.fileName = fileName;
            task.operation = operation;
            task.content = compressRLE(content); // compress content
            task.timestamp = time(nullptr);
            queue[rear++] = task;
            set_color(32);
            cout << "Sync task for '" << fileName << "' (" << operation << ") enqueued." << endl;
            set_color(0);
        }
        else
        {
            set_color(31);
            cout << "Sync queue full." << endl;
            set_color(0);
        }
    }
    void processQueue(File* files[], int& file_counter, RecycleBin& bin) //process queue 
    {
        if (!isOnline)
        {
            set_color(31);
            cout << "Offline: Sync tasks will process when online." << endl;
            set_color(0);
            return;
        }
        while (front < rear)
        {
            SyncTask task = queue[front++];
            if (task.operation == "upload") //upload to cloud 
            {
                set_color(34);
                cout << "Uploaded '" << task.fileName << "' to cloud." << endl;
                set_color(0);
            }
            else if (task.operation == "delete") //delete form cloud 
            {
                set_color(34);
                cout << "Deleted '" << task.fileName << "' from cloud." << endl;
                set_color(0);
            }
        }
    }
    void toggleNetworkStatus() //on , off status 
    {
        isOnline = !isOnline;
        set_color(33);
        cout << "Network status: " << (isOnline ? "Online" : "Offline") << endl;
        set_color(0);
    }
    bool getNetworkStatus()
    {
        return isOnline;  //get status 
    }
};
//***********************************************************************************************************************//
bool isValidUsername(const string& username) // check user validity 
{
    if (username.empty())
    {
        return false;
    }
    if (username[0] >= '0' && username[0] <= '9')
    {
        return false;
    }
    for (size_t i = 0; i < username.length(); i++)
    {
        if (!(username[i] >= 'A' && username[i] <= 'Z') &&
            !(username[i] >= 'a' && username[i] <= 'z') &&
            !(username[i] >= '0' && username[i] <= '9'))
        {
            return false;
        }
    }
    return true;
}
//**************************************************************************************************************************//
int main()
{
    UserManagement userMgr;
    File* files[max_files];
    int file_counter = 0; //file counter
    RecycleBin bin; //implement using stack and link list 
    RecentFiles recent; //implment using stack and link list
    FileSharing graph; //implment usinh graph
    CloudSync cloudSync; //object of class sync 
    PermissionGraph permGraph; // Added declaration
    FileHashTable hashTable; // Hash table for file metadata
    int choice = -1;
    string input;  //input 
    string username, password, question, answer, content, name, toUser;
    do
    {
        cout << endl;
        set_color(37);
        cout << "**********************************************" << endl;
        cout << "==============================================" << endl;
        set_color(32);
        cout << "*********** FILE MANAGEMENT SYSTEM ***********     " << endl;
        set_color(36);
        cout << "1. Signup" << endl;
        cout << "2. Login" << endl;
        cout << "3. Recover Password" << endl;
        cout << "4. Logout" << endl;
        cout << "5. Create File" << endl;
        cout << "6. Update File " << endl;
        cout << "7. View Latest File Content" << endl;
        cout << "8. View All Versions of File " << endl;
        cout << "9. Rollback File " << endl;
        cout << "10. Delete File " << endl;
        cout << "11. Restore File from Recycle Bin " << endl;
        cout << "12. Show Recycle Bin " << endl;
        cout << "13. Show Recent Files " << endl;
        cout << "14. Share File " << endl;
        cout << "15. BFS Traversal " << endl;
        cout << "16. DFS Traversal " << endl;
        cout << "17. Compress File " << endl;
        cout << "18. File searching" << endl;
        cout << "19. Sync with Cloud" << endl;
        cout << "20. Grant Permissions" << endl;
        cout << "21. Show File Metadata" << endl;
        cout << "0. Exit" << endl;
        set_color(37);
        cout << "**********************************************" << endl;
        cout << "==============================================" << endl;
        cout << endl;
        set_color(0); //reset colour 
        set_color(33); //set colour
        cout << "Enter your choice: ";
        getline(cin, input);
        set_color(0);
        try  //exception handling 
        {
            for (size_t i = 0; i < input.length(); i++)
            {
                if (input[i] < '0' || input[i] > '9')
                {
                    throw "Invalid input so enter a valid digit.";
                }
            }
            choice = stoi(input); //convert string to numeric value 
            if (choice < 0 || choice > 21)
            {
                throw "Choice is not valid Please enter digit.";
            }
        }
        catch (const char* msg)
        {
            set_color(31);
            cout << msg << endl;
            set_color(0);
            continue;
        }
        catch (const exception& e)
        {
            set_color(31);
            cout << "Invalid input so enter a valid digit." << endl;
            set_color(0);
            continue;
        }
        if (choice != 0 && userMgr.isLoggedIn()) //permission check 
        {
            if (!permGraph.hasPermission(userMgr.getCurrentUser(), choice))
            {
                set_color(31);
                cout << "Permission denied for this action." << endl;
                set_color(0);
                continue;
            }
        }
        switch (choice)
        {
            //--------------------------------SignUp--------------------------------
        case 1:
            do
            {
                set_color(35);
                cout << "Enter username: ";
                getline(cin, username);
                set_color(0);
                if (!isValidUsername(username))
                {
                    set_color(31);
                    cout << "Invalid username must contain alphabets and cannot start with a digit." << endl;
                    set_color(0);
                }
            } while (!isValidUsername(username));
            set_color(35);
            cout << "Enter password: ";
            getline(cin, password);
            cout << "Enter security question: ";
            getline(cin, question);
            cout << "Enter answer: ";
            getline(cin, answer);
            {
                cout << "Enter role (0 for Viewer, 1 for Editor, 2 for Admin): ";
                string roleInput;
                getline(cin, roleInput);
                UserRole role;
                try
                {
                    int roleNum = stoi(roleInput);
                    if (roleNum < 0 || roleNum > 2)
                        throw "Invalid role.";
                    role = static_cast<UserRole>(roleNum);
                }
                catch (...)
                {
                    set_color(31);
                    cout << "Invalid role default is viewer" << endl;
                    set_color(0);
                    role = UserRole::VIEWER;
                }
                userMgr.signup(username, password, question, answer, role);
                graph.addUser(username);
                permGraph.addUser(username, role); // Add to permission graph
            }
            set_color(0);
            break;
            //--------------------------------Login--------------------------------
        case 2:
            do
            {
                set_color(35);
                cout << "Enter username: ";
                getline(cin, username);
                if (!isValidUsername(username))
                {
                    set_color(31);
                    cout << "Invalid username. " << endl;
                    set_color(0);
                }
            } while (!isValidUsername(username));
            set_color(35);
            cout << "Enter password: ";
            getline(cin, password);
            if (userMgr.login(username, password))
            {
                UserRole role = userMgr.getUserRole(username);
                set_color(32);
                cout << "User role: " << (role == UserRole::ADMIN ? "Admin" : role == UserRole::EDITOR ? "Editor" : "Viewer") << endl;
                set_color(0);
            }
            set_color(0);
            break;
            //--------------------------------Password Recovery--------------------------------
        case 3:
            if (!userMgr.isLoggedIn())
            {
                set_color(31);
                cout << "User not logged in so no recovery of password is possible." << endl;
                set_color(0);
            }
            else
            {
                set_color(35);
                cout << "Enter username: ";
                getline(cin, username);
                userMgr.showSecurityQuestion(username);
                set_color(35);
                cout << "Enter answer: ";
                getline(cin, answer);
                set_color(35);
                cout << "Enter new password: ";
                getline(cin, password);
                if (!userMgr.recoverPassword(username, answer, password))
                {
                    set_color(31);
                    cout << "Username does not match any logged-in user or invalid security answer." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------Logout--------------------------------
        case 4:
            userMgr.logout();
            break;
            //--------------------------------Create File--------------------------------
        case 5:
            if (userMgr.isLoggedIn() && file_counter < max_files)
            {
                set_color(35);
                cout << "Enter file name: ";
                getline(cin, name);
                set_color(35);
                cout << "Enter content: ";
                getline(cin, content);
                set_color(35);
                cout << "Enter file type (e.g., txt, doc): ";
                string fileType;
                getline(cin, fileType);
                int fileSize = content.length(); //size based on length of content
                // Create file and add to hash table
                File* newFile = new File(name, content);
                if (hashTable.insert(name, fileSize, fileType, userMgr.getCurrentUser(), newFile))
                {
                    files[file_counter] = newFile;
                    recent.addFile(files[file_counter]);
                    file_counter++;
                    cloudSync.enqueueTask(name, "upload", content);
                    set_color(32);
                    cout << "File created." << endl;
                    set_color(0);
                }
            }
            else
            {
                set_color(31);
                cout << "Login required or file limit reached." << endl;
            }
            break;
            //--------------------------------Update File--------------------------------
        case 6:
            set_color(35);
            cout << "Enter file name to update: ";
            getline(cin, name);
            {
                File* file = hashTable.search(name);
                bool inBin = false;
                if (file)
                {
                    cout << "Enter new content: ";
                    getline(cin, content);
                    file->updateContent(content); //update content
                    recent.addFile(file); //add to that file 
                    cloudSync.enqueueTask(name, "upload", content); //to sync queue 
                    hashTable.remove(name);  // Update metadata in hash table
                    hashTable.insert(name, content.length(), "txt", userMgr.getCurrentUser(), file);
                }
                for (int i = 0; i <= bin.top; i++)
                {
                    if (bin.stack[i]->name == name)
                    {
                        inBin = true;
                        break;
                    }
                }
                if (!file)
                {
                    set_color(31);
                    cout << "File does not exist or may be in recycle bin." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------View Latest Content--------------------------------
        case 7:
            set_color(35);
            cout << "Enter file name: ";
            getline(cin, name);
            {
                File* file = hashTable.search(name);
                bool inBin = false;
                if (file)
                {
                    file->showLatestContent();
                }
                for (int i = 0; i <= bin.top; i++)
                {
                    if (bin.stack[i]->name == name)
                    {
                        inBin = true;
                        break;
                    }
                }
                if (!file)
                {
                    set_color(31);
                    cout << "File does not exist or may be in recycle bin." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------Show All Version--------------------------------
        case 8:
            set_color(35);
            cout << "Enter file name: ";
            getline(cin, name);
            {
                File* file = hashTable.search(name);
                bool inRecycleBin = false;
                if (file)
                {
                    file->showAllVersions();
                }
                for (int i = 0; i <= bin.top; i++)
                {
                    if (bin.stack[i]->name == name)
                    {
                        inRecycleBin = true;
                        break;
                    }
                }
                if (!file)
                {
                    set_color(31);
                    cout << "File does not exist or may be in recycle bin." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------Roll Back--------------------------------
        case 9:
            set_color(35);
            cout << "Enter file name: ";
            getline(cin, name);
            {
                File* file = hashTable.search(name);
                bool inRecycleBin = false;
                if (file)
                {
                    file->rollbackVersion();
                }
                for (int i = 0; i <= bin.top; i++)
                {
                    if (bin.stack[i]->name == name)
                    {
                        inRecycleBin = true;
                        break;
                    }
                }
                if (!file)
                {
                    set_color(31);
                    cout << "File does not exist or may be in recycle bin." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------Delete File--------------------------------
        case 10:
            set_color(35);
            cout << "Enter file name to delete: ";
            getline(cin, name);
            {
                File* file = hashTable.search(name);
                if (file)
                {
                    for (int i = 0; i < file_counter; i++)
                    {
                        if (files[i]->name == name)
                        {
                            bin.push(files[i]);
                            recent.removeFile(files[i]);
                            hashTable.remove(name);
                            for (int j = i; j < file_counter - 1; j++)
                            {
                                files[j] = files[j + 1]; //left movement
                            }
                            file_counter--;
                            cloudSync.enqueueTask(name, "delete", "");
                            set_color(32);
                            cout << "File moved to Recycle Bin." << endl;
                            set_color(0);
                            break;
                        }
                    }
                }
                else
                {
                    set_color(31);
                    cout << "File not created" << endl;
                }
            }
            break;
            //--------------------------------Restore File--------------------------------
        case 11:
            if (!bin.isEmpty())
            {
                File* restored = bin.pop();
                if (file_counter < max_files)
                {
                    files[file_counter] = restored;
                    hashTable.insert(restored->name, restored->head_version->content.length(),
                        "txt", userMgr.getCurrentUser(), restored);
                    recent.addFile(restored);
                    file_counter++;
                    set_color(32);
                    cout << "File restored." << endl;
                }
                else
                {
                    set_color(31);
                    cout << "Cannot restore. File limit reached." << endl;
                }
            }
            else
            {
                set_color(31);
                cout << "Recycle Bin is empty." << endl;
                set_color(0);
            }
            break;
            //--------------------------------Show Recycle Bin--------------------------------
        case 12:
            bin.display();
            break;
            //--------------------------------Show Recent Files--------------------------------
        case 13:
            recent.showRecentFiles();
            break;
            //--------------------------------File Sharing--------------------------------
        case 14:
            if (!userMgr.isLoggedIn())
            {
                set_color(31);
                cout << "User not logged in so no sharing." << endl;
            }
            else
            {
                set_color(35);
                cout << "Enter your username: ";
                getline(cin, username);
                cout << "Enter first username: ";
                getline(cin, toUser);
                string toUser2;
                cout << "Enter second username: ";
                getline(cin, toUser2);
                cout << "Enter file name to share: ";
                getline(cin, name);
                graph.share_file(username, toUser, toUser2, name, files, file_counter, bin);
                set_color(0);
            }
            break;
            //--------------------------------BFS Traversal--------------------------------
        case 15:
            set_color(35);
            cout << "Enter starting username for BFS: ";
            getline(cin, username);
            graph.bfs(username);
            break;
            //--------------------------------DFS Traversal--------------------------------
        case 16:
            set_color(35);
            cout << "Enter starting username for DFS: ";
            getline(cin, username);
            graph.dfs(username);
            break;
            //--------------------------------Compress--------------------------------
        case 17:
            set_color(35);
            cout << "Enter file name to compress: ";
            getline(cin, name);
            {
                File* file = hashTable.search(name);
                bool inBin = false;
                if (file)
                {
                    set_color(35);
                    cout << "Compressed content of '" << name << "': " << file->head_version->content << endl;
                    set_color(32);
                    cout << "File compression displayed successfully." << endl;
                    set_color(0);
                }
                for (int i = 0; i <= bin.top; i++)
                {
                    if (bin.stack[i] && bin.stack[i]->name == name)
                    {
                        inBin = true;
                        break;
                    }
                }
                if (!file)
                {
                    set_color(31);
                    cout << "File does not exist or may be in recycle bin." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------File Searching--------------------------------
        case 18:
            if (!userMgr.isLoggedIn())
            {
                set_color(31);
                cout << "Login required." << endl;
                set_color(0);
            }
            else
            {
                set_color(35);
                cout << "Enter file name to search: ";
                getline(cin, name);
                File* file = hashTable.search(name);
                if (file)
                {
                    set_color(32);
                    cout << "File found: " << file->name << endl;
                    file->showLatestContent();
                    set_color(0);
                }
                else
                {
                    set_color(31);
                    cout << "File '" << name << "' not found." << endl;
                    set_color(0);
                }
            }
            break;
            //--------------------------------Cloud Synchronization--------------------------------
        case 19:
            if (!userMgr.isLoggedIn())
            {
                set_color(31);
                cout << "Login required." << endl;
                set_color(0);
            }
            else
            {
                set_color(35);
                cout << "1. Trigger Sync" << endl;
                cout << "2Toggle Network Status" << endl;
                cout << "Enter choice : ";
                string syncChoice;
                getline(cin, syncChoice);
                if (syncChoice == "1")
                {
                    cloudSync.processQueue(files, file_counter, bin);
                }
                else if (syncChoice == "2")
                {
                    cloudSync.toggleNetworkStatus();
                }
                else
                {
                    set_color(31);
                    cout << "Invalid choice." << endl;
                }
                set_color(0);
            }
            break;
            //--------------------------------Grant Permission--------------------------------
        case 20:
            if (!userMgr.isLoggedIn())
            {
                set_color(31);
                cout << "Login required." << endl;
                set_color(0);
            }
            else
            {
                set_color(35);
                cout << "Enter admin username: ";
                getline(cin, username);
                cout << "Enter target username: ";
                string targetUser;
                getline(cin, targetUser);
                cout << "Grant read permission? (y/n): ";
                string readInput;
                getline(cin, readInput);
                bool read = (readInput == "y" || readInput == "Y");
                cout << "Grant write permission? (y/n): ";
                string writeInput;
                getline(cin, writeInput);
                bool write = (writeInput == "y" || writeInput == "Y");
                cout << "Grant execute permission? (y/n): ";
                string execInput;
                getline(cin, execInput);
                bool execute = (execInput == "y" || execInput == "Y");
                permGraph.grantPermission(username, targetUser, read, write, execute);
                set_color(0);
            }
            break;
            //--------------------------------Show File Metadata--------------------------------
        case 21:
            if (!userMgr.isLoggedIn())
            {
                set_color(31);
                cout << "Login required." << endl;
                set_color(0);
            }
            else
            {
                hashTable.display();
            }
            break;
            //--------------------------------Program Ends--------------------------------
        case 0:
            set_color(33);
            cout << "exit program so bye " << endl;
            break;
        default:
            set_color(31);
            cout << "Invalid choice " << endl;
            set_color(0);
        }
    } while (choice != 0);
    system("pause");
    return 0;
}