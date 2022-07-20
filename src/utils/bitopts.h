#ifndef _H_BITOPS
#define _H_BITOPS
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>

inline void assert(uint64_t cond)
{
    if (!cond) {
        printf("Assertion failed: \n");
    }
}
inline void assert2(uint64_t x, uint64_t y)
{
    if (x != y) {
        printf("Assertion failed: \n");
    }
}

class BitOps {

  public:
    static uint64_t xor_b(uint64_t x, uint64_t y)
    {
        uint64_t ret = x ^ y;
        if (ret >= 0)
            return ret;
        else
            return ret + 0x100000000;
    }

    static uint64_t xor64_b(uint64_t x, uint64_t y)
    {
        uint64_t xh = floor(x / 0x100000000);
        uint64_t yh = floor(y / 0x100000000);
        uint64_t xl = x % 0x100000000;
        uint64_t yl = y % 0x100000000;
        return xor_b(xh, yh) * 0x100000000 + xor_b(xl, yl);
    }

    static uint64_t and_b(uint64_t x, uint64_t y)
    {
        uint64_t ret = x & y;
        if (ret >= 0)
            return ret;
        else
            return ret + 0x100000000;
    }

    static uint64_t and64_b(uint64_t x, uint64_t y)
    {
        int64_t xh = floor(x / 0x100000000);
        int64_t yh = floor(y / 0x100000000);
        int64_t xl = x % 0x100000000;
        int64_t yl = y % 0x100000000;
        return and_b(xh, yh) * 0x100000000 + and_b(xl, yl);
    }

    static uint64_t or_b(uint64_t x, uint64_t y)
    {
        uint64_t ret = x | y;
        if (ret >= 0)
            return ret;
        else
            return ret + 0x100000000;
    }

    static uint64_t or64_b(uint64_t x, uint64_t y)
    {
        int64_t xh = floor(x / 0x100000000);
        int64_t yh = floor(y / 0x100000000);
        int64_t xl = x % 0x100000000;
        int64_t yl = y % 0x100000000;
        return or_b(xh, yh) * 0x100000000 + or_b(xl, yl);
    }

    static uint64_t not_b(uint64_t x)
    {
        int32_t ret = ~x;
        if (ret >= 0)
            return ret;
        else
            return ret + 0x100000000;
    }

    static uint64_t lowest_set_bit(uint64_t val, uint64_t len)
    {
        for (uint64_t i = 0; i < len; i++) {
            if (val & 1 << i)
                return i;
        }
        return len;
    }

    static uint64_t bit_count(uint64_t val, uint64_t len)
    {
        int64_t count = 0;
        for (int64_t i = 0; i < len; i++) {
            if (val & 1 << i)
                count++;
        }
        return count;
    }

    static uint64_t clear_bit(uint64_t uint, uint64_t pos)
    {
        if (uint < 0x80000000 && pos < 31)
            return uint & ~(1 << pos);
        if (pos < 31) {
            int64_t ret = uint & ~(1 << pos);
            if (ret < 0)
                ret += 0x100000000;
            return ret;
        } else {
            if (uint >= 0x80000000)
                return uint - 0x80000000;
            else
                return uint;
        }
    }

    static uint64_t clear_bits(uint64_t uint, uint64_t start, uint64_t end)
    {
        if (uint < 0x80000000 && start < 31)
            return uint & ~(((1 << (start + 1)) - 1) & ~((1 << end) - 1));
        if (start < 31) {
            int64_t ret = uint & ~(((1 << (start + 1)) - 1) & ~((1 << end) - 1));
            if (ret < 0)
                ret += 0x100000000;
            return ret;
        }
        uint64_t rval2 = 0;
        for (uint64_t i = 0; i < 32; i++) {
            if (!((32 - i - 1) <= start && (32 - i - 1) >= end)) {
                if ((uint >> (31 - i)) & 0x1) {
                    rval2 += 1 << (31 - i);
                }
            }
        }
        return rval2;
    }

