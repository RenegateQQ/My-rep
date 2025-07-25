#ifndef __PROGTEST__
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#endif /* __PROGTEST__ */
class CCell 
{
  public:
    virtual ~CCell() = default;
    virtual std::shared_ptr<CCell> clone() const = 0;
    virtual std::vector<std::string> render(unsigned width, unsigned height) const = 0;
    virtual std::pair<size_t, size_t> getSize() const = 0;
    virtual void updateText(const std::string &) {}
    virtual bool operator==(const CCell &other) const = 0;
    bool operator!=(const CCell &other) const { return !(*this == other); }
};
      
class CEmpty : public CCell {
  public:
    std::shared_ptr<CCell> clone() const override {
      return std::make_shared<CEmpty>();
    }
  
    std::vector<std::string> render(unsigned width, unsigned height) const override {
      return std::vector<std::string>(height, std::string(width, ' '));
    }
  
    std::pair<size_t, size_t> getSize() const override {
      return {0, 0};
    }
  
    bool operator==(const CCell &other) const override {
      return dynamic_cast<const CEmpty *>(&other) != nullptr;
    }
};
      
class CText : public CCell {
  public:
    enum Align { ALIGN_LEFT, ALIGN_RIGHT };
  
    CText(const std::string &text, Align align) : mText(text), mAlign(align) {}
  
    void setText(const std::string &text)  { mText = text; }
  
    std::shared_ptr<CCell> clone() const override {
      return std::make_shared<CText>(*this);
    }
  
    std::pair<size_t, size_t> getSize() const override {
      std::istringstream is(mText);
      std::string line;
      size_t width = 0, height = 0;
      while (std::getline(is, line)) {
        width = std::max(width, line.length());
        ++height;
      }
      return {width, height};
    }
  
    std::vector<std::string> render(unsigned width, unsigned height) const override {
      std::vector<std::string> lines;
      std::istringstream is(mText);
      std::string line;
      while (std::getline(is, line)) {
        if (line.length() > width) line.resize(width);
        if (mAlign == ALIGN_LEFT)
          line += std::string(width - line.length(), ' ');
        else
          line = std::string(width - line.length(), ' ') + line;
        lines.push_back(line);
      }
      while (lines.size() < height)
        lines.push_back(std::string(width, ' '));
      return lines;
    }
  
    bool operator==(const CCell &other) const override {
      const CText *p = dynamic_cast<const CText *>(&other);
      return p && mText == p->mText && mAlign == p->mAlign;
    }
  
  private:
    std::string mText;
    Align mAlign;
};
      
class CImage : public CCell 
{
  public:
    CImage &addRow(const std::string &row) {
      mRows.push_back(row);
      return *this;
    }
  
    std::shared_ptr<CCell> clone() const override {
      return std::make_shared<CImage>(*this);
    }
  
    std::pair<size_t, size_t> getSize() const override {
      size_t height = mRows.size();
      size_t width = height ? mRows[0].length() : 0;
      return {width, height};
    }
  
    std::vector<std::string> render(unsigned width, unsigned height) const override {
      std::vector<std::string> output(height, std::string(width, ' '));
      size_t imgH = mRows.size();
      size_t imgW = imgH ? mRows[0].length() : 0;
      size_t offsetY = (height > imgH) ? (height - imgH) / 2 : 0;
      size_t offsetX = (width > imgW) ? (width - imgW) / 2 : 0;
  
      for (size_t i = 0; i < imgH && i + offsetY < height; ++i) {
        for (size_t j = 0; j < imgW && j + offsetX < width; ++j) {
          output[i + offsetY][j + offsetX] = mRows[i][j];
        }
      }
      return output;
    }
  
    bool operator==(const CCell &other) const override {
      const CImage *p = dynamic_cast<const CImage *>(&other);
      return p && mRows == p->mRows;
    }
  
  private:
    std::vector<std::string> mRows;
};
  
class CTable {
  public:
    CTable(size_t rows, size_t cols)
        : mRows(rows), mCols(cols), mData(rows, std::vector<std::shared_ptr<CCell>>(cols)) {
      for (auto &row : mData)
        for (auto &cell : row)
          cell = std::make_shared<CEmpty>();
    }
  
