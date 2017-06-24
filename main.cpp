//    Copyright (C) 2017 Michał Karol <michal.p.karol@gmail.com>

#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <thread>
#include <mutex>
#include <string.h>

using namespace std;

static const vector<wchar_t> accepted = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', L'ą', L'ć', L'ę',
    L'ł', L'ń', L'ó', L'ś', L'ź', L'ż'
};

static const vector <wchar_t> whitespaces {' ', '\n', '\r', '\t'};
static const double CUTOFF = 0.0001;
static const size_t BUFFER_SIZE = 2000000;


void countWords(const deque<wchar_t>& data, const size_t characters,
                const deque<wchar_t>::const_iterator& lastPos, map<deque<wchar_t>, uint>& words) noexcept {
    deque<wchar_t> chars;

    for (deque<wchar_t>::const_iterator i = data.begin(); i != lastPos; i++) {
        if (find(accepted.begin(), accepted.end(), (*i)) != accepted.end()) {
            chars.push_back((*i));
        } else {
            if (find(whitespaces.begin(), whitespaces.end(), (*i)) != whitespaces.end()) {
                if (chars.size() == characters) {
                    words[chars]++;
                }
            }

            chars.clear();
        }
    }
}
void countCharacters(const deque<wchar_t>& data, const size_t characters,
                     const deque<wchar_t>::const_iterator& lastPos, map<deque<wchar_t>, uint>& strings) noexcept {
    deque<wchar_t> chars;

    for (deque<wchar_t>::const_iterator i = data.begin(); i != lastPos; i++) {
        if (find(accepted.begin(), accepted.end(), (*i)) != accepted.end()) {
            chars.push_back((*i));
        } else {
            chars.clear();
        }

        if (chars.size() == characters) {
            strings[chars]++;
            chars.pop_front();
        }
    }
}
void save(wofstream& stream, const map<deque<wchar_t>, uint>& map) {
    vector<pair<deque<wchar_t>, uint>> values(map.begin(), map.end());
    sort(values.begin(), values.end(), [](const pair<deque<wchar_t>, uint>& t1,
                                          const pair<deque<wchar_t>, uint>& t2) -> bool { return  ((t1.second == t2.second) ? t1.first < t2.first : t1.second > t2.second); });

    size_t sum = 0;

    for (const pair<deque<wchar_t>, uint>& pair : values) {
        sum += pair.second;
    }

    stream.precision(4);
    stream << fixed;

    for (const pair<deque<wchar_t>, uint>& pair : values) {
        double percent = ((pair.second * 100.0) / sum);

        if (percent >= CUTOFF) {

            for (const wchar_t& c : pair.first) {
                stream << c;
            }

            stream << ' ';
            stream << pair.second;

            stream << ' ';
            stream << percent;

            stream << '\n';
        }

    }

    stream << sum << '\n';

    stream.flush();
    stream.close();
}

void process(string filename, size_t numberOfCharacters, map<deque<wchar_t>, uint>& words,
             map<deque<wchar_t>, uint>& strings, mutex& wordsMutex, mutex& stringsMutex) {
    map<deque<wchar_t>, uint> wordsLocal;
    map<deque<wchar_t>, uint> stringsLocal;

    wifstream input;
    input.imbue(locale(""));
    input.open(filename);

    if (!input.is_open()) {
        cout << "Cannot open file: " << filename << endl;
        throw invalid_argument("IO error");
    }

    deque<wchar_t> buffer;

    for (wchar_t c; input.get(c);) {
        buffer.push_back(tolower(c));

        if (buffer.size() > BUFFER_SIZE) {
            deque<wchar_t>::const_iterator lastIndex = find(buffer.rbegin(), buffer.rend(),
                    static_cast<wchar_t>(' ')).base();

            if (lastIndex != buffer.rend().base()) {
                thread wordsThread(countWords, ref(buffer), numberOfCharacters, ref(lastIndex), ref(wordsLocal));
                thread stringsThread(countCharacters, ref(buffer), numberOfCharacters, ref(lastIndex),
                                     ref(stringsLocal));

                wordsThread.join();
                stringsThread.join();
                buffer.erase(buffer.begin(), lastIndex);
            }

        }
    }

    // Parsig rest of buffer
    deque<wchar_t>::const_iterator lastIndex = buffer.end();
    thread wordsThread(countWords, ref(buffer), numberOfCharacters, ref(lastIndex), ref(wordsLocal));
    thread stringsThread(countCharacters, ref(buffer), numberOfCharacters, ref(lastIndex),
                         ref(stringsLocal));

    wordsThread.join();
    // Merging words maps
    wordsMutex.lock();

    for (const pair<deque<wchar_t>, uint>& p : wordsLocal) {
        words[p.first] += p.second;
    }

    wordsMutex.unlock();

    stringsThread.join();
    // Merging strings maps
    stringsMutex.lock();

    for (const pair<deque<wchar_t>, uint>& p : stringsLocal) {
        strings[p.first] += p.second;
    }

    stringsMutex.unlock();

    cout << "File: " << filename << " processed." << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Usage: wiki2data inputFilePrefix numberOfFiles numberOfThreads numberOfCharacters" << endl;
        return 1;
    }

    string inputFilePrefix = string(argv[1], strlen(argv[1]));
    long numberOfFiles = stol(string(argv[2], strlen(argv[2])));
    int numberOfThreads = stoi(string(argv[3], strlen(argv[3])));
    long numberOfCharactersTmp = stol(string(argv[4], strlen(argv[4])));

    if (numberOfFiles <= 0) {
        cout << "Number of files should be greater than 0" << endl;
        return 2;
    }

    if (numberOfThreads <= 0) {
        cout << "Number of threads should be greater than 0" << endl;
        return 3;
    }

    if (numberOfCharactersTmp <= 0) {
        cout << "Number of characters should be greater than 0" << endl;
        return 4;
    }

    size_t numberOfCharacters = static_cast<size_t>(numberOfCharactersTmp);

    map<deque<wchar_t>, uint> words;
    map<deque<wchar_t>, uint> strings;

    mutex wordsMutex;
    mutex stringsMutex;

    deque<thread> activeThreads;

    for (int f = 0; f < numberOfFiles; f++) {
        char* suffix = new char[8];
        sprintf(suffix, "_%02d", f);
        string filename = inputFilePrefix + suffix;

        activeThreads.push_back(thread(process, filename, numberOfCharacters, ref(words), ref(strings),
                                       ref(wordsMutex),
                                       ref(stringsMutex)));

        // Wait until place in queue is available
        if (activeThreads.size() == static_cast<size_t>(numberOfThreads)) {
            activeThreads.front().join();
            activeThreads.pop_front();
        }
    }

    // Join all threads left;
    for (thread& t : activeThreads) {
        t.join();
    }

    activeThreads.clear();


    char* wordFilename = new char[8];
    sprintf(wordFilename, "word_%02zu", numberOfCharacters);

    wofstream wordData;
    wordData.imbue(locale(""));
    wordData.open(wordFilename);
    save(wordData, words);


    char* stringsFilename = new char[8];
    sprintf(stringsFilename, "char_%02zu", numberOfCharacters);

    wofstream stringsData;
    stringsData.imbue(locale(""));
    stringsData.open(stringsFilename);
    save(stringsData, strings);

    return 0;
}