    static uint64_t set_bits(uint64_t uint, uint64_t start, uint64_t end, uint64_t val)
    {
        return or_b(clear_bits(uint, start, end), lsl(val, end));
    }

    static uint64_t set_bit(uint64_t uint, uint64_t pos, uint64_t val)
    {
        if (val)
            if (pos == 31)
                return or_b(uint, 0x80000000);
            else
                return or_b(uint, val << pos);
        else if (pos == 31)
            return clear_bit(uint, 31);
        else
            return and_b(uint, not_b(1 << pos));
    }

    static uint64_t get_bit(uint64_t uint, uint64_t pos, uint64_t dummy)
    {
        return (uint & (1 << pos)) >> pos;
    }

    static uint64_t get_bit64(uint64_t ulong, uint64_t pos, uint64_t dummy)
    {
        if (pos > 31) {
            uint64_t ulong_h = floor(ulong / 0x100000000);
            return get_bit(ulong_h, pos - 31, 0);
        } else {
            uint64_t ulong_l = ulong % 0x100000000;
            return get_bit(ulong_l, pos, 0);
        }
    }

    static uint64_t get_bits(uint64_t uint, uint64_t start, uint64_t end)
    {
        if (start == 31) {
            if (end != 0)
                return uint >> end;
            if (uint > 0xffffffff)
                and_b(uint, 0xffffffff);
            else
                return uint;
        }
        uint64_t ret = (uint >> end) & ((1 << (start - end + 1)) - 1);
        if (ret >= 0x100000000)
            return ret - 0x100000000;
        else
            return ret;
    }

    static uint64_t get_bits64(uint64_t ulong, uint64_t start, uint64_t end)
    {
        if (ulong < 0x80000000 && start < 31 && end < 31)
            get_bits(ulong, start, end);

        uint64_t ulong_h = floor(ulong / 0x100000000);
        uint64_t ulong_l = ulong % 0x100000000;
        uint64_t ret     = 0;
        if (start > 31) {
            if (start == 32) {
                ret += get_bit(ulong_h, 0, 0) << (31 - end + 1);
            } else {
                if (end > 31)
                    ret += get_bits(ulong_h, start - 32, end - 32);
                else
                    ret += get_bits(ulong_h, start - 31, 0) << (31 - end + 1);
            }
        }
        if (end <= 31) {
            if (end == 31)
                ret += get_bit(ulong_l, 31, 0);
            else
                ret += get_bits(ulong_l, start < 31 ? start : 31, end);
        }
        return ret;
    }

    static uint64_t sign_extend(uint64_t x, uint64_t x_len, uint64_t n)
    {
        uint64_t sign = get_bit(x, x_len - 1, 0);
        if (sign) {
            uint64_t tmp = 0;
            if (n == 32)
                tmp = 0xffffffff;
            else
                tmp = (1 << n) - 1;

            uint64_t ret = x | (tmp & ~((1 << x_len) - 1));
            if (ret < 0)
                return ret + 0x100000000;
            else
                return ret;
        } else
            return x;
    }

    static uint64_t lsl(uint64_t x, uint64_t n)
    {
        int32_t ret = x << n;
        if (ret >= 0 && ret >= x) {
            return ret;
        } else {
            return x * pow(2, n);
        }
    }

    static uint64_t lsr(uint64_t x, uint64_t n)
    {
        return (n == 32) ? 0 : x >> n;
    }

    static uint64_t asr(uint64_t x, uint64_t n)
    {
        if (n == 32)
            return 0;

        int64_t x64 = x;
        int64_t n64 = n;
        int32_t x32 = x;
        int32_t n32 = n;

        // uint64_t ret    = x >> n;
        // int64_t  reti   = x >> n;
        int32_t ret  = x32 >> n32;
        bool    flg  = ret < 0;
        int64_t rret = ret + 0x100000000;

        if (flg) {
            return rret;
        }
        return ret;
    }

