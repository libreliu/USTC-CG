#include "PolynomialList.h"
#include <iterator>
#include <iostream>
//#include <experimental/iterator> // not implemented in MSVC19 yet
#include "infix_iterator.h"

using namespace std;

PolynomialList::PolynomialList(const PolynomialList& other) {
    for (auto& t : other.m_Polynomial) {
        AddOneTerm(t);
    }
}

PolynomialList::PolynomialList(const string& file) {
    ReadFromFile(file);
}

PolynomialList::PolynomialList(const double* cof, const int* deg, int n) {
    for (int i = 0; i < n; i++)
        AddOneTerm(Term(deg[i], cof[i]));
}

PolynomialList::PolynomialList(const vector<int>& deg, const vector<double>& cof) {
    for (int i = 0; i < deg.size(); i++)
        AddOneTerm(Term(deg[i], cof[i]));
    
}

double PolynomialList::coff(int i) const {
    for (auto& t : m_Polynomial) {
        if (t.deg == i) {
            return t.cof;
        }
    }
    return 0.;
}

// modifiable?
double& PolynomialList::coff(int i) {
    
    for (auto& t : m_Polynomial) {
        if (t.deg == i) {
            return t.cof;
        }
    }

    return AddOneTerm(Term(i, 0.0)).cof;
}

void PolynomialList::compress() {
    auto it = m_Polynomial.begin();
    for (; it != m_Polynomial.end(); it++) {
        if (fabs(it->cof) <= 1e-5) {
            // delete this
            //m_Polynomial.remove(*it); // 这个的问题是我没有 const _Ty [ with _Ty = Term ] 到 Term 的转换
            it = m_Polynomial.erase(it);
            if (it == m_Polynomial.end())
                break;
        }
    }
}

PolynomialList PolynomialList::operator+(const PolynomialList& right) const {
    PolynomialList res(*this);
    for (auto& t : right.m_Polynomial) {
        res.AddOneTerm(t);
    }
    return res;
}

PolynomialList PolynomialList::operator-(const PolynomialList& right) const {
    PolynomialList res(*this);
    for (auto& t : right.m_Polynomial) {
        res.AddOneTerm(Term(t.deg, -t.cof));
    }
    return res;
}

PolynomialList PolynomialList::operator*(const PolynomialList& right) const {
    PolynomialList res;
    for (auto& p : right.m_Polynomial) {
        for (auto& q : right.m_Polynomial) {
            res.AddOneTerm(Term(p.deg + q.deg, p.cof + q.cof));
        }
    }
    return res;
}

PolynomialList& PolynomialList::operator=(const PolynomialList& right) {
    this->m_Polynomial = right.m_Polynomial;
    return *this;
}

void PolynomialList::Print() const {
    // compress first?
    std::copy(std::begin(m_Polynomial), 
        std::end(m_Polynomial),
        infix_ostream_iterator<Term>(std::cout, " + "));
    std::cout << std::endl;
}

// fscanf is actually safe here
#define _CRT_SECURE_NO_WARNINGS
bool PolynomialList::ReadFromFile(const string& file) {
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
#undef _CRT_SECURE_NO_WARNINGS

// Polynomial list is in ascending order
PolynomialList::Term& PolynomialList::AddOneTerm(const Term& term) {
    auto it = m_Polynomial.begin();
    for (; it != m_Polynomial.end(); it++) {
        if (it->deg == term.deg) {
            it->cof += term.cof;
            return *it;
        }
        else if (it->deg > term.deg) {
            break;
        }
    }

    // insert before it
    return *(m_Polynomial.insert(it, term));
}
