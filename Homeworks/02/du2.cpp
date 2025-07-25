#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#endif /* __PROGTEST__ */
using namespace std;

class CString {
  public:
    CString() : m_data(new char[1]), m_size(0) {
      m_data[0] = '\0';
    }
    CString(const char* str) : m_size(strlen(str)) {
      m_data = new char[m_size + 1];
      strcpy(m_data, str);
    }
    CString(const CString& other) : m_size(other.m_size) {
      m_data = new char[m_size + 1];
      strcpy(m_data, other.m_data);
    }
    CString(CString&& other) noexcept : m_data(other.m_data), m_size(other.m_size) {
      other.m_data = nullptr;
      other.m_size = 0;
    }
    CString& operator=(const CString& other) {
      if (this != &other) {
        delete[] m_data;
        m_size = other.m_size;
        m_data = new char[m_size + 1];
        memcpy(m_data, other.m_data, m_size + 1);
      }
      return *this;
    }
    CString& operator=(CString&& other) noexcept {
      if (this != &other) {
        delete[] m_data;
        m_data = other.m_data;
        m_size = other.m_size;
        other.m_data = nullptr;
        other.m_size = 0;
      }
      return *this;
    }
    ~CString() {
      delete[] m_data;
    }
    const char* c_str() const {
      return m_data;
    }
    size_t size() const {
      return m_size;
    }
    bool operator==(const CString& other) const {
      return strcmp(m_data, other.m_data) == 0;
    }
    bool operator!=(const CString& other) const {
      return strcmp(m_data, other.m_data) != 0;
    }
    char& operator[](size_t index) {
      return m_data[index];
    }
    const char& operator[](size_t index) const {
      return m_data[index];
    }
    friend std::ostream& operator<<(std::ostream& os, const CString& str) {
      return os << str.c_str();
    }
    bool operator<(const CString& other) const {
      return strcmp(m_data, other.m_data) < 0;
    }
    bool operator>(const CString& other) const {
      return strcmp(m_data, other.m_data) > 0;
    }
  private:
    char* m_data;
    size_t m_size;
};

struct TPerson {
  CString m_Id;
  CString m_Name;
  CString m_Surname;
  CString m_Date;
  CString m_Street;
  CString m_City;
};

class Vector {
  public:
    using value_type = TPerson;
    Vector() : m_capacity(0), m_size(0), m_data(nullptr) {}
    Vector(const Vector& other) : m_capacity(other.m_capacity), m_size(other.m_size) {
      m_data = new TPerson[m_capacity];
      for (size_t i = 0; i < m_size; i++) {
        m_data[i] = other.m_data[i];
      }
    }
    Vector(Vector&& other) noexcept : m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data) {
      other.m_data = nullptr;
      other.m_capacity = 0;
      other.m_size = 0;
    }
    Vector& operator=(const Vector& other) {
      if (this == &other) return *this;
      TPerson* newData = new TPerson[other.m_capacity];
      for (size_t i = 0; i < other.m_size; ++i) {
        newData[i] = other.m_data[i];
      }
      delete[] m_data;
      m_data = newData;
      m_capacity = other.m_capacity;
      m_size = other.m_size;
      return *this;
    }
    Vector& operator=(Vector&& other) noexcept {
      if (this != &other) {
        delete[] m_data;
        m_data = other.m_data;
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        other.m_data = nullptr;
        other.m_capacity = 0;
        other.m_size = 0;
      }
      return *this;
    }
    ~Vector() {
      delete[] m_data;
    }
    void push_back(value_type&& value) {
      if (m_size >= m_capacity) {
        realloc();
      }
      m_data[m_size++] = std::move(value);
    }
    size_t size() const {
      return m_size;
    }
    value_type& at(size_t index) {
      assert(index < size());
      return m_data[index];
    }
    const value_type& at(size_t index) const {
      assert(index < size());
      return m_data[index];
    }
    size_t lower_bound(const CString& id) const {
      size_t left = 0, right = m_size;
      while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (m_data[mid].m_Id < id) {
          left = mid + 1;
        } else {
          right = mid;
        }
      }
      return left;
    }
    void insert(size_t index, value_type&& value) {
      assert(index <= m_size);
      if (m_size == m_capacity) {
        realloc();
      }
      for (size_t i = m_size; i > index; --i) {
        m_data[i] = std::move(m_data[i - 1]);
      }
      m_data[index] = std::move(value);
      ++m_size;
    }
  private:
    size_t m_capacity;
    size_t m_size;
    value_type* m_data;
    void realloc() {
      m_capacity = m_capacity * 2 + 1;
      TPerson* data = new TPerson[m_capacity];
      for (size_t i = 0; i < m_size; i++) {
        data[i] = std::move(m_data[i]);
      }
      delete[] m_data;
      m_data = data;
    }
};