    static uint64_t sint32(uint64_t x)
    {
        return x & 0xffffffff;
    }

    static uint64_t uint32(uint64_t x)
    {
        return and64_b(x, 0xffffffff);
    }

    static uint64_t toUint32(uint64_t x)
    {
        // if (x < 0) {
        //     if (x < (1 << 31)) {
        //         x = x + 0x10000000000000000;
        //     } else {
        //         x = x + 0x100000000;
        //     }
        // }
        return x;    // and64_b(x, 0xffffffff);
    }

    static uint64_t copy_bits(uint64_t dest, uint64_t start, uint64_t end, uint64_t src)
    {
        return set_bits(dest, start, end, get_bits(src, start, end));
    }

    static uint64_t copy_bit(uint64_t dest, uint64_t pos, uint64_t src)
    {
        return set_bit(dest, pos, get_bit(src, pos, 0));
    }

    static uint64_t ror(uint64_t value, uint64_t amount)
    {
        uint64_t valueu = value;
        uint64_t m      = amount % 32;
        uint64_t lo     = value & ((1 << m) - 1);
        uint64_t result = (valueu >> m) + lsl(lo, (32 - m));
        return result;
    }

    static uint64_t count_leading_zero_bits(uint64_t val)
    {
        uint64_t n = 0;
        for (int i = 31; i >= 0; i--) {
            if (get_bit(val, i, 0))
                break;
            n++;
        }
        return n;
    }

