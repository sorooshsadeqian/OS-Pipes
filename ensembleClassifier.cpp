#include <bits/stdc++.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LINEAR_CLASSIFIERS_PIPE_NAME "./Assets/named_pipes/lcp_"
#define VOTER_CLASSIFIER_PIPE_NAME "./Assets/named_pipes/vp"
#define LINEAR_CLASSIFIER_OBJECT_NAME "LinearClassifier.out"
#define VOTER_OBJECT_NAME "Voter.out"
#define DATASET_FILE_NAME "./Assets/validation/dataset.csv"

using namespace std;

void get_filenames(const string& name, vector<string>& v);
void read_csv(const string& name, vector<int>& data_list);
void create_pipes(vector<int*>& pipes, int size);
void create_linear_classifier_processes(int count, vector<pid_t>& children, vector<int*> pipes);
void pass_info_to_linear_classifiers(vector<string> weight_vector_filenames, vector<int*> pipes);
pid_t create_voter();
void pass_info_to_voter(int lc_count);
float calculate_accuracy(vector <int> results, string path);
void get_results_from_voter(vector <int>& results);

int main (int argc, char* argv[]) {

    if (argc != 3) {
        cout << "At least 2 arguments should be given! The weight_vectors and the validation directory" << endl;
        return 1;
    }
    vector<string> weight_vector_filenames;
    get_filenames(argv[2], weight_vector_filenames);
    sort(weight_vector_filenames.begin(), weight_vector_filenames.end());
    int lc_count = weight_vector_filenames.size();
    string classifier_address(argv[2]);
    for (int i = 0 ; i < weight_vector_filenames.size() ; i++) {
        weight_vector_filenames[i] = classifier_address + "/" + weight_vector_filenames[i];
    }
   
    vector <int*> pipes;
    create_pipes(pipes, lc_count);

    for (int i = 0 ; i < lc_count ; i++) {
        string pipename = LINEAR_CLASSIFIERS_PIPE_NAME;
        pipename += to_string(i);
        remove(pipename.c_str());
    }
    remove(VOTER_CLASSIFIER_PIPE_NAME);

    vector <pid_t> linear_classifiers;
    create_linear_classifier_processes(lc_count, linear_classifiers, pipes);
    pass_info_to_linear_classifiers(weight_vector_filenames, pipes);

    open(VOTER_CLASSIFIER_PIPE_NAME, O_CREAT, S_IRUSR | S_IWUSR);
    mkfifo(VOTER_CLASSIFIER_PIPE_NAME, 0666);
    pid_t voter = create_voter();
    pass_info_to_voter(lc_count);
     for (int i = 0 ; i < lc_count ; i++) {
        wait(NULL);
    }
    wait(NULL);
    vector<int> results;
    get_results_from_voter(results);
    string validation_path (argv[1]);
    float accuracy = calculate_accuracy(results, validation_path);
    cout << fixed << setprecision(2) << "Accuracy: " << accuracy << "%" << endl;
    for (int i = 0 ; i < lc_count ; i++) {
        string pipename = LINEAR_CLASSIFIERS_PIPE_NAME;
        pipename += to_string(i);
        remove(pipename.c_str());
    }
    remove(VOTER_CLASSIFIER_PIPE_NAME);
    
    
    return 0;
}

void get_results_from_voter(vector <int>& results) {
    int fd = open(VOTER_CLASSIFIER_PIPE_NAME, O_RDWR);
    char buf [4096];
    int read_size = read(fd, buf, 4090);
    string buf_str(buf, read_size);
    istringstream ss(buf_str);
    string token;
    while (getline(ss, token, ',')) {
        results.push_back(atoi(token.c_str()));
    }
}


float calculate_accuracy(vector <int> results, string path) {

    path = "./" + path + "/labels.csv";

    vector<int> labels;
    read_csv(path, labels);

    
    int correct_results = 0;
    for (int i = 0 ; i < results.size() ; i++) {
        if (results[i] == labels[i]) {
            correct_results++;
        }
    }

    float accuracy = (float) correct_results / results.size();
    return accuracy * 100;
}

void pass_info_to_voter(int lc_count) {
    int fd = open(VOTER_CLASSIFIER_PIPE_NAME, O_WRONLY);
    string info = LINEAR_CLASSIFIERS_PIPE_NAME;
    info += ",";
    info += to_string(lc_count);
    int write_size = write(fd, info.c_str(), info.size() + 1);
    close(fd);
}


pid_t create_voter() {
    pid_t voter = fork();
    if (voter == 0) {
        char* argv[3];
        string process_name = VOTER_OBJECT_NAME;
        string pipe_name = VOTER_CLASSIFIER_PIPE_NAME;
        argv[0] = (char*) process_name.c_str();
        argv[1] = (char*) pipe_name.c_str();
        argv[2] = NULL;
        int a = execv(argv[0], argv);
    }
    else {
        return voter;
    }
}


void pass_info_to_linear_classifiers(vector<string> weight_vector_filenames, vector<int*> pipes) {
    for (int i = 0 ; i < pipes.size() ; i++) {
        string info_to_pass = "\0"; 
        info_to_pass += LINEAR_CLASSIFIERS_PIPE_NAME + to_string(i) + "," + weight_vector_filenames[i] + "," + DATASET_FILE_NAME + ",";
        write(pipes[i][1], info_to_pass.c_str(), strlen(info_to_pass.c_str()));
    }
}


void create_linear_classifier_processes(int count, vector<pid_t>& children, vector<int*> pipes) {
    for (int i = 0 ; i < count ; i++) {
        pid_t child_id = fork();
        if (child_id == 0) {
            char* argv[2];
            string process_name = LINEAR_CLASSIFIER_OBJECT_NAME;
            argv[0] = (char*) process_name.c_str();
            argv[1] = (char*) to_string(pipes[i][0]).c_str();
            argv[2] = NULL;
            execv(argv[0], argv);
        }
        else {
            children.push_back(child_id);
        }
    }
}


void create_pipes(vector<int*>& pipes, int size) {
    for (int i = 0 ; i < size ; i++) {
        pipes.push_back(new int[2]);
        pipe(pipes[i]);
    }
}


void get_filenames(const string& name, vector<string>& v) {
    DIR* current_dir = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(current_dir)) != NULL) {
        if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
            v.push_back(dp->d_name);
    }
    closedir(current_dir);
}


void read_csv(const string& name, vector<int>& data_list) {
    ifstream file(name);
    string line = "";
    getline(file, line);
    while (getline(file, line))
    { 
        istringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            data_list.push_back(stoi(token));
        }
    }
    file.close();
}