class CRegister {
  public:
    CRegister() = default;
    ~CRegister() = default;
    CRegister(const CRegister& other) : db(other.db) {}
    CRegister& operator=(const CRegister& other) {
      if (this != &other) {
        db = other.db;
      }
      return *this;
    }
    bool add(const char id[], const char name[], const char surname[], const char date[], const char street[], const char city[]) {
      CString cid(id);
      size_t pos = db.lower_bound(cid);
      if (pos < db.size() && db.at(pos).m_Id == cid) {
        return false;
      }
      db.insert(pos, TPerson{std::move(cid), CString(name), CString(surname), CString(date), CString(street), CString(city)});
      return true;
    }
    bool resettle(const char id[], const char date[], const char street[], const char city[]) {
      CString cid(id);
      size_t pos = db.lower_bound(cid);
      if (pos == db.size() || db.at(pos).m_Id != cid) {
        return false;
      }
      for (size_t i = pos; i < db.size() && db.at(i).m_Id == cid; ++i) {
        if (db.at(i).m_Date == date) {
          return false;
        }
      }
      db.insert(pos, TPerson{std::move(cid), db.at(pos).m_Name, db.at(pos).m_Surname, CString(date), CString(street), CString(city)});
      return true;
    }
    bool print(std::ostream &os, const char id[]) const {
      CString cid(id);
      size_t pos = db.lower_bound(cid);
      if (pos == db.size() || db.at(pos).m_Id != cid) {
        return false;
      }
      TPerson records[1000];
      size_t count = 0;
      while (pos < db.size() && db.at(pos).m_Id == cid) {
        records[count++] = db.at(pos++);
      }
      for (size_t i = 1; i < count; ++i) {
        TPerson key = records[i];
        size_t j = i;
        while (j > 0 && records[j - 1].m_Date > key.m_Date) {
          records[j] = records[j - 1];
          --j;
        }
        records[j] = key;
      }
      os << cid << " " << records[0].m_Name << " " << records[0].m_Surname << "\n";
      for (size_t i = 0; i < count; ++i) {
        os << records[i].m_Date << " " << records[i].m_Street << " " << records[i].m_City << "\n";
      }
      return true;
    }
  private:
    Vector db;
};

#ifndef __PROGTEST__
int main ()
{
  char   lID[12], lDate[12], lName[50], lSurname[50], lStreet[50], lCity[50];
  std::ostringstream oss;
  CRegister  a;
  assert ( a . add ( "123456/7890", "John", "Smith", "2000-01-01", "Main street", "Seattle" ) == true );
  assert ( a . add ( "987654/3210", "Freddy", "Kruger", "2001-02-03", "Elm street", "Sacramento" ) == true );
  assert ( a . resettle ( "123456/7890", "2003-05-12", "Elm street", "Atlanta" ) == true );
  assert ( a . resettle ( "123456/7890", "2002-12-05", "Sunset boulevard", "Los Angeles" ) == true );
  oss . str ( "" );
  assert ( a . print ( oss, "123456/7890" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###" ) );
  oss . str ( "" );
  assert ( a . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
)###" ) );
  CRegister b ( a );
  assert ( b . resettle ( "987654/3210", "2008-04-12", "Elm street", "Cinccinati" ) == true );
  assert ( a . resettle ( "987654/3210", "2007-02-11", "Elm street", "Indianapolis" ) == true );
  oss . str ( "" );
  assert ( a . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2007-02-11 Elm street Indianapolis
)###" ) );
  oss . str ( "" );
  assert ( b . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###" ) );
  a = b;
  assert ( a . resettle ( "987654/3210", "2011-05-05", "Elm street", "Salt Lake City" ) == true );
  oss . str ( "" );
  assert ( a . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
2011-05-05 Elm street Salt Lake City
)###" ) );
  oss . str ( "" );
  assert ( b . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###" ) );
  assert ( b . add ( "987654/3210", "Joe", "Lee", "2010-03-17", "Abbey road", "London" ) == false );
  assert ( a . resettle ( "987654/3210", "2001-02-03", "Second street", "Milwaukee" ) == false );
  oss . str ( "" );
  assert ( a . print ( oss, "666666/6666" ) == false );

  CRegister  c;
  strncpy ( lID, "123456/7890", sizeof ( lID ) );
  strncpy ( lName, "John", sizeof ( lName ) );
  strncpy ( lSurname, "Smith", sizeof ( lSurname ) );
  strncpy ( lDate, "2000-01-01", sizeof ( lDate) );
  strncpy ( lStreet, "Main street", sizeof ( lStreet ) );
  strncpy ( lCity, "Seattle", sizeof ( lCity ) );
  assert (  c . add ( lID, lName, lSurname, lDate, lStreet, lCity ) == true );
  strncpy ( lID, "987654/3210", sizeof ( lID ) );
  strncpy ( lName, "Freddy", sizeof ( lName ) );
  strncpy ( lSurname, "Kruger", sizeof ( lSurname ) );
  strncpy ( lDate, "2001-02-03", sizeof ( lDate) );
  strncpy ( lStreet, "Elm street", sizeof ( lStreet ) );
  strncpy ( lCity, "Sacramento", sizeof ( lCity ) );
  assert (  c . add ( lID, lName, lSurname, lDate, lStreet, lCity ) == true );
  strncpy ( lID, "123456/7890", sizeof ( lID ) );
  strncpy ( lDate, "2003-05-12", sizeof ( lDate) );
  strncpy ( lStreet, "Elm street", sizeof ( lStreet ) );
  strncpy ( lCity, "Atlanta", sizeof ( lCity ) );
  assert ( c . resettle ( lID, lDate, lStreet, lCity ) == true );
  strncpy ( lID, "123456/7890", sizeof ( lID ) );
  strncpy ( lDate, "2002-12-05", sizeof ( lDate) );
  strncpy ( lStreet, "Sunset boulevard", sizeof ( lStreet ) );
  strncpy ( lCity, "Los Angeles", sizeof ( lCity ) );
  assert ( c . resettle ( lID, lDate, lStreet, lCity ) == true );
  oss . str ( "" );
  assert ( c . print ( oss, "123456/7890" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###" ) );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