    static void test()
    {
        assert2(clear_bit(0xffffffff, 0), 0xfffffffe);
        assert2(clear_bit(0x13, 31), 0x13);
        assert2(clear_bit(0x13, 0), 0x12);

        assert2(clear_bits(0xffffffff, 31, 0), 0);
        assert2(clear_bits(0xffffffff, 31, 16), 0x0000ffff);
        assert2(clear_bits(0xffffffff, 15, 0), 0xffff0000);
        assert2(clear_bits(0xffffffff, 15, 12), 0xffff0fff);
        assert2(clear_bits(0x0fffffff, 15, 12), 0x0fff0fff);

        uint64_t tmp = 0;
        assert(xor_b(0xffffffff, 0xffffffff) == 0);
        assert(xor_b(0x11111111, 0x22222222) == 0x33333333);
        assert(xor_b(0xf0000000, 0xf0000000) == 0);

        assert(xor64_b(0xffffffff, 0xffffffff) == 0);
        assert(xor64_b(0x11111111, 0x22222222) == 0x33333333);
        assert(xor64_b(0xf0000000, 0xf0000000) == 0);
        assert(xor64_b(0x1f0000000, 0xf0000000) == 0x100000000);

        assert(not_b(0xffffffff) == 0x00000000);
        assert(not_b(0x00000000) == 0xffffffff);
        assert(not_b(0x00000001) == 0xfffffffe);
        assert(not_b(0x80000000) == 0x7fffffff);

        assert(or_b(0x11111111, 0x22222222) == 0x33333333);
        assert(or_b(0xffffffff, 0x00000000) == 0xffffffff);
        assert(or_b(0xffffffff, 0xffffffff) == 0xffffffff);

        assert(or64_b(0x11111111, 0x22222222) == 0x33333333);
        assert(or64_b(0xffffffff, 0x00000000) == 0xffffffff);
        assert(or64_b(0xffffffff, 0xffffffff) == 0xffffffff);
        assert(or64_b(0xf00000000, 0x00000000) == 0xf00000000);
        assert(or64_b(0xf00000000, 0x0000000f) == 0xf0000000f);

        assert(and_b(0x11111111, 0x22222222) == 0);
        assert(and_b(0xffffffff, 0) == 0);

        assert(and64_b(0x11111111, 0x22222222) == 0);
        assert2(and64_b(0xffffffff, 0), 0);
        assert2(and64_b(0xffffffffffff, 0), 0);
        assert2(and64_b(0xffffffffffff, 0xffffffff), 0xffffffff);

        assert2(get_bit(0xffffffff, 31, 0), 1);
        assert2(get_bit(0xffffffff, 0, 0), 1);
        assert(get_bit(0x80000000, 31, 0) == 1);
        assert(get_bit(0, 31, 0) == 0);
        assert(get_bit(0, 0, 0) == 0);
        assert(get_bit(0x7fffffff, 31, 0) == 0);
        assert2(get_bit(0x80000000, 31, 0), 1);

        assert(get_bit64(0xffffffff, 31, 0) == 1);
        assert2(get_bit64(0xffffffff, 0, 0), 1);
        assert(get_bit64(0x80000000, 31, 0) == 1);
        assert(get_bit64(0, 31, 0) == 0);
        assert(get_bit64(0, 0, 0) == 0);
        assert(get_bit64(0x7fffffff, 31, 0) == 0);
        assert(get_bit64(0xffffffffffff, 31, 0) == 1);
        assert2(get_bit64(0xffffffffffff, 50, 0), 0);

        assert(get_bits(0xffffffff, 31, 0) == 0xffffffff);
        assert(get_bits(0xffffffff, 31, 16) == 0xffff);
        assert(get_bits(0, 31, 0) == 0);
        assert(get_bits(0x13, 4, 0) == 0x13);
        assert2(get_bits(0xf0000000, 31, 27), 0x1e);
        assert2(get_bits(0xc0000000, 31, 27), 0x18);

        assert2(get_bits64(0xffffffff, 31, 0), 0xffffffff);
        assert2(get_bits64(0xffffffff, 31, 16), 0xffff);
        assert2(get_bits64(0, 31, 0), 0);
        assert2(get_bits64(0x13, 4, 0), 0x13);
        assert2(get_bits64(0x100000000, 31, 0), 0);
        assert2(get_bits64(0x100000000, 31, 0), 0);
        assert2(get_bits64(0x100000000, 32, 31), 2);
        assert2(get_bits64(0x300000000, 32, 31), 2);
        assert2(get_bits64(0x180000000, 32, 31), 3);
        assert2(get_bits64(0xf00000000, 33, 32), 3);
        assert2(get_bits64(0xf00000000, 34, 33), 3);
        assert2(get_bits64(0x180000000, 34, 31), 3);
        assert2(get_bits64(0x180000000, 34, 30), 6);
        assert2(get_bits64(0x100000000, 51, 32), 1);

        assert(set_bit(0xffffffff, 0, 0) == 0xfffffffe);
        assert(set_bit(0xffffffff, 31, 0) == 0x7fffffff);
        assert(set_bit(0xffffffff, 31, 1) == 0xffffffff);
        assert(set_bit(0x13, 31, 0) == 0x13);
        assert(set_bit(0, 31, 1) == 0x80000000);
        assert(set_bit(0, 0, 1) == 1);
        assert(set_bit(0, 2, 1) == 4);

        assert(set_bits(0xffffffff, 31, 0, 0) == 0);
        assert(set_bits(0xffffffff, 15, 0, 0) == 0xffff0000);
        assert(set_bits(0, 4, 0, 0x13) == 0x13);
        assert2(set_bits(0xf0000000, 31, 27, 0x1e), 0xf0000000);
        assert2(set_bits(0x00000000, 31, 27, 0x1e), 0xf0000000);
        assert2(set_bits(0xf0000000, 31, 27, 0x18), 0xc0000000);

        assert2(lsl(1, 1), 2);
        assert2(lsl(0xf0000000, 1), 0x1e0000000);
        assert2(lsl(0xffffffff, 1), 0x1fffffffe);
        assert2(lsl(0xf0f0f0f0, 4), 0xf0f0f0f00);
        assert2(lsl(0x100000000, 1), 0x200000000);

        assert2(lsr(1, 1), 0);
        assert2(lsr(0xf0000000, 1), 0x78000000);
        assert2(lsr(0xffffffff, 1), 0x7fffffff);
        assert2(lsr(0xf0f0f0f0, 4), 0x0f0f0f0f);
        assert2(lsr(0x80000000, 32), 0);
        assert2(lsr(0x80000000, 1), 0x40000000);

        assert2(lsr(1, 1), 0);
        assert2(lsr(0xf0000000, 1), 0x78000000);
        assert2(lsr(0xffffffff, 1), 0x7fffffff);
        assert2(lsr(0xf0f0f0f0, 4), 0x0f0f0f0f);
        assert2(lsr(0x80000000, 32), 0);
        assert2(lsr(0x80000000, 1), 0x40000000);

        assert2(sint32(0x00000000), 0x00000000);
        assert2(sint32(0x80000000), 0x80000000 & 0xffffffff);
        assert2(sint32(0x100000000), 0x00000000);

        assert2(uint32(0x00000000), 0x00000000);
        assert2(uint32(0x80000000), 0x80000000);
        assert2(uint32(0x100000000), 0x00000000);
        assert2(uint32(0xffffffff), 0xffffffff);
        assert2(uint32(0xfffffffff), 0xffffffff);

        assert2(sign_extend(0, 26, 32), 0);
        assert2(sign_extend(0, 1, 32), 0);
        assert2(sign_extend(1, 1, 32), 0xffffffff);
        assert2(sign_extend(0x0000ffff, 16, 32), 0xffffffff);
        assert2(sign_extend(0x00007fff, 16, 32), 0x00007fff);
        assert2(sign_extend(0xffffe3 << 2, 26, 32), 0xffffff8c);

        assert2(copy_bits(0xf0000000, 31, 27, 0), 0);
        assert2(copy_bits(0xf0000000, 31, 27, 0xc0000000), 0xc0000000);

        assert2(copy_bit(0, 0, 1), 1);
        assert2(copy_bit(1, 0, 0), 0);
        assert2(copy_bit(0xffffffff, 0, 0), 0xfffffffe);
        assert2(copy_bit(0xffffffff, 31, 0), 0x7fffffff);

        assert2(ror(0x10000000, 1), 0x08000000);
        assert2(ror(0x10000001, 1), 0x88000000);
        assert2(ror(0xffffffff, 1), 0xffffffff);
        assert2(ror(0x0000ffff, 16), 0xffff0000);
        assert2(ror(0x000ffff0, 16), 0xfff0000f);

        assert2(count_leading_zero_bits(0), 32);
        assert2(count_leading_zero_bits(0x80000000), 0);
        assert2(count_leading_zero_bits(0x00008000), 16);
    };
};

