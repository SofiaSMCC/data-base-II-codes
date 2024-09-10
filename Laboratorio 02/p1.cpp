#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>

struct Record
{
    int codigo;
    char nombre[12];
    char apellido[12];
    int ciclo;
    long left = -1;
    long right = -1;
    int height = 0;
    void showData()
    {
        std::cout << "\nCodigo: " << codigo;
        std::cout << "\nNombre: " << nombre;
        std::cout << "\nApellido: " << apellido;
        std::cout << "\nCiclo : " << ciclo;
        std::cout << "\nLeft : " << left;
        std::cout << "\nRight : " << right;
        std::cout << "\nHeight : " << height;
    }
    bool operator==(const Record &other) const
    {
        return codigo == other.codigo && strcmp(nombre, other.nombre) == 0 &&
               strcmp(apellido, other.apellido) == 0 && ciclo == other.ciclo;
    }
};
template <typename TK>
class AVLFile
{
private:
    std::string filename;
    long pos_root;
    const long offset = sizeof(pos_root);

public:
    AVLFile(std::string filename)
    {
        this->filename = filename;
        this->pos_root = -1;
        std::fstream file(filename, std::ios::in | std::ios::out |
                                        std::ios::binary);
        if (!file.is_open())
        {
            file.open(filename, std::ios::out | std::ios::binary);
            file.write((char *)&pos_root, offset);
            file.close();
            loadDataFromCSV(); // Lee los datos del archivo .csv y los inserta al
            filename.dat
        }
        else
        {
            loadRootPos();
        }
        file.close();
    }
    ~AVLFile()
    {
        saveRootPos();
    }
    void loadDataFromCSV()
    {
        std::cout << "Loading from file \n";
        std::ifstream file("datos.csv");
        std::string line;
        getline(file, line);
        while (getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;
            Record record;
            getline(ss, token, ',');
            record.codigo = stoi(token);
            getline(ss, token, ',');
            strncpy(record.nombre, token.c_str(), sizeof(record.nombre));
            record.nombre[sizeof(record.nombre) - 1] = '\0'; // Asegurar el nulltermination
            getline(ss, token, ',');
            strncpy(record.apellido, token.c_str(), sizeof(record.apellido));
            record.apellido[sizeof(record.apellido) - 1] = '\0'; // Asegurar el
            null - termination
                       getline(ss, token, ',');
            record.ciclo = stoi(token);
            insert(record);
        }
        file.close();
    }
    Record find(TK key)
    {
        return find(pos_root, key);
    }
    Record find(long pos, TK key)
    {
        if (pos == -1)
        {
            return Record{};
        }
        auto temp = getRecord(pos);
        if (key < temp.codigo)
        {
            return find(temp.left, key);
        }
        else if (key > temp.codigo)
        {
            return find(temp.right, key);
        }
        else
        {
            return temp;
        }
    }
    Record getRecord(long pos)
    {
        std::fstream file(filename, std::ios::binary | std::ios::in |
                                        std::ios::out);
        Record record;
        file.seekg(offset + pos * sizeof(Record), std::ios::beg);
        file.read((char *)&record, sizeof(Record));
        file.close();
        return record;
    }
    void setRecord(long pos, Record record)
    {
        std::fstream file(filename, std::ios::binary | std::ios::in |
                                        std::ios::out);
        file.seekp(offset + pos * sizeof(Record), std::ios::beg);
        file.write((char *)&record, sizeof(Record));
        file.close();
    }
    void insert(Record record)
    {
        insert(pos_root, record);
    }
    void insert(long &pos, Record record)
    {
        if (pos == -1)
        {
            pos = getSize();
            setRecord(pos, record);
        }
        else
        {
            auto temp = getRecord(pos);
            if (record.codigo < temp.codigo)
            {
                if (temp.left == -1)
                {
                    temp.left = getSize();
                    setRecord(temp.left, record);
                }
                else
                {
                    insert(temp.left, record);
                }
            }
            else if (record.codigo > temp.codigo)
            {
                if (temp.right == -1)
                {
                    temp.right = getSize();
                    setRecord(temp.right, record);
                }
                else
                {
                    insert(temp.right, record);
                }
            }
            updateHeight(temp);
            balance(pos);
            setRecord(pos, temp);
        }
    }
    auto loadRootPos()
    {
        std::ifstream file(filename, std::ios::binary);
        file.seekg(0, std::ios::beg);
        file.read((char *)&pos_root, offset);
    }
    auto saveRootPos()
    {
        std::fstream file(filename, std::ios::binary | std::ios::in |
                                        std::ios::out);
        file.seekp(0, std::ios::beg);
        file.write((char *)&pos_root, offset);
    }
    void updateHeight(Record &record)
    {
        int leftHeight = getHeight(record.left);
        int rightHeight = getHeight(record.right);
        record.height = std::max(leftHeight, rightHeight);
    }
    int getHeight(long pos)
    {
        if (pos == -1)
            return -1;
        Record record = getRecord(pos);
        return record.height;
    }
    int getBalanceFactor(long pos)
    {
        if (pos == -1)
            return 0;
        Record record = getRecord(pos);
        return getHeight(record.left) - getHeight(record.right);
    }
    void balance(long &pos)
    {
        if (pos == -1)
            return;
        Record record = getRecord(pos);
        int hb = getBalanceFactor(pos);
        if (hb > 1)
        {
            if (getBalanceFactor(record.left) < 0)
                rotateLeft(record.left);
            rotateRight(pos);
        }
        else if (hb < -1)
        {
            if (getBalanceFactor(record.right) > 0)
                rotateRight(record.right);
            rotateLeft(pos);
        }
        updateHeight(record);
        setRecord(pos, record);
    }
    void rotateRight(long &pos)
    {
        Record record = getRecord(pos);
        Record leftChild = getRecord(record.left);
        record.left = leftChild.right;
        leftChild.right = pos;
        updateHeight(record);
        updateHeight(leftChild);
        setRecord(pos, record);
        setRecord(record.left, leftChild);
    }
    void rotateLeft(long &pos)
    {
        Record record = getRecord(pos);
        Record rightChild = getRecord(record.right);
        record.right = rightChild.left;
        rightChild.left = pos;
        updateHeight(record);
        updateHeight(rightChild);
        setRecord(pos, record);
        setRecord(record.right, rightChild);
    }
    auto getSize()
    {
        std::ifstream file(filename, std::ios::binary);
        file.seekg(0, std::ios::end);
        return ((int)file.tellg() - offset) / sizeof(Record);
    }
    std::vector<Record> inorder()
    {
        std::vector<Record> res;
        inorder(pos_root, res);
        return res;
    }
    void inorder(long pos, std::vector<Record> &res)
    {
        if (pos == -1)
            return;
        Record temp = getRecord(pos);
        if (temp.left != -1)
        {
            inorder(temp.left, res);
        }
        res.push_back(temp);
        if (temp.right != -1)
        {
            inorder(temp.right, res);
        }
    }
    bool remove(TK key)
    {
        return remove(pos_root, key);
    }
    bool remove(long &pos, TK key)
    {
        std::fstream file(filename, std::ios::binary | std::ios::in |
                                        std::ios::out);
        if (pos == -1)
            return false;
        Record record = find(key);
        if (record.left == -1 | record.right == -1)
        {
            long tempPos = (record.left != -1) ? record.left : record.right;
            if (tempPos == -1)
            {
                pos = -1;
            }
            else
            {
                Record temp = getRecord(pos);
                record = temp;
                pos = tempPos;
            }
        }
        else
        {
            long nextPos = getMinValueRecord(record.right);
            Record nextNode = getRecord(nextPos);
            record.codigo = nextNode.codigo;
            remove(record.right, nextNode.codigo);
        }
        if (pos != -1)
        {
            updateHeight(record);
            balance(pos);
            setRecord(pos, record);
        }
        return true;
    }
    auto getMinValueRecord(long pos)
    {
        Record record = getRecord(pos);
        long temp = pos;
        while (record.left != -1)
        {
            temp = record.left;
            record = getRecord(record.left);
        }
        return temp;
    }
};
std::vector<Record> leerCSV(const std::string &filename)
{
    std::ifstream file(filename);
    std::vector<Record> records;
    std::string line;
    if (!file.is_open())
    {
        std::cout << "No se pudo abrir el archivo.\n";
        return records;
    }
    getline(file, line); // Se agregó la siguiente linea para omitir el header.
    while (getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        Record record;
        getline(ss, token, ',');
        record.codigo = stoi(token);
        getline(ss, token, ',');
        strncpy(record.nombre, token.c_str(), sizeof(record.nombre));
        record.nombre[sizeof(record.nombre) - 1] = '\0'; // Asegurar el nulltermination
        getline(ss, token, ',');
        strncpy(record.apellido, token.c_str(), sizeof(record.apellido));
        record.apellido[sizeof(record.apellido) - 1] = '\0'; // Asegurar el nulltermination
        getline(ss, token, ',');
        record.ciclo = stoi(token);
        records.push_back(record);
    }
    file.close();
    return records;
}
void readFile(AVLFile<int> file)
{
    std::cout << "------------------------------------------\n";
    std::vector<Record> records = leerCSV("datos.csv");
    bool passed = true;
    for (auto &record : records)
    {
        Record r = file.find(record.codigo);
        if (!(r == record))
        {
            passed = false;
            std::cout << "Error en el record con codigo: " << record.codigo << "\n";
            std::cout << "Se esperaba: \n";
            record.showData();
            std::cout << "\nSe obtuvo: \n";
            r.showData();
            std::cout << "\n";
            break;
        }
    }
    if (passed)
        std::cout << "Todos los records fueron leidos correctamente\n";
}

int main()
{
    AVLFile<int> file("avl_data.dat");
    readFile(file);
    Record rec1 = {1023, "Luciano", "Aguirre", 5, -1, -1, 0};
    file.insert(rec1);
    Record rec2 = {99999999, "Diana", "Jesfen", 5, -1, -1, 0};
    file.insert(rec2);
    std::cout << "Tree size: " << file.getSize() << "\n";
    auto vec = file.inorder();
    for (auto &v : vec)
    {
        v.showData();
        std::cout << "\n";
    }
    return 0;
}