#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <map>

using namespace std;

string inputFile = "data.txt";
string outputFile = "output.txt";
string timeStampsFile = "timestamps.txt";

ofstream fout(outputFile);

const int batchNum = 7;
const int hashDims = 500000;
const int maxStringLenght = 1000;
const int dims[7] = { 100, 500, 1000, 5000, 10000, 50000, 100000 };
const unsigned long long module = 2147483647; //большое простое число
const int p = 41;
vector<long long> p_pow(maxStringLenght);

//key: companyName

//простой хэш
unsigned int naiveHash(string key) {
    unsigned int result = 1;

    for (int i = 0; i < key.length(); ++i)
        result = (result * (key[i] - 'a' + 1)) % hashDims;

    return result % hashDims;
}

//сложный хэш
unsigned int complicatedHash(string key) {
    unsigned long long hash = 0;
    for (size_t j = 0; j < key.length(); ++j)
        hash = (hash + (key[j] - 'a' + 1) * p_pow[j]) % module;

    return hash % hashDims;
}

struct Automobile {
    string name;
    string brand;
    int year;
    string number;
    string color;
    unsigned int hash;

    Automobile() {
        this->hash = naiveHash(this->name);
    }

    Automobile(string name, string brand, int year, string number, string color,
        unsigned int (*hashFunc)(string))
    {
        this->name = name;
        this->brand = brand;
        this->year = year;
        this->number = number;
        this->color = color;
        this->hash = hashFunc(name);
    }

    friend bool operator== (const Automobile& a, const Automobile& b) {
        if (a.year == b.year && a.name == b.name && a.brand == b.brand
            && a.number == b.number && a.color == b.color)
            return true;
        return false;
    }

    friend bool operator< (const Automobile& a, const Automobile& b) {
        if (a.number < b.number || ((a.number == b.number) && (a.year < b.year)) ||
            ((a.number == b.number) && (a.year == b.year) && (a.brand < b.brand)) ||
            ((a.number == b.number) && (a.year == b.year) && (a.brand == b.brand) && (a.color < b.color)) ||
            ((a.number == b.number) && (a.year == b.year) && (a.brand == b.brand) && (a.color == b.color) && (a.name < b.name)))
            return true;
        return false;
    }

    friend bool operator<= (const Automobile& a, const Automobile& b) {
        if (a < b || a == b)
            return true;
        return false;
    }

    friend bool operator> (const Automobile& a, const Automobile& b) {
        if (!(a < b) && !(a == b))
            return true;
        return false;
    }

    friend bool operator>= (const Automobile& a, const Automobile& b) {
        if (!(a < b))
            return true;
        return false;
    }

    friend ostream& operator<<(ostream& os, const Automobile& a) {
        os << a.name << ' ' << a.brand << ' ' << a.year << ' ' << a.number << ' ' << a.color << '\n';
        return os;
    }

    string getName() const {
        return this->name;
    }

    unsigned int getHash() const {
        return this->hash;
    }
};

//функция для чтения объектов из текстового файла
vector<vector<Automobile>> readTxtFile(unsigned int (*hashFunc)(string)) {
    ifstream fin(inputFile);

    vector<vector<Automobile>> result;

    int dim;
    string name, brand, number, color;
    int year;

    for (int i = 0; i < batchNum; ++i) {
        //Ввод числа записей
        fin >> dim;
        vector<Automobile> v;
        for (int j = 0; j < dim; ++j) {
            //Ввод полей по порядку
            fin >> name >> brand >> year >> number >> color;
            Automobile temp(name, brand, year, number, color, hashFunc);
            v.push_back(temp);
        }
        result.push_back(v);
    }

    return result;
}

void writeTime(string title, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end, int divideBy) {
    fout << title;

    fout << chrono::duration_cast<chrono::microseconds>(end - start).count() / divideBy << " [микросекунд]\n";
}

struct HashItem {
    vector<string> values;
};

class HashTable {
    vector<HashItem> table;

public:
    HashTable() {
        table.resize(hashDims);
    }

    void insert(Automobile& item) {
        //fout << "insert " << item.getHash() << '\n';
        HashItem& currentItem = this->table[item.getHash()];

        //при совпадении элементов выходим
        for (auto i : currentItem.values)
            if (i == item.getName())
                return;

        currentItem.values.push_back(item.getName());
    }

    vector<string> find(Automobile& item) {
        HashItem& currentItem = this->table[item.getHash()];

        return currentItem.values;
    }

    void clear() {
        this->table.clear();
        this->table.resize(hashDims);
    }


    unsigned int getCollisions() {
        unsigned int result = 0;

        for (auto n : this->table)
            if (n.values.size() > 1)
                result += n.values.size() - 1;

        return result;
    }
};

int main()
{
    setlocale(LC_ALL, "Russian");

    //заполнение массива степеней
    p_pow[0] = 1;
    for (size_t i = 1; i < p_pow.size(); ++i)
        p_pow[i] = p_pow[i - 1] * p;

    vector<vector<Automobile>> naiveArray = readTxtFile(naiveHash);

    HashTable naiveTable;

    std::chrono::steady_clock::time_point start, end;

    fout << "Простая хэш функция:\n" << '\n';

    for (int i = 0; i < batchNum; ++i) {
        //заполнение таблицы
        for (int j = 0; j < naiveArray[i].size(); ++j)
            naiveTable.insert(naiveArray[i][j]);

        start = std::chrono::steady_clock::now();

        //поиск всех элементов для получения среднего результата
        for (int j = 0; j < naiveArray[i].size(); ++j) {
            Automobile objectToFind = naiveArray[i][j];
            vector<string> foundItems = naiveTable.find(objectToFind);
        }

        end = std::chrono::steady_clock::now();
        fout << "\"Простая\" реализация таблицы с " + to_string(naiveArray[i].size()) + " элементами:\n";
        writeTime("На поиск в среднем уходит: ", start, end, naiveArray[i].size());
        fout << "Коллизий: " << naiveTable.getCollisions() << "\n\n";
        naiveTable.clear();
        start = end;
    }

    naiveArray.clear();

    vector<vector<Automobile>> complicatedArray = readTxtFile(complicatedHash);

    HashTable complicatedTable;

    fout << "Сложная хэш функция:\n" << '\n';

    for (int i = 0; i < batchNum; ++i) {
        //заполнение таблицы
        for (int j = 0; j < complicatedArray[i].size(); ++j)
            complicatedTable.insert(complicatedArray[i][j]);

        start = std::chrono::steady_clock::now();

        //поиск всех элементов для получения среднего результата
        for (int j = 0; j < complicatedArray[i].size(); ++j) {
            Automobile objectToFind = complicatedArray[i][j];
            vector<string> foundItems = complicatedTable.find(objectToFind);
        }

        end = std::chrono::steady_clock::now();
        fout << "\"Сложная\" реализация таблицы с " + to_string(complicatedArray[i].size()) + " элементами:\n";
        writeTime("На поиск в среднем уходит: ", start, end, complicatedArray[i].size());
        fout << "Коллизий: " << complicatedTable.getCollisions() << "\n\n";
        complicatedTable.clear();
    }

    return 0;
}
