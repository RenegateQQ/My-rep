#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <optional>
#include <memory>
#include <stdexcept>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#endif /* __PROGTEST__ */

class CLinker
{
public:
    CLinker() = default;
    ~CLinker() = default;
    CLinker(const CLinker&) = delete;
    CLinker& operator=(const CLinker&) = delete;

    CLinker& addFile(const std::string& fileName)
    {
        std::ifstream ifs(fileName, std::ios::binary);
        if (!ifs)
            throw std::runtime_error("Cannot read input file");

        ObjectFile obj;
        obj.fileName = fileName;

        obj.numExports = readUint32(ifs);
        obj.numImports = readUint32(ifs);
        obj.codeSize = readUint32(ifs);

        for (uint32_t i = 0; i < obj.numExports; ++i) {
            std::string name = readString(ifs);
            uint32_t offset = readUint32(ifs);

            if (obj.exports.count(name))
                throw std::runtime_error("Duplicate symbol: " + name);

            obj.exports[name] = offset;
        }

        for (uint32_t i = 0; i < obj.numImports; ++i) {
            std::string name = readString(ifs);
            uint32_t count = readUint32(ifs);
            std::vector<uint32_t> offsets(count);
            for (uint32_t j = 0; j < count; ++j)
                offsets[j] = readUint32(ifs);
            obj.imports[name].insert(obj.imports[name].end(), offsets.begin(), offsets.end());
        }

        obj.code.resize(obj.codeSize);
        ifs.read(reinterpret_cast<char*>(obj.code.data()), obj.codeSize);
        if (ifs.gcount() != static_cast<std::streamsize>(obj.codeSize))
            throw std::runtime_error("Error reading code section");

        objects.push_back(std::move(obj));
        size_t fileIndex = objects.size() - 1;

        for (const auto& [name, offset] : objects[fileIndex].exports) {
            if (exportedSymbols.count(name))
                throw std::runtime_error("Duplicate symbol: " + name);
            exportedSymbols[name] = { fileIndex, offset };
        }

        return *this;
    }

    void linkOutput(const std::string& outFileName, const std::string& entryPoint)
    {
        if (exportedSymbols.count(entryPoint) == 0)
            throw std::runtime_error("Undefined symbol " + entryPoint);

        std::vector<uint8_t> outputBuffer;
        std::unordered_map<std::string, uint32_t> newFunctionAddresses;
        std::unordered_map<std::string, uint32_t> functionSizes;
        std::unordered_set<std::string> visited;

        std::deque<std::string> toProcess;
        toProcess.push_back(entryPoint);
        std::vector<std::string> orderedFunctions;

        while (!toProcess.empty()) {
            std::string func = toProcess.front();
            toProcess.pop_front();

            if (visited.count(func))
                continue;
            visited.insert(func);
            orderedFunctions.push_back(func);

            const auto& [fileIndex, offset] = exportedSymbols.at(func);
            auto& obj = objects[fileIndex];

            uint32_t funcStart = offset;
            uint32_t funcEnd = obj.codeSize;
            for (const auto& [name, off] : obj.exports) {
                if (off > funcStart && off < funcEnd)
                    funcEnd = off;
            }
            if (funcEnd > obj.code.size())
                funcEnd = obj.code.size();

            if (funcStart >= obj.code.size() || funcEnd > obj.code.size() || funcStart > funcEnd)
                throw std::runtime_error("Invalid function boundaries for " + func);

            uint32_t newOffset = outputBuffer.size();
            newFunctionAddresses[func] = newOffset;
            functionSizes[func] = funcEnd - funcStart;

            outputBuffer.insert(outputBuffer.end(), obj.code.begin() + funcStart, obj.code.begin() + funcEnd);

            for (const auto& [importName, positions] : obj.imports) {
                for (uint32_t pos : positions) {
                    if (pos >= funcStart && pos < funcEnd) {
                        if (exportedSymbols.count(importName) == 0)
                            throw std::runtime_error("Undefined symbol " + importName);
                        toProcess.push_back(importName);
                    }
                }
            }
        }

        for (const std::string& func : orderedFunctions) {
            const auto& [fileIndex, offset] = exportedSymbols.at(func);
            auto& obj = objects[fileIndex];

            uint32_t funcStart = offset;
            uint32_t funcEnd = obj.codeSize;
            for (const auto& [name, off] : obj.exports) {
                if (off > funcStart && off < funcEnd)
                    funcEnd = off;
            }
            if (funcEnd > obj.code.size())
                funcEnd = obj.code.size();

            uint32_t newOffset = newFunctionAddresses[func];
            uint32_t funcSize = functionSizes[func];

            for (const auto& [importName, positions] : obj.imports) {
                for (uint32_t pos : positions) {
                    if (pos >= funcStart && pos + 4 <= funcEnd) {
                        if (newFunctionAddresses.count(importName) == 0)
                            throw std::runtime_error("Undefined symbol " + importName);

                        uint32_t relPos = newOffset + (pos - funcStart);
                        if (relPos + 4 > newOffset + funcSize)
                            throw std::runtime_error("Invalid patch position");

                        uint32_t importAddr = newFunctionAddresses[importName];
                        memcpy(&outputBuffer[relPos], &importAddr, sizeof(uint32_t));
                    }
                }
            }
        }

        std::ofstream ofs(outFileName, std::ios::binary);
        if (!ofs)
            throw std::runtime_error("Failed to open output file");

        ofs.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
        if (!ofs)
            throw std::runtime_error("Error writing output file");
    }

private:
    struct ObjectFile {
        std::string fileName;
        uint32_t numExports = 0;
        uint32_t numImports = 0;
        uint32_t codeSize = 0;
        std::unordered_map<std::string, uint32_t> exports;
        std::unordered_map<std::string, std::vector<uint32_t>> imports;
        std::vector<uint8_t> code;
    };

