#include <bits/stdc++.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// #define LINEAR_CLASSIFIERS_PIPE_NAME "./Assets/named_pipes/lcp_"

using namespace std;

int main (int argc, char* argv[]) {
    int max_class_num = 0;
    int fd = open(argv[1], O_RDONLY);
    char buf[4096];
    int read_size = read(fd, buf, 100);
    close(fd);

    string buf_string(buf, read_size);
    istringstream ss(buf_string);
    string token;
    vector<string> args;
    while (getline(ss, token, ',')) {

        args.push_back(token);
    }
    ////// GETTING THE RESULTS FROM THE LCs
    vector<string> results;
    int lc_count = stoi(args[1]);

    for (int i = 0 ; i < lc_count ; i++) {
        string pipe_name = args[0] + to_string(i);
        int fd = open(pipe_name.c_str(), O_RDONLY);
        while ( fd < 0 ) {
            fd = open(pipe_name.c_str(), O_RDONLY);
        }
        char* temp_res = new char[4096];
        int r_size = read(fd, temp_res, 4090);
        string temp_res_string (temp_res, r_size);
        results.push_back(temp_res_string);
        close(fd);
    }

    vector <vector <int> > res_int;
    for (int i = 0 ; i < results.size() ; i++ ) {
        vector<int> cur_lc_res;
        istringstream ss(results[i]);
        string token;
        while (getline(ss, token, ',')) {

            int val_to_push = stoi(token);
            cur_lc_res.push_back(val_to_push);
            if (val_to_push > max_class_num) {
                max_class_num = val_to_push;
            }
        }
        res_int.push_back(cur_lc_res);
    }
    //// WE HAVE ALL THE RESULTS IN res_int

    vector<int> final_results;
    
    for (int i = 0 ; i < res_int[0].size() ; i++) {
        vector <int> temporary_res(max_class_num + 1, 0);
        for (int j = 0 ; j < res_int.size() ; j++) { 
            temporary_res[res_int[j][i]]++;
        }
        int max = 0, index = 0;
        for (int k = 0 ; k < temporary_res.size() ; k++) {
            if (temporary_res[k] > max) {
                max = temporary_res[k];
                index = k;
            }
        }
        final_results.push_back(index);
    }



    buf[0] = '\0';
    string line = "";
    for (int i = 0 ; i < final_results.size() ; i++) {
        line += to_string(final_results[i]);
        if (i != final_results.size() - 1)
            line += ",";
    }
    fd = open(argv[1], O_RDWR);
    int write_size = write(fd, line.c_str(), line.size());
    close(fd);
    exit(0);
}