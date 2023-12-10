#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;

const int chunkSize = 10000000; // 10^7 numbers

template <typename T>
class priority_queue{
private:
    int sz, cap, isMax;
    T* arr = nullptr;

    void swap(T* a, T* b){
        T temp = *a;
        *a = *b;
        *b = temp;
    }

    void changeCap(){
        int oldCap = cap;
        int newCap = 0;
        if (oldCap == 0)
            newCap = 1;
        else
            newCap = 2 * oldCap;
        T* temp = new T[newCap];
        for (int i = 0; i < cap; i++){
            temp[i] = arr[i];
        }
        cap = newCap;
        if(arr != nullptr){
            delete[] arr;
        }
        arr = temp;
    }

    void Push_Heapify(int node){
        while(node > 0){
            int p = (node - 1) / 2;
            int child;
            if((isMax && arr[node] > arr[p]) || (!isMax && arr[node] < arr[p])){
                swap(&arr[p], &arr[node]);
                node = p;
            } 
            else
                break;
        }
    }

    void Pop_Heapify(int node){
        int index = node;
        int lc = 2 * node + 1;
        int rc = 2 * node + 2;
        int child;
        if((lc < sz) && ((isMax && arr[lc] > arr[index]) || (!isMax && arr[lc] < arr[index]))){
            index = lc;
            child = index;
        }
        if((rc < sz) && ((isMax && arr[rc] > arr[index]) || (!isMax && arr[rc] < arr[index]))){
            int val;
            index = rc;
            child = index;
        }
        if(index != node){
            swap(&arr[index], &arr[node]);
            Pop_Heapify(index);
        }
    }

public:
    priority_queue(int isMaxHeap){
        isMax = isMaxHeap;
        sz = 0;
        cap = 0;
    }

    int size(){
        int currSize = sz;
        return currSize;
    }

    bool empty(){
        return sz == 0;
    }

    void push(T el){
        if (sz == cap){
            changeCap();
        }
        arr[sz] = el;
        sz++;
        Push_Heapify(sz - 1);
    }

    T top(){
        if (!empty()){
            return arr[0];
        }
        return T();
    }

    void pop(){
        if (!empty()){
            arr[0] = arr[sz - 1];
            sz--;
            Pop_Heapify(0);
        }
    }
};

// Function to merge k sorted files into a single sorted file.
void mergeFiles(const vector<string>& tempFiles, const string& outputFile){
    vector<ifstream> input(tempFiles.size());
    size_t i=0;
    size_t tempFilesSize = tempFiles.size();
    while(i<tempFilesSize){
        input[i].open(tempFiles[i]);
        i++;
    }
    ofstream output(outputFile);
    priority_queue<pair<long long int, long long int>> minHeap(0);
    vector<pair<long long int, long long int>> buffer; // Buffer to store sorted data before writing to the output.

    int fileCount = 0;
    int totalNumbers = 0;

    // Populate the min-heap with the values from each temp file.
    for(size_t i = 0; i < tempFiles.size(); ++i){
        long long int num;
        if(input[i] >> num){
            minHeap.push(make_pair(num, i));
        }
    }

    while(!minHeap.empty()){
        pair<long long int, long long int> minVal = minHeap.top();
        minHeap.pop();

        // Store the minimum value in the buffer.
        buffer.push_back(minVal);

        // Read the next number from the same temp file and add it to the min-heap.
        long long int num;
        if(input[minVal.second] >> num){
            minHeap.push(make_pair(num, minVal.second));
        }

        if(buffer.size() >= chunkSize){
            //write buffer to output file
            for(const pair<long long int, long long int>& data : buffer){
                output << data.first << "\n";
            }
            buffer.clear();
        }

        totalNumbers++;

        // Check if the current file is completed
        if(input[minVal.second].eof()){
            input[minVal.second].close();
            fileCount++;
        }
    }

    // Write any remaining data from the buffer to the output file.
    for(const pair<long long int, long long int>& data : buffer){
        output << data.first << "\n";
    }
    buffer.clear();

    output.close();
    long long int nums = min(chunkSize,totalNumbers);
    cout << "Number of integers in a temporary file: " << nums << endl;
    cout << "Number of temporary files created: " << fileCount << endl;
    //cout << "Total numbers written to the output file: " << totalNumbers << endl;
}

int readChunk(ifstream &ipFile, vector<long long int> &chunk){
    long long int num;
    while(chunk.size() < chunkSize && (ipFile >> num)){
        chunk.push_back(num);
    }
    if(chunk.empty()){
        return 0;
    }
    return chunk.size();
}

void writeToTempFile(string tempFileName, vector<long long int> &chunk){
    ofstream tempFile(tempFileName);
    for(long long int value : chunk){
        tempFile << value << "\n";
    }
    tempFile.close();
}

int main(int argc, char* argv[]){
    auto start = chrono::high_resolution_clock::now();
    if(argc != 3){
        cout << "Usage: " << argv[0] << " <inputFilePath> <outputFilePath>" << endl;
        return 1;
    }

    vector<long long int> chunk;
    vector<string> tempFiles;
    const string outputFilePath = argv[2];
    const string inputFilePath = argv[1];
    ifstream inputFile(inputFilePath);

    if(!inputFile){
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }

    while(1){
        // read chunk from input file
        if(!readChunk(inputFile, chunk))
            break;

        // Sort the chunk
        sort(chunk.begin(), chunk.end());

        // write chunk to temp file
        string tempFileName = "temp" + to_string(tempFiles.size()) + ".txt";
        writeToTempFile(tempFileName, chunk);
        tempFiles.push_back(tempFileName);
        chunk.clear();
    }
    inputFile.close();

    // Merge temp files to create the output file
    mergeFiles(tempFiles, outputFilePath);

    // Delete temp files
    for(const string& tempFile : tempFiles){
        remove(tempFile.c_str());
    }

    // Calculate execution time
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    double seconds = static_cast<double>(duration.count()) / 1e6;

    cout << "Time taken: " << fixed << setprecision(2) << seconds << " seconds" << endl;

    return 0;
}
