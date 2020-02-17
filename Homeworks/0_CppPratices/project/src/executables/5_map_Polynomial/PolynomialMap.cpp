#include "PolynomialMap.h"
#include <iterator>
#include <iostream>
//#include <experimental/iterator> // not implemented in MSVC19 yet
#include "infix_iterator.h"

using namespace std;

PolynomialMap::PolynomialMap(const PolynomialMap& other) {
    for (auto& t : other.m_Polynomial) {
        AddOneTerm(t);
    }
}

PolynomialMap::PolynomialMap(const string& file) {
    ReadFromFile(file);
}

PolynomialMap::PolynomialMap(const double* cof, const int* deg, int n) {
    for (int i = 0; i < n; i++)
        AddOneTerm(Term(deg[i], cof[i]));
}

PolynomialMap::PolynomialMap(const vector<int>& deg, const vector<double>& cof) {
    for (int i = 0; i < deg.size(); i++)
        AddOneTerm(Term(deg[i], cof[i]));
    
}

double PolynomialMap::coff(int i) const {
    auto it = m_Polynomial.find(i);
    if (it != m_Polynomial.end()) {
        return it->second;
    }
    else {
        return 0.;
    }
}

// modifiable?
double& PolynomialMap::coff(int i) {
    
    auto it = m_Polynomial.find(i);
    if (it != m_Polynomial.end()) {
        return (it->second);
    }
    else {
        m_Polynomial[i] = 0.0;
        return m_Polynomial[i];
    }
}

void PolynomialMap::compress() {
    auto it = m_Polynomial.begin();
    for (; it != m_Polynomial.end(); it++) {
        if (fabs(it->second) <= 1e-5) {
            // delete this
            //m_Polynomial.remove(*it);
            it = m_Polynomial.erase(it);
            if (it == m_Polynomial.end())
                break;
        }
    }
}

PolynomialMap PolynomialMap::operator+(const PolynomialMap& right) const {
    PolynomialMap res(*this);
    for (auto& t : right.m_Polynomial) {
        res.AddOneTerm(Term(t));
    }
    return res;
}

PolynomialMap PolynomialMap::operator-(const PolynomialMap& right) const {
    PolynomialMap res(*this);
    for (auto& t : right.m_Polynomial) {
        res.AddOneTerm(Term(t.first, -t.second));
    }
    return res;
}

PolynomialMap PolynomialMap::operator*(const PolynomialMap& right) const {
    PolynomialMap res;
    for (auto& p : right.m_Polynomial) {
        for (auto& q : right.m_Polynomial) {
            res.AddOneTerm(Term(p.first + q.first, p.second + q.second));
        }
    }
    return res;
}

PolynomialMap& PolynomialMap::operator=(const PolynomialMap& right) {
    this->m_Polynomial = right.m_Polynomial;
    return *this;
}

void PolynomialMap::Print() const {
    // compress first?
    std::copy(std::begin(m_Polynomial), 
        std::end(m_Polynomial),
        infix_ostream_iterator<Term>(std::cout, " + "));
    std::cout << std::endl;
}

bool PolynomialMap::ReadFromFile(const string& file) {
    FILE *fp = fopen(file.c_str(), "r+");
    if (fp == NULL) {
        return false;
    }

    int num = 0;
    if (fscanf(fp, "P %d", &num) != 1) {
        fprintf(stderr, "Read invalid.\n");
        return false;
    }
    for (int i = 0; i < num; i++) {
        double cof;
        int deg;
        if (fscanf(fp, "%d %lf", &deg, &cof) != 2) {
            fprintf(stderr, "Read invalid at term %d.\n", i);
            return false;
        }
        AddOneTerm(Term(deg, cof));
    }

    return true;
}

// Polynomial Map is in ascending order
//**HOWEVER, THE REFERENCE RETURNED IS NOT MODIFIABLE
const PolynomialMap::Term& PolynomialMap::AddOneTerm(const Term& term) {
    auto it = m_Polynomial.find(term.deg);
    if (it != m_Polynomial.end()) {
        it->second += term.cof;
    } else {
        m_Polynomial[term.deg] = term.cof;
    }
    return Term(term.deg, term.cof);
}
