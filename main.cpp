#include <bits/stdc++.h>
#include "json.hpp" 
using namespace std;
using json = nlohmann::json;

// ---------------- BIGINT IMPLEMENTATION ----------------
struct BigInt {
    vector<int> d; // base-10, reversed
    bool neg = false;

    BigInt(long long num = 0) { *this = num; }
    BigInt& operator=(long long num) {
        neg = false;
        if (num < 0) { neg = true; num = -num; }
        d.clear();
        if (num == 0) d.push_back(0);
        while (num > 0) { d.push_back(num % 10); num /= 10; }
        return *this;
    }
    BigInt(const string &s) { fromString(s); }

    void fromString(const string &s) {
        neg = false;
        d.clear();
        int start = 0;
        if (s[0] == '-') { neg = true; start = 1; }
        for (int i = s.size() - 1; i >= start; --i)
            d.push_back(s[i] - '0');
        trim();
    }
    static BigInt fromBase(const string &val, int base) {
        BigInt res(0), b(1);
        for (int i = val.size() - 1; i >= 0; --i) {
            int digit;
            if (isdigit(val[i])) digit = val[i] - '0';
            else if (isalpha(val[i])) digit = tolower(val[i]) - 'a' + 10;
            else throw runtime_error("Invalid char");
            res = res + (b * digit);
            b = b * base;
        }
        return res;
    }
    void trim() {
        while (d.size() > 1 && d.back() == 0) d.pop_back();
        if (d.size() == 1 && d[0] == 0) neg = false;
    }
    string str() const {
        string s;
        if (neg && !(d.size() == 1 && d[0] == 0)) s.push_back('-');
        for (int i = d.size() - 1; i >= 0; --i) s.push_back('0' + d[i]);
        return s;
    }
    // Addition
    BigInt operator+(const BigInt &o) const {
        if (neg == o.neg) {
            BigInt res;
            res.neg = neg;
            int carry = 0;
            res.d.resize(max(d.size(), o.d.size()) + 1, 0);
            for (size_t i = 0; i < res.d.size(); i++) {
                int sum = carry;
                if (i < d.size()) sum += d[i];
                if (i < o.d.size()) sum += o.d[i];
                res.d[i] = sum % 10;
                carry = sum / 10;
            }
            res.trim();
            return res;
        }
        // handle different signs
        BigInt a = *this, b = o;
        if (a.neg) { a.neg = false; return b - a; }
        if (b.neg) { b.neg = false; return a - b; }
        return BigInt(); // dummy
    }
    // Subtraction (assuming *this and o are positive, *this >= o)
    BigInt operator-(const BigInt &o) const {
        if (neg != o.neg) { BigInt tmp = o; tmp.neg = !o.neg; return *this + tmp; }
        if (*this == o) return BigInt(0);
        bool sign = false;
        BigInt a = *this, b = o;
        if (absLess(a, b)) { swap(a, b); sign = true; }
        BigInt res;
        res.d.resize(a.d.size(), 0);
        int carry = 0;
        for (size_t i = 0; i < a.d.size(); i++) {
            int diff = a.d[i] - carry - (i < b.d.size() ? b.d[i] : 0);
            if (diff < 0) { diff += 10; carry = 1; } else carry = 0;
            res.d[i] = diff;
        }
        res.neg = sign;
        res.trim();
        return res;
    }
    // Multiply by int
    BigInt operator*(int m) const {
        BigInt res;
        res.neg = (neg != (m < 0));
        long long mm = m < 0 ? -1LL * m : m;
        res.d.assign(d.size() + 10, 0);
        long long carry = 0;
        for (size_t i = 0; i < d.size(); i++) {
            long long cur = carry + 1LL * d[i] * mm;
            res.d[i] = cur % 10;
            carry = cur / 10;
        }
        size_t idx = d.size();
        while (carry) { res.d[idx++] = carry % 10; carry /= 10; }
        res.trim();
        return res;
    }
    // Multiply BigInts
    BigInt operator*(const BigInt &o) const {
        BigInt res;
        res.neg = (neg != o.neg);
        res.d.assign(d.size() + o.d.size(), 0);
        for (size_t i = 0; i < d.size(); i++) {
            int carry = 0;
            for (size_t j = 0; j < o.d.size() || carry; j++) {
                long long cur = res.d[i + j] +
                   d[i] * 1LL * (j < o.d.size() ? o.d[j] : 0) + carry;
                res.d[i + j] = cur % 10;
                carry = cur / 10;
            }
        }
        res.trim();
        return res;
    }
    // Divide by int
    BigInt operator/(long long v) const {
        BigInt res;
        res.neg = (neg != (v < 0));
        long long vv = v < 0 ? -v : v;
        res.d.assign(d.size(), 0);
        long long rem = 0;
        for (int i = (int)d.size() - 1; i >= 0; --i) {
            long long cur = d[i] + rem * 10;
            res.d[i] = cur / vv;
            rem = cur % vv;
        }
        res.trim();
        return res;
    }
    bool operator==(const BigInt &o) const { return neg==o.neg && d == o.d; }
    static bool absLess(const BigInt &a, const BigInt &b) {
        if (a.d.size() != b.d.size()) return a.d.size() < b.d.size();
        for (int i = (int)a.d.size()-1; i >= 0; --i)
            if (a.d[i] != b.d[i]) return a.d[i] < b.d[i];
        return false;
    }
};

// -------------------- MAIN ------------------------
int main() {
    ifstream fin("data.json");
    if (!fin) { cerr << "Could not open input.json\n"; return 1; }
    json j; fin >> j;

    int k = j["keys"]["k"];
    vector<int> x(k);
    vector<BigInt> y(k);

    int idx = 0;
    for (auto &el : j.items()) {
        if (el.key() == "keys") continue;
        int xi = stoi(el.key());
        int base = stoi(el.value()["base"].get<string>());
        string valStr = el.value()["value"];
        y[idx] = BigInt::fromBase(valStr, base);
        x[idx] = xi;
        idx++;
        if (idx == k) break; // only k points
    }

    BigInt secret(0);

    // Lagrange interpolation at x=0
    for (int i = 0; i < k; i++) {
        BigInt numerator(1);
        long long denominator = 1;
        for (int j2 = 0; j2 < k; j2++) {
            if (i != j2) {
                numerator = numerator * (-x[j2]);
                denominator *= (x[i] - x[j2]);
            }
        }
        BigInt term = numerator / denominator; // divide BigInt by small int
        term = term * (y[i]);
        secret = secret + term;
    }

    cout << "Secret C = " << secret.str() << "\n";
    return 0;
}
