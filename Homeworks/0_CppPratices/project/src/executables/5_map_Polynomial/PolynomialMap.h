#pragma once
#ifndef POLYNOMIALMap_H
#define POLYNOMIALMap_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

class PolynomialMap
{
public:
    PolynomialMap() { };
    PolynomialMap(const PolynomialMap& other);
    PolynomialMap(const std::string& file); // initialization using file
    PolynomialMap(const double* cof, const int* deg, int n);
    PolynomialMap(const std::vector<int>& deg, const std::vector<double>& cof);

    double& coff(int i);
    double coff(int i) const;

    void compress();

    // overload
    PolynomialMap operator+(const PolynomialMap& right) const; //Overload operator +
    PolynomialMap operator-(const PolynomialMap& right) const; //Overload operator -
    PolynomialMap operator*(const PolynomialMap& right) const; //Overload operator *
    PolynomialMap& operator=(const PolynomialMap& right); //Overload operator =

    void Print() const;

private:
    struct Term {
        int deg;
        double cof;

        Term(int deg, double cof) : deg(deg), cof(cof) { }
        Term() : Term(0, 0) { }
        friend std::ostream& operator<<(std::ostream& os, const Term& t) {
            os << t.cof <<  "* x^" << t.deg;
            return os;
        }

        Term(const std::pair<const int, double>& p) : deg(p.first), cof(p.second) { }
    };
    bool ReadFromFile(const std::string& file);
    const Term& AddOneTerm(const Term& term); // add one term into m_Polynomial

private:
    std::map<int, double> m_Polynomial;
};

#endif // POLYNOMIALMap_H