class Number64 {

  public:
    int64_t high      = 0;
    int64_t low       = 0;
    int64_t overflow  = 0;
    int64_t _overflow = 0;

  public:
    Number64(int64_t _high, int64_t _low)
    {
        high = _high;
        low  = _low;
    }
    ~Number64()
    {
    }

    int64_t mul32(int64_t sub, int64_t obj)
    {
        int64_t sub_hi = BitOps::get_bits(sub, 31, 16);
        int64_t sub_lo = BitOps::get_bits(sub, 15, 0);
        int64_t obj_hi = BitOps::get_bits(obj, 31, 16);
        int64_t obj_lo = BitOps::get_bits(obj, 15, 0);
        int64_t ret    = sub_lo * obj_lo + _overflow;
        int64_t tmp_hi = sub_lo * obj_hi + sub_hi * obj_lo;
        ret += BitOps::get_bits(tmp_hi, 15, 0) * 0x10000;
        _overflow = BitOps::get_bits64(tmp_hi, 32, 16) + BitOps::get_bits64(ret, 51, 32);
        ret       = BitOps::get_bits64(ret, 31, 0);
        _overflow += sub_hi * obj_hi;
        return ret;
    }

    Number64 *mul_n(Number64 *obj)
    {
        _overflow   = 0;
        int64_t ret = mul32(low, obj->low);
        return new Number64(_overflow, ret);
    }