    CTable(const CTable &src)
        : mRows(src.mRows), mCols(src.mCols), mData(mRows, std::vector<std::shared_ptr<CCell>>(mCols)) {
      for (size_t i = 0; i < mRows; ++i)
        for (size_t j = 0; j < mCols; ++j)
          mData[i][j] = src.mData[i][j]->clone();
    }
  
    CTable &operator=(const CTable &src) {
      if (this == &src)
        return *this;
      mRows = src.mRows;
      mCols = src.mCols;
      mData = std::vector<std::vector<std::shared_ptr<CCell>>>(mRows, std::vector<std::shared_ptr<CCell>>(mCols));
      for (size_t i = 0; i < mRows; ++i)
        for (size_t j = 0; j < mCols; ++j)
          mData[i][j] = src.mData[i][j]->clone();
      return *this;
    }
  
    const CCell &getCell(size_t row, size_t col) const {
      return *mData.at(row).at(col);
    }
  
    CCell &getCell(size_t row, size_t col) {
      return *mData.at(row).at(col);
    }
  
    void setCell(size_t row, size_t col, const CCell &val) {
      mData.at(row).at(col) = val.clone();
    }
  
    bool operator==(const CTable &other) const {
      if (mRows != other.mRows || mCols != other.mCols)
        return false;
      for (size_t i = 0; i < mRows; ++i)
        for (size_t j = 0; j < mCols; ++j)
          if (!(*mData[i][j] == *other.mData[i][j]))
            return false;
      return true;
    }
  
    bool operator!=(const CTable &other) const {
      return !(*this == other);
    }
  
    friend std::ostream &operator<<(std::ostream &os, const CTable &tbl) {
      std::vector<size_t> colWidths(tbl.mCols, 0);
      std::vector<size_t> rowHeights(tbl.mRows, 0);
  
      for (size_t i = 0; i < tbl.mRows; ++i) {
        for (size_t j = 0; j < tbl.mCols; ++j) {
          auto [w, h] = tbl.mData[i][j]->getSize();
          colWidths[j] = std::max(colWidths[j], w);
          rowHeights[i] = std::max(rowHeights[i], h);
        }
      }
  
      std::vector<std::vector<std::vector<std::string>>> rendered(tbl.mRows);
      for (size_t i = 0; i < tbl.mRows; ++i) {
        rendered[i].resize(tbl.mCols);
        for (size_t j = 0; j < tbl.mCols; ++j) {
          rendered[i][j] = tbl.mData[i][j]->render(colWidths[j], rowHeights[i]);
        }
      }
  
      for (size_t i = 0; i < tbl.mRows; ++i) {
        os << '+';
        for (size_t j = 0; j < tbl.mCols; ++j)
          os << std::string(colWidths[j], '-') << '+';
        os << '\n';
  
        for (size_t line = 0; line < rowHeights[i]; ++line) {
          os << '|';
          for (size_t j = 0; j < tbl.mCols; ++j) {
            os << rendered[i][j][line] << '|';
          }
          os << '\n';
        }
      }
  
      os << '+';
      for (size_t j = 0; j < tbl.mCols; ++j)
        os << std::string(colWidths[j], '-') << '+';
      os << '\n';
      return os;
    }
  
  private:
    size_t mRows, mCols;
    std::vector<std::vector<std::shared_ptr<CCell>>> mData;
};
  
      
            
