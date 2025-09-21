# Huffman File Zipper 🔐📦

A simple **file compression and decompression tool** written in **C++** using the **Huffman Coding algorithm**.  
This program demonstrates how lossless compression works by encoding frequently occurring bytes with shorter bit patterns.

---

## ✨ Features
- ✅ Compress any file using **Huffman coding**
- ✅ Decompress back to the **original file** (lossless)
- ✅ Works with **binary and text files**
- ✅ Handles large files efficiently (stream-based, not fully loaded into memory)
- ✅ Beginner-friendly, clean code

---

## 📖 How It Works
1. Counts the **frequency of each byte** in the input file.  
2. Builds a **Huffman tree** based on frequencies.  
3. Generates **Huffman codes** (shorter codes for frequent symbols).  
4. Stores a header with:
   - Original file size
   - Frequency table (to rebuild the tree during decompression)  
5. Writes compressed bitstream to the output file.  
6. During decompression, the program **rebuilds the Huffman tree** and restores the original data.

---