    Number64 *add_n(Number64 *obj)
    {
        int64_t tmp      = low + obj->low;
        int64_t overflow = BitOps::get_bits64(tmp, 51, 32);
        int64_t low      = BitOps::get_bits64(tmp, 31, 0);
        tmp              = high + obj->high + overflow;
        overflow         = BitOps::get_bits64(tmp, 51, 32);
        int64_t high     = BitOps::get_bits64(tmp, 31, 0);
        return new Number64(high, low);
    }

    Number64 *sub_n(Number64 *obj)
    {
        if (high > obj->high) {
            int64_t hi = high - obj->high;
            int64_t lo = low - obj->low;
            if (lo < 0) {
                hi -= 1;
                lo += 0x100000000;
            }
            return new Number64(hi, lo);
        } else if (high < obj->high) {
            int64_t hi = high - obj->high;
            int64_t lo = low - obj->low;
            if (hi < 0)
                hi += 0x100000000;
            if (lo < 0) {
                lo += 0x100000000;
                hi -= 1;
            }
            return new Number64(hi, lo);
        } else {
            int64_t lo = low - obj->low;
            return new Number64(0, lo);
        }
    }

    Number64 *lsl(int64_t amount)
    {
        uint64_t lowu = low;
        high          = lowu >> (32 - amount);
        low           = BitOps::lsl(BitOps::get_bits(low, 32 - amount - 1, 0), amount);
        return this;
    }

    Number64 *sign_extend_n(int64_t from, int64_t to)
    {
        if (BitOps::get_bit(low, from - 1, 0)) {
            if (to <= 32)
                return new Number64(high, BitOps::sign_extend(low, from, to));

            uint64_t low2 = (from == 32) ? low : BitOps::sign_extend(low, from, 32);
            int64_t  high = BitOps::sign_extend(1, 1, to - 32);
            return new Number64(high, low2);
        } else {
            return new Number64(high, low);
        }
    }

    bool is_zero()
    {
        return high == 0 && low == 0;
    }

    void assert_equal(Number64 *sub, Number64 *obj, std::string message)
    {
        if (!(sub->high == obj->high && sub->low == obj->low)) {
            assert(sub->high == obj->high && sub->low == obj->low);
        }
    }

