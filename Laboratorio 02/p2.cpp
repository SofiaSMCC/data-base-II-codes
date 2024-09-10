#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <cstring>

using namespace std;

struct Record
{
    int codigo;
    char nombre[12];
    char apellido[12];
    int ciclo;

    Record() : codigo(0), ciclo(0)
    {
        memset(nombre, 0, sizeof(nombre));
        memset(apellido, 0, sizeof(apellido));
    }
    Record(int codigo, const string &nombre, const string &apellido, int ciclo)
        : codigo(codigo), ciclo(ciclo)
    {
        strncpy(this->nombre, nombre.c_str(), sizeof(this->nombre) - 1);
        this->nombre[sizeof(this->nombre) - 1] = '\0';
        strncpy(this->apellido, apellido.c_str(), sizeof(this->apellido) - 1);
        this->apellido[sizeof(this->apellido) - 1] = '\0';
    }

    void showData() const
    {
        cout << "\nCodigo: " << codigo;
        cout << "\nNombre: " << nombre;
        cout << "\nApellido: " << apellido;
        cout << "\nCiclo : " << ciclo;
        cout << endl;
    }

    bool operator==(const Record &other) const
    {
        return codigo == other.codigo && strcmp(nombre, other.nombre) == 0 && strcmp(apellido, other.apellido) == 0 && ciclo == other.ciclo;
    }
};

class RandomFile
{
private:
    string fileName;
    string indexName;
    map<int, long> index;

public:
    RandomFile(string _fileName) : fileName(_fileName), indexName(_fileName + "_ind")
    {
        readIndex();
    }

    ~RandomFile()
    {
        writeIndex();
    }

    const map<int, long> &getIndex() const
    {
        return index;
    }

    void readIndex()
    {
        ifstream indexFile(indexName, ios::binary);
        if (indexFile)
        {
            int codigo;
            long pos;
            while (indexFile.read(reinterpret_cast<char *>(&codigo), sizeof(codigo)) &&
                   indexFile.read(reinterpret_cast<char *>(&pos), sizeof(pos)))
            {
                index[codigo] = pos;
            }
        }
    }

    void writeIndex()
    {
        ofstream indexFile(indexName, ios::binary | ios::trunc);
        for (const auto &entry : index)
        {
            indexFile.write(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first));
            indexFile.write(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
        }
    }

    void write_record(const Record &record)
    {
        fstream dataFile(fileName, ios::binary | ios::in | ios::out | ios::app);
        if (!dataFile.is_open())
        {
            dataFile.open(fileName, ios::binary | ios::out);
        }

        dataFile.seekp(0, ios::end);
        long pos = dataFile.tellp();
        dataFile.write(reinterpret_cast<const char *>(&record), sizeof(record));
        dataFile.close();

        index[record.codigo] = pos;
    }

    Record *find(int key)
    {
        auto it = index.find(key);
        if (it != index.end())
        {
            long pos = it->second;
            fstream dataFile(fileName, ios::binary | ios::in);
            dataFile.seekg(pos);
            Record *record = new Record();
            dataFile.read(reinterpret_cast<char *>(record), sizeof(Record));
            dataFile.close();
            return record;
        }
        return nullptr;
    }

    void scanAll()
    {
        ifstream dataFile(fileName, ios::binary);
        Record record;
        while (dataFile.read(reinterpret_cast<char *>(&record), sizeof(Record)))
        {
            record.showData();
        }
        dataFile.close();
    }

    void scanAllByIndex()
    {
        for (const auto &entry : index)
        {
            ifstream dataFile(fileName, ios::binary);
            dataFile.seekg(entry.second);
            Record record;
            dataFile.read(reinterpret_cast<char *>(&record), sizeof(Record));
            dataFile.close();
            record.showData();
        }
    }

    void remove(int key)
    {
        auto it = index.find(key);
        if (it != index.end())
        {
            long pos = it->second;

            index.erase(it);

            vector<Record> records;
            ifstream dataFile(fileName, ios::binary);
            Record record;
            while (dataFile.read(reinterpret_cast<char *>(&record), sizeof(Record)))
            {
                if (dataFile.tellg() != pos)
                {
                    records.push_back(record);
                }
            }
            dataFile.close();

            ofstream outFile(fileName, ios::binary | ios::trunc);
            for (const auto &rec : records)
            {
                outFile.write(reinterpret_cast<const char *>(&rec), sizeof(Record));
            }
            outFile.close();
        }
    }

    void buildFromCSV(const string &csvFileName)
    {
        ifstream csvFile(csvFileName);
        if (!csvFile.is_open())
            return;

        string line;
        getline(csvFile, line);

        while (getline(csvFile, line))
        {
            stringstream ss(line);
            string token;
            int codigo;
            string nombre, apellido;
            int ciclo;

            getline(ss, token, ',');
            codigo = stoi(token);

            getline(ss, token, ',');
            nombre = token;

            getline(ss, token, ',');
            apellido = token;

            getline(ss, token, ',');
            ciclo = stoi(token);

            Record record(codigo, nombre, apellido, ciclo);
            write_record(record);
        }
        csvFile.close();
    }
};

//------------------ testing ------------------
void testLeerIndice(RandomFile &rf)
{
    assert(!rf.getIndex().empty() && "El índice no debería estar vacío después de leer desde el archivo CSV.");
    cout << "testLeerIndice pasado." << endl;
}

void testBuscarRegistro(RandomFile &rf, const Record &expected)
{
    Record *result = rf.find(expected.codigo);
    assert(result != nullptr && "El registro no se encontró en la base de datos.");
    assert(*result == expected && "El registro no concuerda con el registrado en BD.");
    delete result;
    cout << "testBuscarRegistro para el codigo " << expected.codigo << " pasado." << endl;
}

int main()
{
    RandomFile rf("rf_data.dat");

    rf.buildFromCSV("datos.csv");

    testLeerIndice(rf);

    testBuscarRegistro(rf, Record(23803540, "Isabel", "Gil", 5));
    testBuscarRegistro(rf, Record(51979300, "Montserrat", "Navarro", 2));
    testBuscarRegistro(rf, Record(66994658, "Marta", "Sanz", 3));
    testBuscarRegistro(rf, Record(21678159, "Vicente", "Garrido", 1));
    testBuscarRegistro(rf, Record(66384772, "Francisca", "Rubio", 10));
    testBuscarRegistro(rf, Record(19425339, "Rosa", "Ramos", 4));

    cout << "Todos los tests pasaron correctamente." << endl;

    // Todos los registros en el archivo rf_data.dat
    // Imprimirá todos los registros
    // rf.scanAll();

    // Prueba de eliminación y búsqueda de un registro
    rf.remove(51979300);
    testBuscarRegistro(rf, Record(51979300, "Montserrat", "Navarro", 2));

    // Prueba de búsqueda de un registro con datos incorrectos
    testBuscarRegistro(rf, Record(21678159, "Vicente", "Garrido", 17));

    return 0;
}
