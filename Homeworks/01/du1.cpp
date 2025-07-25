#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <span>
#include <algorithm>
#include <memory>
#include <compare>
#include <complex>
#endif /* __PROGTEST__ */
using namespace std;
// keep this dummy version if you do not implement a real manipulator
std::ios_base & ( * poly_var ( const std::string & name ) ) ( std::ios_base & x )
{
  return [] ( std::ios_base & ios ) -> std::ios_base & { return ios; };
}

class CPolynomial
{
  public:
    CPolynomial() = default;

    CPolynomial(const CPolynomial& other) = default;

    ~CPolynomial() = default;

    CPolynomial& operator=(const CPolynomial& other) 
    {
      if (this != &other) { 
        coefficients = other.coefficients; 
      }
      return *this;
    }
    friend ostream& operator<<(ostream& os, const CPolynomial& poly) {
      ostringstream ss;
      bool first = true;
      vector<pair<int, double>> sortedCoeffs = poly.coefficients;
      sort(sortedCoeffs.begin(), sortedCoeffs.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
      });

      for (const auto& coeff : sortedCoeffs) 
      {
        if (coeff.second == 0) continue;
        if (!first) {
            ss << (coeff.second > 0 ? " + " : " - ");
        } else {
            if (coeff.second < 0) ss << "- ";
            first = false;
        }
        double absValue = abs(coeff.second);
        if (absValue != 1 || coeff.first == 0) {
            ss << absValue;
        }
        if(coeff.second == 1 || coeff.second == -1 )
        {
          ss << "x";
          if (coeff.first >= 1) 
          {
            ss << "^" << coeff.first;
          }
        }
        else if (coeff.first > 0) 
        {
          ss << "*x";
          if (coeff.first >= 1) 
          {
            ss << "^" << coeff.first;
          }
        }
      }
      if (first) {
        ss << "0";
      }
      os << ss.str();
      return os;
    }
  CPolynomial operator*(double scalar) const 
  {
    CPolynomial result = *this;
    for (auto& coeff : result.coefficients) {
        coeff.second *= scalar;
    }
    result.removeZeros();
    return result;
  }
  CPolynomial operator*(const CPolynomial& other) const {
    CPolynomial result;
    for (const auto& [deg1, coeff1] : coefficients) {
        for (const auto& [deg2, coeff2] : other.coefficients) {
            result[deg1 + deg2] += coeff1 * coeff2;
        }
    }
    result.removeZeros();
    return result;
  }
  CPolynomial& operator*=(double scalar) {
    for (auto& coeff : coefficients) {
        coeff.second *= scalar;
    }
    removeZeros();
    return *this;
  }
  CPolynomial& operator*=(const CPolynomial& other) {
    std::vector<std::pair<int, double>> result;
    for (const auto& [deg1, coeff1] : coefficients) {
        for (const auto& [deg2, coeff2] : other.coefficients) {
            bool found = false;
            for (auto& [degResult, coeffResult] : result) {
                if (degResult == deg1 + deg2) {
                    coeffResult += coeff1 * coeff2;
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.emplace_back(deg1 + deg2, coeff1 * coeff2);
            }
        }
    }
    coefficients = move(result);
    removeZeros();
    return *this;
  }
  bool operator==(const CPolynomial& other) const {
    auto filteredA = coefficients;
    auto filteredB = other.coefficients;

    filteredA.erase(remove_if(filteredA.begin(), filteredA.end(), 
                                   [](const auto& coeff) { return coeff.second == 0.0; }), 
                    filteredA.end());
    
    filteredB.erase(remove_if(filteredB.begin(), filteredB.end(), 
                                   [](const auto& coeff) { return coeff.second == 0.0; }), 
                    filteredB.end());

  
    return filteredA == filteredB;
  }

  bool operator!=(const CPolynomial& other) const {
    return coefficients != other.coefficients;
  }
  double& operator[](int degree) 
  {
    for (auto& coeff : coefficients) {
        if (coeff.first == degree) {
            return coeff.second;
        }
    }
    coefficients.emplace_back(degree, 0.0);
    return coefficients.back().second;
  }
  const double operator[](int degree) const 
  {
    for (const auto& coeff : coefficients) {
        if (coeff.first == degree) {
            return coeff.second;
        }
    }
    return 0.0;
  }
  unsigned degree() const 
  {
    unsigned maxDeg = 0;
    for (const auto& [deg, coeff] : coefficients) {
        if (fabs(coeff) > 1e-9) { 
            maxDeg = max(maxDeg, (unsigned)deg);
        }
    }
    return maxDeg;
  }
  double operator()(double x) const 
  {
    double result = 0.0;
    for (const auto& coeff : coefficients) {
        result += coeff.second * pow(x, coeff.first);
    }
    return result;
  }
  explicit operator bool() const {
    for (const auto& [deg, coeff] : coefficients) {
        if (coeff != 0.0) return true;
    }
    return false;
  }
  bool operator!() const {
    return static_cast<bool>(*this) == false;
  }
  private:
  vector<pair<int, double>> coefficients;

  void removeZeros() {
    coefficients.erase(std::remove_if(coefficients.begin(), coefficients.end(), [](const auto& coeff) {
        return coeff.second == 0.0;
    }), coefficients.end());
    if (coefficients.empty()) {
        coefficients.emplace_back(0, 0.0);
    }
}


};