    void TestNumber64()
    {
        auto ret  = new Number64(0, 0);
        auto arg1 = new Number64(0, 0);
        auto ans  = ret->mul_n(arg1);
        auto obj  = new Number64(0, 0);
        assert_equal(ans, obj, "0 * 0");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 1);
        arg1 = new Number64(0, 1);
        ans  = ret->mul_n(arg1);
        obj  = new Number64(0, 1);
        assert_equal(ans, obj, "1 * 1");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0x80000000);
        arg1 = new Number64(0, 2);
        ans  = ret->mul_n(arg1);
        obj  = new Number64(1, 0);
        assert_equal(ans, obj, "0x80000000 * 2");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0x80000000);
        arg1 = new Number64(0, 4);
        ans  = ret->mul_n(arg1);
        obj  = new Number64(2, 0);
        assert_equal(ans, obj, "0x80000000 * 4");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0x8fffffff);
        arg1 = new Number64(0, 0x10);
        ans  = ret->mul_n(arg1);
        obj  = new Number64(8, 0xfffffff0);
        assert_equal(ans, obj, "0x8fffffff * 0x10");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0x0fffffff);
        arg1 = new Number64(0, 0x0fffffff);
        ans  = ret->mul_n(arg1);
        obj  = new Number64(0x00ffffff, 0xe0000001);
        assert_equal(ans, obj, "0x0fffffff * 0x0fffffff");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0xffffffff);
        arg1 = new Number64(0, 0xffffffff);
        ans  = ret->mul_n(arg1);
        obj  = new Number64(0xfffffffe, 1);
        assert_equal(ans, obj, "0xffffffff * 0xffffffff");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        // add tests

        ret  = new Number64(0, 0);
        arg1 = new Number64(0, 0);
        ans  = ret->add_n(arg1);
        obj  = new Number64(0, 0);
        assert_equal(ans, obj, "0 + 0");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0);
        arg1 = new Number64(0, 1);
        ans  = ret->add_n(arg1);
        obj  = new Number64(0, 1);
        assert_equal(ans, obj, "0 + 0");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(1, 0);
        arg1 = new Number64(1, 0);
        ans  = ret->add_n(arg1);
        obj  = new Number64(2, 0);
        assert_equal(ans, obj, "1:0 + 1:0");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0xffffffff);
        arg1 = new Number64(0, 1);
        ans  = ret->add_n(arg1);
        obj  = new Number64(1, 0);
        assert_equal(ans, obj, "0:0xffffffff + 0:1");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0x1ffffffff);
        arg1 = new Number64(0, 1);
        ans  = ret->add_n(arg1);
        obj  = new Number64(2, 0);
        assert_equal(ans, obj, "0:0x1ffffffff + 0:1");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0xeeeeeeee);
        arg1 = new Number64(0, 0x11111111);
        ans  = ret->add_n(arg1);
        obj  = new Number64(0, 0xffffffff);
        assert_equal(ans, obj, "0:0xeeeeeeee + 0:0x11111111");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0xf, 0xf);
        arg1 = new Number64(0x1, 0x1);
        ans  = ret->add_n(arg1);
        obj  = new Number64(0x10, 0x10);
        assert_equal(ans, obj, "0xf:0xf + 0x1:0x1");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0xf, 0xffffffff);
        arg1 = new Number64(0x1, 0x1);
        ans  = ret->add_n(arg1);
        obj  = new Number64(0x11, 0);
        assert_equal(ans, obj, "0xf:0xfffffffff + 0x1:0x1");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        // sub tests

        ret  = new Number64(0, 1);
        arg1 = new Number64(0, 1);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(0, 0);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0);
        arg1 = new Number64(0, 1);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(0, -1);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(1, 0);
        arg1 = new Number64(0, 1);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(0, 0xffffffff);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(1, 0xf);
        arg1 = new Number64(0, 1);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(1, 0xe);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(1, 0xf);
        arg1 = new Number64(0, 0xf0000000);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(0, 0x1000000f);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0, 0);
        arg1 = new Number64(0xf, 0);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(0x100000000 - 0xf, 0);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        ret  = new Number64(0xe, 0);
        arg1 = new Number64(0xf, 0xf0000000);
        ans  = ret->sub_n(arg1);
        obj  = new Number64(0xfffffffe, 0x10000000);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete arg1;
        delete ans;

        // // lsl tests
        ret = new Number64(0, 1);
        ans = ret->lsl(1);
        obj = new Number64(0, 2);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;

        ret = new Number64(0, 0x80000000);
        ans = ret->lsl(1);
        obj = new Number64(1, 0);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;

        ret = new Number64(0, 0x80000000);
        ans = ret->lsl(16);
        obj = new Number64(0x8000, 0);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;

        ret = new Number64(0, 0x80000000);
        ans = ret->lsl(31);
        obj = new Number64(0x40000000, 0);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;

        // sign_extend tests
        ret = new Number64(0, 0x10000000);
        ans = ret->sign_extend_n(31, 32);
        obj = new Number64(0, 0x10000000);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete ans;

        ret = new Number64(0, 0x10000000);
        ans = ret->sign_extend_n(29, 32);
        obj = new Number64(0, 0xf0000000);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete ans;

        ret = new Number64(0, 0x10000000);
        ans = ret->sign_extend_n(29, 36);
        obj = new Number64(0xf, 0xf0000000);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete ans;

        ret = new Number64(0, 0x80000000);
        ans = ret->sign_extend_n(32, 64);
        obj = new Number64(0xffffffff, 0x80000000);
        assert_equal(ans, obj, "");
        delete ret;
        delete obj;
        delete ans;
    };
};

#endif