/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

//
// pvector.cpp
//
// Test vector operations
//
#include <utilib_config.h>

#define USING_STL

#include <utilib/std_headers.h>
#include <utilib/pvector.h>
#include <utilib/IntVector.h>
#include <utilib/BitArray.h>
#include <utilib/CharString.h>
#include <utilib/CommonIO.h>
#include <utilib/Num2DArray.h>

using namespace utilib;

int test_pvector(int,char**)
{
pvector<int> a(10);

for (int i=0; i<10; i++) a[i] = i;
ucout << "a= " << a << " " << std::endl;

a.resize(15);
ucout << "a= " << a << " " << std::endl;

a.resize(5);
ucout << "a= " << a << " " << std::endl;

{
pvector<int> A;
A = a;
ucout << "a= " << a << " " << std::endl;
ucout << "A= " << A << " " << std::endl;

a[0] = -1;
ucout << "a= " << a << " " << std::endl;
ucout << "A= " << A << " " << std::endl;

//A &= a;
A[0] = 10;
ucout << "a= " << a << " " << std::endl;
ucout << "A= " << A << " " << std::endl;

{
pvector<int> A2;
//A2 &= A;
ucout << "a= " << a << " " << std::endl;
ucout << "A= " << A << " " << std::endl;
ucout << "A2= " << A2 << " " << std::endl;

a.resize(8);
ucout << "a= " << a << " " << std::endl;
ucout << "A= " << A << " " << std::endl;

//int bar=0;
}

ucout << "a= " << a << " " << std::endl;
ucout << "A= " << A << " " << std::endl;

//int foo=01;
}

ucout << "a= " << a << " " << std::endl;

a -= 1;
ucout << "a-=1: " << a << " " << std::endl;
a = a.operator-();
ucout << "a=-a: " << a << " " << std::endl;

pvector<int> AA;
AA = a;
AA *= 2;
ucout << "AA: " << AA << " " << std::endl;
a += AA;
ucout << "a+=AA: " << a << " " << std::endl;

ucout << std::endl;
ucout << "a= " << a << " " << std::endl;
std::ofstream ofstr("testfile");
ofstr << a;
ofstr.close();
a = 0;
std::ifstream ifstr("testfile");
ifstr >> a;
ifstr.close();
ucout << "a= " << a << " " << std::endl;

a.push_back(a);
ucout << "a= " << a << " " << std::endl;
ucout << std::endl;




NumArray<int> foo1;
NumArray<int> foo2;
//foo1 &= foo2;
foo1.resize(3);
foo1 <<  1;
ucout << "These two arrays should be equal." << std::endl;
ucout << "foo1= " << foo1 << " " << std::endl;
ucout << "foo2= " << foo2 << " " << std::endl;


NumArray<int> bar1(10);
NumArray<int> bar2(10);
bar1 << 1;
bar2 << 2;
bar2[2]=0;
bar1.set_subvec(3,4,bar2,2);
ucout << "This array should contain a subsequence of 2 indeces, beginning with a zero." << std::endl;
ucout << "bar1= " << bar1 << " " << std::endl;
ucout << "bar2= " << bar2 << " " << std::endl;

BitArray b(10);
BitArray B;

{for (int i=0; i<10; i++) b.set(i);}
ucout << "b= " << b << std::endl;

b.resize(15);
ucout << "b= " << b << std::endl;

b.resize(5);
ucout << "b= " << b << std::endl;

B = b;
ucout << "b= " << b << std::endl;
ucout << "B= " << B << std::endl;

b.reset(0);
ucout << "b= " << b << std::endl;
ucout << "B= " << B << std::endl;

ucout << std::endl;


CharString str(10);
CharString STR(10);

{for (int i=0; i<10; i++) str[i] = (char)((int)'a' + i);}
ucout << "str= " << str << std::endl;

str.resize(15);
ucout << "str= " << str << std::endl;

str.resize(5);
ucout << "str= " << str << std::endl;

STR = str;
ucout << "str= " << str << std::endl;
ucout << "STR= " << STR << std::endl;

str[0] = 'A';
ucout << "str= " << str << std::endl;
ucout << "STR= " << STR << std::endl;

{
Num2DArray<double> tmp2(11,2);
tmp2 << 0.0;
ucout << tmp2 << std::endl;
}


return 0;
}
