#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>
using namespace std;
using namespace std::chrono;

mutex cout_mutex;

void multiply_worker(const vector<vector<int>>& A, const vector<vector<int>>& B,
    vector<vector<int>>& C, int start_row, int end_row, bool print_output) {
    int m = A[0].size();
    int k = B[0].size();
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < k; ++j) {
            int sum = 0;
            for (int l = 0; l < m; ++l) {
                sum += A[i][l] * B[l][j];
            }
            C[i][j] = sum;
            if (print_output) {
                this_thread::sleep_for(chrono::microseconds(rand() % 5000));
                lock_guard<mutex> lock(cout_mutex);
                cout << "[" << i << "," << j << "]=" << C[i][j] << " (Thread: " << this_thread::get_id() << ")" << endl;
            }
        }
    }
}

long long parallel_multiply(const vector<vector<int>>& A, const vector<vector<int>>& B,
    vector<vector<int>>& C, int num_threads, bool print_output = false) {
    int n = A.size();
    vector<thread> threads;
    int chunk_size = n / num_threads;
    int remainder = n % num_threads;
    auto start_time = high_resolution_clock::now();
    int current_row = 0;
    for (int i = 0; i < num_threads; ++i) {
        int end_row = current_row + chunk_size + (i < remainder ? 1 : 0);
        if (current_row < end_row) {
            threads.emplace_back(multiply_worker, ref(A), ref(B), ref(C), current_row, end_row, print_output);
        }
        current_row = end_row;
    }
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    auto end_time = high_resolution_clock::now();
    return duration_cast<milliseconds>(end_time - start_time).count();
}

int main() {
    cout << "--- Task 1.1: Parallelism Demonstration ---" << endl;
    int n1 = 5, m1 = 5, k1 = 5;
    vector<vector<int>> A1(n1, vector<int>(m1, 2));
    vector<vector<int>> B1(m1, vector<int>(k1, 3));
    vector<vector<int>> C1(n1, vector<int>(k1, 0));
    parallel_multiply(A1, B1, C1, 5, true);

    cout << "\n--- Task 1.2: Performance Benchmark ---" << endl;
    int n2 = 1000, m2 = 1000, k2 = 1000;
    vector<vector<int>> A2(n2, vector<int>(m2, 1));
    vector<vector<int>> B2(m2, vector<int>(k2, 1));
    vector<vector<int>> C2(n2, vector<int>(k2, 0));

    vector<int> thread_counts = { 1, 2, 4, 5, 6, 8, 16 };
    cout << "Matrix size: " << n2 << "x" << m2 << " * " << m2 << "x" << k2 << endl;

    for (int t : thread_counts) {
        for (auto& row : C2) fill(row.begin(), row.end(), 0);
        long long time_taken = parallel_multiply(A2, B2, C2, t, false);
        cout << "Threads: " << t << " | Execution time: " << time_taken << " ms" << endl;
    }
    return 0;
}
