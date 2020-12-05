#include <stdint.h>

#define F (1<< 14)             //fixed point 1
#define INT_MAX ((1<< 31)-1)
#define INT_MIN (-(1<< 31))

int int_to_fp(int n);
int fp_to_int_round(int x);
int fp_to_int(int x);
int fp_add_fp(int x,int y);
int fp_add_int(int x, int n);
int fp_sub_int(int x, int n);
int fp_mult_fp(int x, int y);
int fp_mult_int(int x,int y);
int fp_div_fp(int x, int y);
int fp_div_int(int x, int n);


/* Transforms Integer n into Fixed point. */
int int_to_fp(int n)
{
    return n*F;
}

/* Transforms Fixed point X into Integer rounding up to nearest integer.*/
int fp_to_int_round(int x)
{
    return x/F;
}

/* Transforms Fixed point X into Integer rounding down to nearest integer.*/
int fp_to_int(int x)
{
    return (x>=0)? (x+F/2)/F : (x-F/2)/F;
}

/* Adds two fixed points.*/
int fp_add_fp(int x,int y)
{
    return x+y;
}

/* Adds one fixed point and one integer.*/
int fp_add_int(int x, int n)
{
    return x+n*F;
}

/* Subtracts one fixed point X by another fp.*/
int fp_sub_fp(int x, int y)
{
    return x-y;
}

/* Subtracts fixed point X by integer n.*/
int fp_sub_int(int x, int n)
{
    return x-n*F;
}

/* Multiplies x by y (two fixed points).*/
int fp_mult_fp(int x, int y)
{
    return ((int64_t)x)*y/F;
}

/* Multiplies x by n (one fixed point & integer).*/ 
int fp_mult_int(int x,int n)
{
    return x*n;
}

/* Divides x by y (two fixed points).*/
int fp_div_fp(int x, int y)
{
    return ((int64_t)x)*F/y;
}

/* Divides x by n (one fixed point & integer).*/ 
int fp_div_int(int x, int n)
{
    return x/n;
}