#ifndef __PROGTEST__
int main ()
{
  std::ostringstream oss;
  CTable t0 ( 3, 2 );
  t0 . setCell ( 0, 0, CText ( "Hello,\n"
        "Hello Kitty", CText::ALIGN_LEFT ) );
  t0 . setCell ( 1, 0, CText ( "Lorem ipsum dolor sit amet", CText::ALIGN_LEFT ) );
  t0 . setCell ( 2, 0, CText ( "Bye,\n"
        "Hello Kitty", CText::ALIGN_RIGHT ) );
  t0 . setCell ( 1, 1, CImage ()
        . addRow ( "###                   " )
        . addRow ( "#  #                  " )
        . addRow ( "#  # # ##   ###    ###" )
        . addRow ( "###  ##    #   #  #  #" )
        . addRow ( "#    #     #   #  #  #" )
        . addRow ( "#    #     #   #  #  #" )
        . addRow ( "#    #      ###    ###" )
        . addRow ( "                     #" )
        . addRow ( "                   ## " )
        . addRow ( "                      " )
        . addRow ( " #    ###   ###   #   " )
        . addRow ( "###  #   # #     ###  " )
        . addRow ( " #   #####  ###   #   " )
        . addRow ( " #   #         #  #   " )
        . addRow ( "  ##  ###   ###    ## " ) );
  t0 . setCell ( 2, 1, CEmpty () );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+--------------------------+----------------------+\n"
        "|Hello,                    |                      |\n"
        "|Hello Kitty               |                      |\n"
        "+--------------------------+----------------------+\n"
        "|Lorem ipsum dolor sit amet|###                   |\n"
        "|                          |#  #                  |\n"
        "|                          |#  # # ##   ###    ###|\n"
        "|                          |###  ##    #   #  #  #|\n"
        "|                          |#    #     #   #  #  #|\n"
        "|                          |#    #     #   #  #  #|\n"
        "|                          |#    #      ###    ###|\n"
        "|                          |                     #|\n"
        "|                          |                   ## |\n"
        "|                          |                      |\n"
        "|                          | #    ###   ###   #   |\n"
        "|                          |###  #   # #     ###  |\n"
        "|                          | #   #####  ###   #   |\n"
        "|                          | #   #         #  #   |\n"
        "|                          |  ##  ###   ###    ## |\n"
        "+--------------------------+----------------------+\n"
        "|                      Bye,|                      |\n"
        "|               Hello Kitty|                      |\n"
        "+--------------------------+----------------------+\n" );
  t0 . setCell ( 0, 1, t0 . getCell ( 1, 1 ) );
  t0 . setCell ( 2, 1, CImage ()
        . addRow ( "*****   *      *  *      ******* ******  *" )
        . addRow ( "*    *  *      *  *      *            *  *" )
        . addRow ( "*    *  *      *  *      *           *   *" )
        . addRow ( "*    *  *      *  *      *****      *    *" )
        . addRow ( "****    *      *  *      *         *     *" )
        . addRow ( "*  *    *      *  *      *        *       " )
        . addRow ( "*   *   *      *  *      *       *       *" )
        . addRow ( "*    *    *****   ****** ******* ******  *" ) );
  dynamic_cast<CText &> ( t0 . getCell ( 1, 0 ) ) . setText ( "Lorem ipsum dolor sit amet,\n"
        "consectetur adipiscing\n"
        "elit. Curabitur scelerisque\n"
        "lorem vitae lectus cursus,\n"
        "vitae porta ante placerat. Class aptent taciti\n"
        "sociosqu ad litora\n"
        "torquent per\n"
        "conubia nostra,\n"
        "per inceptos himenaeos.\n"
        "\n"
        "Donec tincidunt augue\n"
        "sit amet metus\n"
        "pretium volutpat.\n"
        "Donec faucibus,\n"
        "ante sit amet\n"
        "luctus posuere,\n"
        "mauris tellus" );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|Hello,                                        |          ###                             |\n"
        "|Hello Kitty                                   |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |          ###                             |\n"
        "|elit. Curabitur scelerisque                   |          #  #                            |\n"
        "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
        "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
        "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
        "|torquent per                                  |          #    #     #   #  #  #          |\n"
        "|conubia nostra,                               |          #    #      ###    ###          |\n"
        "|per inceptos himenaeos.                       |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|Donec tincidunt augue                         |                                          |\n"
        "|sit amet metus                                |           #    ###   ###   #             |\n"
        "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
        "|Donec faucibus,                               |           #   #####  ###   #             |\n"
        "|ante sit amet                                 |           #   #         #  #             |\n"
        "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  CTable t1 ( t0 );
  t1 . setCell ( 1, 0, CEmpty () );
  t1 . setCell ( 1, 1, CEmpty () );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|Hello,                                        |          ###                             |\n"
        "|Hello Kitty                                   |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |          ###                             |\n"
        "|elit. Curabitur scelerisque                   |          #  #                            |\n"
        "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
        "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
        "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
        "|torquent per                                  |          #    #     #   #  #  #          |\n"
        "|conubia nostra,                               |          #    #      ###    ###          |\n"
        "|per inceptos himenaeos.                       |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|Donec tincidunt augue                         |                                          |\n"
        "|sit amet metus                                |           #    ###   ###   #             |\n"
        "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
        "|Donec faucibus,                               |           #   #####  ###   #             |\n"
        "|ante sit amet                                 |           #   #         #  #             |\n"
        "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  oss . str ("");
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+-----------+------------------------------------------+\n"
        "|Hello,     |          ###                             |\n"
        "|Hello Kitty|          #  #                            |\n"
        "|           |          #  # # ##   ###    ###          |\n"
        "|           |          ###  ##    #   #  #  #          |\n"
        "|           |          #    #     #   #  #  #          |\n"
        "|           |          #    #     #   #  #  #          |\n"
        "|           |          #    #      ###    ###          |\n"
        "|           |                               #          |\n"
        "|           |                             ##           |\n"
        "|           |                                          |\n"
        "|           |           #    ###   ###   #             |\n"
        "|           |          ###  #   # #     ###            |\n"
        "|           |           #   #####  ###   #             |\n"
        "|           |           #   #         #  #             |\n"
        "|           |            ##  ###   ###    ##           |\n"
        "+-----------+------------------------------------------+\n"
        "+-----------+------------------------------------------+\n"
        "|       Bye,|*****   *      *  *      ******* ******  *|\n"
        "|Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|           |*    *  *      *  *      *           *   *|\n"
        "|           |*    *  *      *  *      *****      *    *|\n"
        "|           |****    *      *  *      *         *     *|\n"
        "|           |*  *    *      *  *      *        *       |\n"
        "|           |*   *   *      *  *      *       *       *|\n"
        "|           |*    *    *****   ****** ******* ******  *|\n"
        "+-----------+------------------------------------------+\n" );
  t1 = t0;
  t1 . setCell ( 0, 0, CEmpty () );
  t1 . setCell ( 1, 1, CImage ()
        . addRow ( "  ********                    " )
        . addRow ( " **********                   " )
        . addRow ( "**        **                  " )
        . addRow ( "**             **        **   " )
        . addRow ( "**             **        **   " )
        . addRow ( "***         ********  ********" )
        . addRow ( "****        ********  ********" )
        . addRow ( "****           **        **   " )
        . addRow ( "****           **        **   " )
        . addRow ( "****      **                  " )
        . addRow ( " **********                   " )
        . addRow ( "  ********                    " ) );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|Hello,                                        |          ###                             |\n"
        "|Hello Kitty                                   |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |          ###                             |\n"
        "|elit. Curabitur scelerisque                   |          #  #                            |\n"
        "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
        "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
        "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
        "|torquent per                                  |          #    #     #   #  #  #          |\n"
        "|conubia nostra,                               |          #    #      ###    ###          |\n"
        "|per inceptos himenaeos.                       |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|Donec tincidunt augue                         |                                          |\n"
        "|sit amet metus                                |           #    ###   ###   #             |\n"
        "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
        "|Donec faucibus,                               |           #   #####  ###   #             |\n"
        "|ante sit amet                                 |           #   #         #  #             |\n"
        "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  oss . str ("");
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                              |          ###                             |\n"
        "|                                              |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |                                          |\n"
        "|elit. Curabitur scelerisque                   |        ********                          |\n"
        "|lorem vitae lectus cursus,                    |       **********                         |\n"
        "|vitae porta ante placerat. Class aptent taciti|      **        **                        |\n"
        "|sociosqu ad litora                            |      **             **        **         |\n"
        "|torquent per                                  |      **             **        **         |\n"
        "|conubia nostra,                               |      ***         ********  ********      |\n"
        "|per inceptos himenaeos.                       |      ****        ********  ********      |\n"
        "|                                              |      ****           **        **         |\n"
        "|Donec tincidunt augue                         |      ****           **        **         |\n"
        "|sit amet metus                                |      ****      **                        |\n"
        "|pretium volutpat.                             |       **********                         |\n"
        "|Donec faucibus,                               |        ********                          |\n"
        "|ante sit amet                                 |                                          |\n"
        "|luctus posuere,                               |                                          |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  assert ( t0 != t1 );
  assert ( !( t0 == t1 ) );
  assert ( t0 . getCell ( 1, 1 ) == t0 . getCell ( 0, 1 ) );
  assert ( ! ( t0 . getCell ( 1, 1 ) != t0 . getCell ( 0, 1 ) ) );
  assert ( t0 . getCell ( 0, 0 ) != t0 . getCell ( 0, 1 ) );
  assert ( ! ( t0 . getCell ( 0, 0 ) == t0 . getCell ( 0, 1 ) ) );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
