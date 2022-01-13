// inspired by https://github.com/brmson/dawg-levenshtein/blob/master/Dawg.cpp
#include "MorphAutomat.h"

enum OpType {
	DELETE = 1, REPLACE = 2, INSERT = 4, NOTHING = 0
};

template<typename T>
T** newArray(const size_t n1, const size_t n2) {
	T** ar = new T * [n1];
	for (size_t i = 0; i < n1; i++) {
		ar[i] = new T[n2];
	}
	return ar;
}


template<typename T>
void deleteArray(T** ar, const size_t n1) {
	for (size_t i = 0; i < n1; i++) {
		delete[] ar[i];
	}
	delete[] ar;
}

void CMorphAutomat::FuzzySearchRecursive(const CMorphAutomRelation& node, std::string& word, std::vector<CFuzzyResult>& results, int** rows,
    char** path_rows, int fuzziness, BYTE* path, int depth) const {
    size_t word_length = word.size();
    size_t buffer_size = word_length + 1;

    int* row = rows[depth];
    char* path_row = path_rows[depth];
    int* prev_row = rows[depth - 1];
    row[0] = prev_row[0] + 1;
    path_row[0] = OpType::DELETE;
    int minimum = INT32_MAX;
    for (int i = 1; i < buffer_size; i++) {
        bool cont = true;
        int insert_cost = row[i - 1] + 1;
        int delete_cost = prev_row[i] + 1;
        int replace_cost = prev_row[i - 1];
        if ((BYTE)word[i - 1] != node.GetRelationalChar()) {
            replace_cost++;
            cont = false;
        }
        path_row[i] = 0;
        if (insert_cost <= delete_cost && insert_cost <= replace_cost) {
            path_row[i] |= OpType::INSERT;
            row[i] = insert_cost;
        }
        else if (delete_cost <= replace_cost) {
            row[i] = delete_cost;
            path_row[i] |= OpType::DELETE;
        }
        else if (!cont) {
            row[i] = replace_cost;
            path_row[i] |= OpType::REPLACE;
        }
        else {
            row[i] = replace_cost;
        }
        if (minimum > row[i]) {
            minimum = row[i];
        }
    }
    if (node.GetRelationalChar() == MorphAnnotChar) {
        if (row[word_length] <= fuzziness) {
            CFuzzyResult r;
            r.CorrectedString = std::string(path, path + depth - 1); // -1 AnnotChar
            r.StringDistance = row[word_length];
            results.push_back(r);
        }
    }
    else if (minimum <= fuzziness) {
        size_t nodeNo = node.GetChildNo();
        size_t count = GetChildrenCount(nodeNo);
        for (size_t i = 0; i < count; i++) {
            const CMorphAutomRelation& child = GetChildren(nodeNo)[i];
            path[depth] = child.GetRelationalChar();
            FuzzySearchRecursive(child, word, results, rows, path_rows, fuzziness, path, depth + 1);
        }
    }
    path[depth] = 0;
}


std::vector<CFuzzyResult> CMorphAutomat::FuzzySearch(std::string word, int fuzziness) const {
    word += MorphAnnotChar;
	const size_t word_length = word.size();
	const size_t buffer_size = word_length + 1;
	const size_t buffer_size_f = buffer_size + fuzziness + 1;
	int** rows = newArray<int>(buffer_size_f, buffer_size);
	char** path_rows = newArray<char>(buffer_size_f, buffer_size);
	int* row = rows[0];
	const size_t max_word_size = 200;
	BYTE path[max_word_size];
	std::memset(path, 0, sizeof path);
	for (int i = 0; i < buffer_size; i++) {
		row[i] = i;
		path_rows[0][i] = OpType::INSERT;
	}
	std::vector<CFuzzyResult> results;
    size_t startNode = 0;
	size_t count = GetChildrenCount(startNode);
	for (size_t i = 0; i < count; i++) {
		const CMorphAutomRelation& p = GetChildren(startNode)[i];
        path[0] = p.GetRelationalChar();
        FuzzySearchRecursive(p, word, results, rows, path_rows, fuzziness, path, 1);
	}
	deleteArray(rows, buffer_size_f);
	deleteArray(path_rows, buffer_size_f);
    sort(results.begin(), results.end());
	return results;
}