#ifndef __PROGTEST__
bool smallDiff ( double a,
                 double b )
{
  return fabs(a - b) < 1e-9;
}

bool dumpMatch ( const CPolynomial & x,
                 const std::vector<double> & ref )
{
  if(ref.size() - 1 > x.degree())
  {
    return false;
  }
  for (size_t i = 0; i < ref.size(); ++i) 
  {
    if (!smallDiff(x[i], ref[i])) {
        return false;
    }
  }
  return true;
}

int main ()
{
  CPolynomial a, b, c;
  std::ostringstream out, tmp;

  a[0] = -10;
  a[1] = 3.5;
  a[3] = 1;
  assert ( smallDiff ( a ( 2 ), 5 ) );
  out . str ("");
  out << a;
  assert ( out . str () == "x^3 + 3.5*x^1 - 10" );
  c = a * -2;
  assert ( c . degree () == 3
           && dumpMatch ( c, std::vector<double>{ 20.0, -7.0, -0.0, -2.0 } ) );

  out . str ("");
  out << c;
  assert ( out . str () == "- 2*x^3 - 7*x^1 + 20" );
  out . str ("");
  out << b;
  assert ( out . str () == "0" );
  b[5] = -1;
  b[2] = 3;
  out . str ("");
  out << b;
  assert ( out . str () == "- x^5 + 3*x^2" );
  c = a * b;
  assert ( c . degree () == 8
           && dumpMatch ( c, std::vector<double>{ -0.0, -0.0, -30.0, 10.5, -0.0, 13.0, -3.5, 0.0, -1.0 } ) );

  out . str ("");
  out << c;
  assert ( out . str () == "- x^8 - 3.5*x^6 + 13*x^5 + 10.5*x^3 - 30*x^2" );
  a *= 5;
  assert ( a . degree () == 3
           && dumpMatch ( a, std::vector<double>{ -50.0, 17.5, 0.0, 5.0 } ) );

  a *= b;
  assert ( a . degree () == 8
           && dumpMatch ( a, std::vector<double>{ 0.0, 0.0, -150.0, 52.5, -0.0, 65.0, -17.5, -0.0, -5.0 } ) );

  assert ( a != b );
  b[5] = 0;
  assert ( static_cast<bool> ( b ) );
  assert ( ! ! b );
  b[2] = 0;
  assert ( !(a == b) );
  a *= 0;
  
  assert ( a . degree () == 0
           && dumpMatch ( a, std::vector<double>{ 0.0 } ) );
  assert ( a== b );
  assert ( ! static_cast<bool> ( b ) );
  assert ( ! b );


  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
