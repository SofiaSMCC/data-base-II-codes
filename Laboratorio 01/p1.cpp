#include <iostream>
#include <vector>
#include <fstream>
struct Alumno
{
    char codigo[5];
    char nombre[11];
    char apellidos[20];
    char carrera[15];
    int ciclo;
    float mensualidad;
    int next; // -2: Not eliminated, -1: Is Last eliminated, 0 > : Last eliminated pos
};
// FixedRecord con metodo FREE_LIST
class FixedRecord
{
private:
    std::string fileName;
    const int offset = sizeof(int);

public:
    std::vector<Alumno> alumnos;
    FixedRecord(std::string fileName)
    {
        this->fileName = fileName;
        std::fstream file(fileName, std::ios::in | std::ios::out |
                                        std::ios::binary);
        if (!file)
        {
            std::ofstream newFile(fileName, std::ios::out | std::ios::binary);
            int initialHeader = -1;
            newFile.write((char *)&initialHeader, offset);
            newFile.close();
        }
    }
    int getHeader()
    {
        std::fstream file(fileName, std::ios::in | std::ios::binary);
        int header;
        file.seekg(0, std::ios::beg);
        file.read((char *)&header, offset);
        file.close();
        return header;
    }
    void setHeader(int value)
    {
        std::fstream file(fileName, std::ios::in | std::ios::out |
                                        std::ios::binary);
        file.seekp(0, std::ios::beg);
        file.write((char *)&value, offset);
        file.close();
    }
    std::vector<Alumno> load()
    {
        std::fstream file(fileName, std::ios::in | std::ios::binary);
        file.seekg(offset, std::ios::beg);
        while (true)
        {
            Alumno alumno;
            file.read((char *)&alumno, sizeof(alumno));
            if (file.eof())
                break;
            if (alumno.next == -2)
                alumnos.push_back(alumno);
        }
        file.close();
        return alumnos;
    }
    int getLastFreeIndex(int from)
    {
        Alumno record = readRecord(from);
        if (record.next == -1)
        {
            return from;
        }
        else
        {
            return getLastFreeIndex(record.next);
        }
    }
    void add(Alumno record)
    {
        std::fstream file(fileName, std::ios::in | std::ios::out |
                                        std::ios::binary);
        int header = getHeader();
        record.next = -2;
        if (header == -1)
        { // (No se ha eliminado ninguno, no hay espacios vacios)
            file.seekp(0, std::ios::end);
        }
        else
        {
            int lastFreeIndex = getLastFreeIndex(header);
            if (lastFreeIndex == header)
            {
                setHeader(-1);
            }
            else
            {
                int previousIndex = header;
                while (readRecord(previousIndex).next != lastFreeIndex)
                {
                    previousIndex = readRecord(previousIndex).next;
                }
                Alumno previousRecord = readRecord(previousIndex);
                previousRecord.next = -1;
                file.seekp(offset + previousIndex * sizeof(Alumno), std::ios::beg);
                file.write((char *)&previousRecord, sizeof(previousRecord));
            }
            file.seekp(offset + lastFreeIndex * sizeof(Alumno), std::ios::beg);
        }
        file.write((char *)&record, sizeof(record));
        file.close();
    }
    Alumno readRecord(int pos)
    {
        std::fstream file(fileName, std::ios::in | std::ios::out |
                                        std::ios::binary);
        Alumno alumno;
        file.seekg(offset + pos * sizeof(Alumno), std::ios::beg);
        file.read((char *)&alumno, sizeof(alumno));
        file.close();
        return alumno;
    }
    bool del(int pos)
    {
        std::fstream file(fileName, std::ios::in | std::ios::out |
                                        std::ios::binary);
        file.seekg(0, std::ios::end);
        int lastPos = file.tellg() / sizeof(Alumno) - 1;
        if (pos < 0 || pos > lastPos)
        {
            return false;
        }
        int header = getHeader();
        if (header == -1)
        {
            setHeader(pos);
        }
        else
        {
            int lastFreeIndex = getLastFreeIndex(header);
            Alumno lastFreeRecord = readRecord(lastFreeIndex);
            lastFreeRecord.next = pos;
            file.seekp(offset + lastFreeIndex * sizeof(Alumno), std::ios::beg);
            file.write((char *)&lastFreeRecord, sizeof(lastFreeRecord));
        }
        Alumno posRecord = readRecord(pos);
        posRecord.next = -1;
        file.seekp(offset + pos * sizeof(Alumno), std::ios::beg);
        file.write((char *)&posRecord, sizeof(posRecord));
        file.close();
        return true;
    }
};

int main()
{
    // ** IMPORTANTE ** No debe existir el archivo data1-2.dat antes de ejecutar el codigo.
    FixedRecord records("data1-2.dat");
    Alumno al1 = {"1", "John", "Doe", "CS", 4, 1500.0f};
    Alumno al2 = {"2", "Amanda", "Diggins", "BioIng", 1, 1000.0f};
    Alumno al3 = {"3", "Edward", "Hopkins", "DS", 3, 1650.0f};
    Alumno al4 = {"4", "Renato", "Santos", "CS", 6, 1340.0f};
    records.add(al1);
    records.add(al2);
    records.add(al3);
    records.add(al4);
    records.del(0); // Elimina a "John" (Pos 0)
    records.del(2); // Elimina a "Edward" (Pos 2)
    Alumno al5 = {"5", "Luciano", "Aguirre", "CS", 5, 1300.0f};
    records.add(al5); // Aniade a "Luciano" a Pos 2 (Ultimo espacio liberado hasta este punto)
    Alumno al6 = {"6", "Alberto", "Fernandez", "CS", 5, 1300.0f};
    records.add(al6); // Aniade a "Alberto" a Pos 0 (Ultimo espacio liberado hasta este punto)
    auto alumnos = records.load();
    for (const auto &alumno : alumnos)
    {
        std::cout << "Codigo: " << alumno.codigo << "\n"
                  << "Nombre: " << alumno.nombre << "\n"
                  << "Apellidos: " << alumno.apellidos << "\n"
                  << "Carrera: " << alumno.carrera << "\n"
                  << "Ciclo: " << alumno.ciclo << "\n"
                  << "Mensualidad: " << alumno.mensualidad << "\n"
                  << "Next: " << alumno.next << "\n\n";
    }
    std::cout << "Header: " << records.getHeader() << "\n";
    return 0;
}