    struct ExportInfo {
        size_t fileIndex;  
        uint32_t offset;
    };

    std::vector<ObjectFile> objects;
    std::unordered_map<std::string, ExportInfo> exportedSymbols;

    uint32_t readUint32(std::istream& is) {
        uint8_t buf[4];
        is.read(reinterpret_cast<char*>(buf), 4);
        return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    }

    std::string readString(std::istream& is) {
        uint8_t len;
        is.read(reinterpret_cast<char*>(&len), 1);
        std::string s(len, '\0');
        is.read(&s[0], len);
        return s;
    }
};

#ifndef __PROGTEST__
int main ()
{
  CLinker () . addFile ( "0in0.o" ) . linkOutput ( "0out", "strlen" );

  CLinker () . addFile ( "1in0.o" ) . linkOutput ( "1out", "main" );

  CLinker () . addFile ( "2in0.o" ) . addFile ( "2in1.o" ) . linkOutput ( "2out", "main" );

  CLinker () . addFile ( "3in0.o" ) . addFile ( "3in1.o" ) . linkOutput ( "3out", "towersOfHanoi" );

  
  try
  {
    CLinker () . addFile ( "4in0.o" ) . addFile ( "4in1.o" ) . linkOutput ( "4out", "unusedFunc" );
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::runtime_error & e )
  {
    // e . what (): Undefined symbol qsort
  }
  catch ( ... )
  {
    assert ( "invalid exception" == nullptr );
  }

  try
  {
    CLinker () . addFile ( "5in0.o" ) . linkOutput ( "5out", "main" );
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::runtime_error & e )
  {
    // e . what (): Duplicate symbol: printf
  }
  catch ( ... )
  {
    assert ( "invalid exception" == nullptr );
  }

  try
  {
    CLinker () . addFile ( "6in0.o" ) . linkOutput ( "6out", "strlen" );
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::runtime_error & e )
  {
    // e . what (): Cannot read input file
  }
  catch ( ... )
  {
    assert ( "invalid exception" == nullptr );
  }

  try
  {
    CLinker () . addFile ( "7in0.o" ) . linkOutput ( "7out", "strlen2" );
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::runtime_error & e )
  {
    // e . what (): Undefined symbol strlen2
  }
  catch ( ... )
  {
    assert ( "invalid exception" == nullptr );
  }

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
