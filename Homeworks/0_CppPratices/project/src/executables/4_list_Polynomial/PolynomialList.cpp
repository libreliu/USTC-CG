#include "PolynomialList.h"

using namespace std;

PolynomialList::PolynomialList()
{
    /// do nothing
}

PolynomialList::PolynomialList(const PolynomialList &other)
{
    // TODO
}

PolynomialList::PolynomialList(const string &file)
{
    // TODO
}

PolynomialList::PolynomialList(const double *cof, const int *deg, int n)
{
    // TODO
}

PolynomialList::PolynomialList(const vector<int> &deg, const vector<double> &cof)
{
    // TODO
}

PolynomialList::~PolynomialList()
{
    // TODO
}

double PolynomialList::coff(int i) const
{
    // TODO
    return 0.;
}

double &PolynomialList::coff(int i)
{
    // TODO
    static double error; // you should delete this line
    return error;
}

void PolynomialList::compress()
{
    // TODO
}

PolynomialList PolynomialList::operator+(const PolynomialList &right) const
{
    // TODO
    return {};
}

PolynomialList PolynomialList::operator-(const PolynomialList &right) const
{
    // TODO
    return {};
}

PolynomialList PolynomialList::operator*(const PolynomialList &right) const
{
    // TODO
    return {};
}

PolynomialList &PolynomialList::operator=(const PolynomialList &right)
{
    // TODO
    return *this;
}

void PolynomialList::Print() const
{
    // TODO
}
