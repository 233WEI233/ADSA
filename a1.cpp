#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class BigNum {
private:
    // digits are stored in normal order, e.g. "123" -> {1,2,3}
    vector<int> digits;
    int base;

    void trim() {
        while (digits.size() > 1 && digits[0] == 0) {
            digits.erase(digits.begin());
        }
    }

    static vector<int> trimVector(const vector<int>& v) {
        vector<int> res = v;
        while (res.size() > 1 && res[0] == 0) {
            res.erase(res.begin());
        }
        return res;
    }

    static int compareVectors(const vector<int>& a, const vector<int>& b) {
        vector<int> aa = trimVector(a);
        vector<int> bb = trimVector(b);

        if (aa.size() < bb.size()) return -1;
        if (aa.size() > bb.size()) return 1;

        for (size_t i = 0; i < aa.size(); i++) {
            if (aa[i] < bb[i]) return -1;
            if (aa[i] > bb[i]) return 1;
        }
        return 0;
    }

    static vector<int> addVectors(const vector<int>& a, const vector<int>& b, int base) {
        int i = (int)a.size() - 1;
        int j = (int)b.size() - 1;
        int carry = 0;
        vector<int> res;

        while (i >= 0 || j >= 0 || carry > 0) {
            int x = (i >= 0 ? a[i] : 0);
            int y = (j >= 0 ? b[j] : 0);
            int sum = x + y + carry;
            res.push_back(sum % base);
            carry = sum / base;
            i--;
            j--;
        }

        reverse(res.begin(), res.end());
        return trimVector(res);
    }

    // assume a >= b
    static vector<int> subtractVectors(const vector<int>& a, const vector<int>& b, int base) {
        int i = (int)a.size() - 1;
        int j = (int)b.size() - 1;
        int borrow = 0;
        vector<int> res;

        while (i >= 0) {
            int x = a[i] - borrow;
            int y = (j >= 0 ? b[j] : 0);

            if (x < y) {
                x += base;
                borrow = 1;
            } else {
                borrow = 0;
            }

            res.push_back(x - y);
            i--;
            j--;
        }

        reverse(res.begin(), res.end());
        return trimVector(res);
    }

    static vector<int> shiftLeft(const vector<int>& v, int m) {
        if (v.size() == 1 && v[0] == 0) return v;
        vector<int> res = v;
        for (int i = 0; i < m; i++) {
            res.push_back(0);
        }
        return res;
    }

    static vector<int> padLeft(const vector<int>& v, int n) {
        if ((int)v.size() >= n) return v;
        vector<int> res(n - (int)v.size(), 0);
        res.insert(res.end(), v.begin(), v.end());
        return res;
    }

    static vector<int> slice(const vector<int>& v, int l, int r) {
        if (l >= r) return {0};
        vector<int> res(v.begin() + l, v.begin() + r);
        return trimVector(res);
    }

    static vector<int> schoolMultiply(const vector<int>& a, const vector<int>& b, int base) {
        int n = (int)a.size();
        int m = (int)b.size();
        vector<int> res(n + m, 0);

        for (int i = n - 1; i >= 0; i--) {
            int carry = 0;
            for (int j = m - 1; j >= 0; j--) {
                int cur = res[i + j + 1] + a[i] * b[j] + carry;
                res[i + j + 1] = cur % base;
                carry = cur / base;
            }
            res[i] += carry;
        }

        // normalize
        for (int i = (int)res.size() - 1; i > 0; i--) {
            if (res[i] >= base) {
                res[i - 1] += res[i] / base;
                res[i] %= base;
            }
        }

        return trimVector(res);
    }

    static vector<int> karatsubaMultiply(const vector<int>& x, const vector<int>& y, int base) {
        vector<int> a = trimVector(x);
        vector<int> b = trimVector(y);

        if ((a.size() == 1 && a[0] == 0) || (b.size() == 1 && b[0] == 0)) {
            return {0};
        }

        int n = max((int)a.size(), (int)b.size());

        // small case: use school multiplication
        if (n <= 4) {
            return schoolMultiply(a, b, base);
        }

        if (n % 2 == 1) n++;
        a = padLeft(a, n);
        b = padLeft(b, n);

        int m = n / 2;

        vector<int> aHigh = slice(a, 0, m);
        vector<int> aLow  = slice(a, m, n);
        vector<int> bHigh = slice(b, 0, m);
        vector<int> bLow  = slice(b, m, n);

        vector<int> z0 = karatsubaMultiply(aLow, bLow, base);
        vector<int> z2 = karatsubaMultiply(aHigh, bHigh, base);

        vector<int> sumA = addVectors(aHigh, aLow, base);
        vector<int> sumB = addVectors(bHigh, bLow, base);

        vector<int> z1 = karatsubaMultiply(sumA, sumB, base);
        z1 = subtractVectors(z1, z2, base);
        z1 = subtractVectors(z1, z0, base);

        vector<int> part1 = shiftLeft(z2, 2 * m);
        vector<int> part2 = shiftLeft(z1, m);

        vector<int> result = addVectors(addVectors(part1, part2, base), z0, base);
        return trimVector(result);
    }

public:
    BigNum(string s = "0", int b = 10) : base(b) {
        for (char c : s) {
            digits.push_back(c - '0');
        }
        trim();
    }

    BigNum(const vector<int>& v, int b) : digits(v), base(b) {
        trim();
    }

    string toString() const {
        string s;
        for (int d : digits) {
            s.push_back(char('0' + d));
        }
        return s.empty() ? "0" : s;
    }

    int compare(const BigNum& other) const {
        return compareVectors(this->digits, other.digits);
    }

    BigNum add(const BigNum& other) const {
        return BigNum(addVectors(this->digits, other.digits, base), base);
    }

    // assume *this >= other
    BigNum subtract(const BigNum& other) const {
        return BigNum(subtractVectors(this->digits, other.digits, base), base);
    }

    BigNum multiply(const BigNum& other) const {
        return BigNum(karatsubaMultiply(this->digits, other.digits, base), base);
    }

    BigNum divide(const BigNum& other) const {
        // division by zero protection
        if (other.digits.size() == 1 && other.digits[0] == 0) {
            return BigNum("0", base);
        }

        if (this->compare(other) < 0) {
            return BigNum("0", base);
        }

        vector<int> quotient;
        BigNum remainder("0", base);

        for (size_t i = 0; i < digits.size(); i++) {
            // remainder = remainder * base + digits[i]
            if (!(remainder.digits.size() == 1 && remainder.digits[0] == 0)) {
                remainder.digits.push_back(digits[i]);
            } else {
                remainder.digits[0] = digits[i];
            }
            remainder.trim();

            int qDigit = 0;
            while (remainder.compare(other) >= 0) {
                remainder = remainder.subtract(other);
                qDigit++;
            }
            quotient.push_back(qDigit);
        }

        return BigNum(quotient, base);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string I1, I2;
    int B;
    cin >> I1 >> I2 >> B;

    BigNum a(I1, B);
    BigNum b(I2, B);

    BigNum sum = a.add(b);
    BigNum product = a.multiply(b);
    BigNum quotient = a.divide(b);

    cout << sum.toString() << " "
         << product.toString() << " "
         << quotient.toString() << "\n";

    return 0;
}