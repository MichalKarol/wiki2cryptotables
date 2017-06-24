//    Copyright (C) 2017 Michał Karol <michal.p.karol@gmail.com>

#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <thread>

using namespace std;

// Prepare gloabal map
static map<deque<wchar_t>, uint> words;
static map<deque<wchar_t>, uint> strings;

static const vector<wchar_t> accepted = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', L'ą', L'ć', L'ę',
    L'ł', L'ń', L'ó', L'ś', L'ź', L'ż'
};

static const vector <wchar_t> whitespaces {' ', '\n', '\r', '\t'};
static const double CUTOFF = 0.0001;


void countWords(const deque<wchar_t>& data, const uint characters,
                const deque<wchar_t>::const_iterator& lastPos) {
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
void countCharacters(const deque<wchar_t>& data, const uint characters,
                     const deque<wchar_t>::const_iterator& lastPos) {
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

int main() {
    uint numberOfFiles = 0;
    cout << "Input number of files" << endl;
    cin >> numberOfFiles;

    uint numberOfCharacters = 0;
    cout << "Input number of characters" << endl;
    cin >> numberOfCharacters;

    wifstream input;
    input.imbue(std::locale("pl_PL.UTF-8"));

    for (uint f = 0; f < numberOfFiles; f++) {
        char* filename = new char[8];
        sprintf(filename, "wiki_%02d", f);

        input.open(filename);

        if (!input.is_open()) {
            cout << "Cannot open files" << endl;
            return 1;
        }

        deque<wchar_t> buffer;

        for (wchar_t c; input.get(c);) {
            buffer.push_back(tolower(c));

            if (buffer.size() > 1'000'000) {
                deque<wchar_t>::const_iterator lastIndex = find(buffer.rbegin(), buffer.rend(),
                        static_cast<wchar_t>(' ')).base();

                if (lastIndex != buffer.rend().base()) {
                    thread t1(countWords, ref(buffer), numberOfCharacters, ref(lastIndex));
                    thread t2(countCharacters, ref(buffer), numberOfCharacters, ref(lastIndex));

                    t1.join();
                    t2.join();
                    buffer.erase(buffer.begin(), lastIndex);
                }

            }
        }
    }

    char* wordFilename = new char[8];
    sprintf(wordFilename, "word_%02d", numberOfCharacters);

    wofstream wordData;
    wordData.imbue(std::locale("pl_PL.UTF-8"));
    wordData.open(wordFilename);
    save(wordData, words);


    char* stringsFilename = new char[8];
    sprintf(stringsFilename, "char_%02d", numberOfCharacters);

    wofstream stringsData;
    stringsData.imbue(std::locale("pl_PL.UTF-8"));
    stringsData.open(stringsFilename);
    save(stringsData, strings);

    return 0;
}
