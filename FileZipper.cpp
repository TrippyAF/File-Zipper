#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <cstdint>

using namespace std;

struct Node {
    int symbol; // byte value (0-255), -1 if internal node
    uint64_t freq;
    Node* left;
    Node* right;
    Node(int s, uint64_t f, Node* l=nullptr, Node* r=nullptr)
        : symbol(s), freq(f), left(l), right(r) {}
};

struct CompareNode {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// Bit writer for compression
class BitWriter {
    ofstream& out;
    uint8_t buffer;
    int bitCount;
public:
    BitWriter(ofstream& o) : out(o), buffer(0), bitCount(0) {}
    void writeBit(int bit) {
        buffer = (buffer << 1) | bit;
        bitCount++;
        if (bitCount == 8) flush();
    }
    void writeCode(const string& code) {
        for (char c : code) writeBit(c == '1');
    }
    void flush() {
        if (bitCount > 0) {
            buffer <<= (8 - bitCount); // pad remaining bits
            out.put((char)buffer);
            buffer = 0;
            bitCount = 0;
        }
    }
};

// Bit reader for decompression
class BitReader {
    ifstream& in;
    uint8_t buffer;
    int bitCount;
public:
    BitReader(ifstream& i) : in(i), buffer(0), bitCount(0) {}
    int readBit() {
        if (bitCount == 0) {
            int c = in.get();
            if (c == EOF) return -1;
            buffer = (uint8_t)c;
            bitCount = 8;
        }
        int bit = (buffer >> 7) & 1;
        buffer <<= 1;
        bitCount--;
        return bit;
    }
};

// Build Huffman codes recursively
void buildCodes(Node* root, string code, vector<string>& table) {
    if (!root) return;
    if (!root->left && !root->right) {
        table[root->symbol] = code.empty() ? "0" : code;
        return;
    }
    buildCodes(root->left, code + "0", table);
    buildCodes(root->right, code + "1", table);
}

// Compress function
bool compress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in) { cerr << "Error: cannot open input file\n"; return false; }

    // Count frequencies
    vector<uint64_t> freq(256, 0);
    uint64_t totalBytes = 0;
    char c;
    while (in.get(c)) {
        freq[(uint8_t)c]++;
        totalBytes++;
    }
    in.clear(); in.seekg(0);

    // Build Huffman tree
    priority_queue<Node*, vector<Node*>, CompareNode> pq;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0) pq.push(new Node(i, freq[i]));
    if (pq.empty()) { cerr << "Empty file.\n"; return false; }
    while (pq.size() > 1) {
        Node* a = pq.top(); pq.pop();
        Node* b = pq.top(); pq.pop();
        pq.push(new Node(-1, a->freq + b->freq, a, b));
    }
    Node* root = pq.top();

    // Build code table
    vector<string> codes(256);
    buildCodes(root, "", codes);

    // Write header
    ofstream out(outputFile, ios::binary);
    out.write((char*)&totalBytes, sizeof(totalBytes));
    int unique = 0;
    for (auto f : freq) if (f > 0) unique++;
    out.write((char*)&unique, sizeof(unique));
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            uint8_t sym = i;
            out.write((char*)&sym, sizeof(sym));
            out.write((char*)&freq[i], sizeof(uint64_t));
        }
    }

    // Encode data
    BitWriter bw(out);
    while (in.get(c)) {
        bw.writeCode(codes[(uint8_t)c]);
    }
    bw.flush();
    cout << "Compressed " << totalBytes << " bytes -> " << out.tellp() << " bytes\n";
    return true;
}

// Decompress function
bool decompress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in) { cerr << "Error: cannot open input file\n"; return false; }

    uint64_t originalSize;
    in.read((char*)&originalSize, sizeof(originalSize));
    int unique;
    in.read((char*)&unique, sizeof(unique));

    vector<uint64_t> freq(256, 0);
    for (int i = 0; i < unique; i++) {
        uint8_t sym; uint64_t f;
        in.read((char*)&sym, sizeof(sym));
        in.read((char*)&f, sizeof(f));
        freq[sym] = f;
    }

    // Build Huffman tree
    priority_queue<Node*, vector<Node*>, CompareNode> pq;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0) pq.push(new Node(i, freq[i]));
    while (pq.size() > 1) {
        Node* a = pq.top(); pq.pop();
        Node* b = pq.top(); pq.pop();
        pq.push(new Node(-1, a->freq + b->freq, a, b));
    }
    Node* root = pq.top();

    ofstream out(outputFile, ios::binary);
    BitReader br(in);
    uint64_t written = 0;
    Node* cur = root;

    while (written < originalSize) {
        int bit = br.readBit();
        cur = (bit == 0) ? cur->left : cur->right;
        if (!cur->left && !cur->right) {
            out.put((char)cur->symbol);
            written++;
            cur = root;
        }
    }
    cout << "Decompressed " << originalSize << " bytes.\n";
    return true;
}

// CLI
int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage:\n"
             << "  " << argv[0] << " compress <input> <output>\n"
             << "  " << argv[0] << " decompress <input> <output>\n";
        return 1;
    }
    string mode = argv[1];
    if (mode == "compress") {
        return compress(argv[2], argv[3]) ? 0 : 1;
    } else if (mode == "decompress") {
        return decompress(argv[2], argv[3]) ? 0 : 1;
    } else {
        cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }
}
