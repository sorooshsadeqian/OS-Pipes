#include <bits/stdc++.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

void get_weight_vectors(const string& filename, vector<float>& beta1,vector<float>& beta2,vector<float>& bias);
void calculate_score (vector<float>& beta1, vector<float>& beta2, vector<float>& bias, const string& dataset_name, vector<int>& results);
void read_csv(const string& name, vector<vector< float > >& data_list);
void send_results_to_voter(const string& pipe_name, vector <int> results);

int main (int argc, char* argv[]) {
    char buf[4096];
    int size_read = read(atoi(argv[1]), buf, 1000);
    string buf_string(buf, size_read);
    istringstream ss (buf_string);
    string token;
    vector<string> args;
    while (getline(ss, token, ',')) {
        args.push_back(token);
    }
    int a = atoi(&args[0][args[0].size() - 1]);
    int n_pipe = mkfifo(args[0].c_str(), 0666);
    if (n_pipe < 0) {
        cout << "Pipe creation failed." << endl;
    }
    vector<float> beta1, beta2, bias;
    get_weight_vectors(args[1], beta1, beta2, bias);

    vector<int> results;
    calculate_score(beta1, beta2, bias, args[2], results);
    send_results_to_voter(args[0], results);
    exit(0);
}
void send_results_to_voter(const string& pipe_name, vector <int> results) {
    string res = "";
    for (int i = 0 ; i < results.size() ; i++) {
        res += to_string(results[i]);
        if (i != results.size() - 1)
            res += ",";
    }
    int fd = open(pipe_name.c_str(), O_WRONLY);
    int write_size = write(fd, res.c_str(), strlen(res.c_str()));
    close(fd);
}

void calculate_score (vector<float>& beta1, vector<float>& beta2, vector<float>& bias, const string& dataset_name, vector<int>& results) {
    vector < vector< float> > dataset;
    read_csv(dataset_name, dataset);
    int x;
    for (int i = 0 ; i < dataset.size() ; i++) {
        float max_score = -1 * numeric_limits<float>::infinity();
        int max_index = 0;
        for (int j = 0 ; j < beta1.size() ; j++) {
            float score = beta1[j] * dataset[i][0] + beta2[j] * dataset[i][1] + bias[j];

            if (score > max_score) {
                max_score = score;
                max_index = j;
            }
        }
        results.push_back(max_index);
    }
}

void get_weight_vectors(const string& filename, vector<float>& beta1,vector<float>& beta2,vector<float>& bias) {
    vector< vector <float> > temp_data;
    read_csv(filename, temp_data);
    for (int i = 0 ; i < temp_data.size() ; i++) {
        beta1.push_back(temp_data[i][0]);
        beta2.push_back(temp_data[i][1]);
        bias.push_back(temp_data[i][2]);
    }
}

void read_csv(const string& name, vector<vector< float > >& data_list) {
    ifstream file(name);
    string line = "";
    getline(file, line);
    while (getline(file, line))
    { 
        vector<float> a;
        istringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            a.push_back(stof(token));
        }
        data_list.push_back(a);
    }
    file.close();
}