#ifndef INCLUDE_CA_INT_H_
#define INCLUDE_CA_INT_H_

#define MAX(a,b) ((((a)>(b)) ? a : b)



#ifdef __VIVADO__

//#ifdef INCLUDE_CA_INT_H_

#include <ap_int.h>

//**************************************************************************

//Placeholder for type ca_uint (used in set_slc function)
template<int size>
 struct ca_uint;

//**************************************************************************

template<int size>
class ca_int : public ap_int<size>{
public:

	 ca_int() : ap_int<size>(){}

	 template <int sourceSize>
	 inline ca_int(ap_int<sourceSize> val) : ap_int<size>(val){}

	 template <int sourceSize>
	 inline ca_int(ap_uint<sourceSize> val) : ap_int<size>(val){}

	 template <int sliceSize>
	 void set_slc(int lsb, ca_int<sliceSize> value){
		 ap_int<size>::range(lsb + sliceSize - 1, lsb) = value;
	 }

	 template <int sliceSize>
	 void set_slc(int lsb, ca_uint<sliceSize> value){
		 ap_int<size>::range(lsb + sliceSize - 1, lsb) = value;
	 }

	 template <int sliceSize>
	 ca_int<sliceSize> slc(int lsb){
		 return ca_int<sliceSize>(ap_int<sliceSize>(ap_int<size>::range(lsb + sliceSize - 1, lsb)));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_int<sliceSize> slc(ca_uint<lsbSize> lsb){
		 return ca_int<sliceSize>(ap_int<sliceSize>(ap_int<size>::range(lsb + sliceSize - 1, lsb)));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_int<sliceSize> slc(ca_int<lsbSize> lsb){
		 return ca_int<sliceSize>(ap_int<sliceSize>(ap_int<size>::range(lsb + sliceSize - 1, lsb)));
	 }


	#define CTOR(TYPE) \
	INLINE ca_int(TYPE val) : ap_int<size>(val) {}
	CTOR(bool)
	CTOR(char)
	CTOR(signed char)
	CTOR(unsigned char)
	CTOR(short)
	CTOR(unsigned short)
	CTOR(int)
	CTOR(unsigned int)
	CTOR(long)
	CTOR(unsigned long)
	CTOR(ap_slong)
	CTOR(ap_ulong)
	#undef CTOR
	ca_int(double val) : ap_int<size>(val) {}
	ca_int(float val) : ap_int<size>(val) {}
	ca_int(half val) : ap_int<size>(val) {}

//	#define CTOR(TYPE) \
//		INLINE operator TYPE() const { return ap_int<size>::operator TYPE(); }
//	CTOR(bool)
//	CTOR(char)
//	CTOR(signed char)
//	CTOR(unsigned char)
//	CTOR(short)
//	CTOR(unsigned short)
//	CTOR(int)
//	CTOR(unsigned int)
//	CTOR(long)
//	CTOR(unsigned long)
//	CTOR(ap_slong)
//	CTOR(ap_ulong)
//	#undef CTOR

 };

//**************************************************************************

template<int size>
class ca_uint : public ap_uint<size>{
public:

	 ca_uint() : ap_uint<size>(){}

	 template <int sourceSize>
	 inline ca_uint(ap_int<sourceSize> val) : ap_uint<size>(val){}

	 template <int sourceSize>
	 inline ca_uint(ap_uint<sourceSize> val) : ap_uint<size>(val){}

	 template <int sliceSize>
	 void set_slc(int lsb, ca_int<sliceSize> value){
		 ap_uint<size>::range(lsb + sliceSize - 1, lsb) = value;
	 }

	 template <int sliceSize>
	 void set_slc(int lsb, ca_uint<sliceSize> value){
		 ap_uint<size>::range(lsb + sliceSize - 1, lsb) = value;
	 }

	 template <int sliceSize>
	 ca_uint<sliceSize> slc(int lsb){
		 return ca_uint<sliceSize>(ap_uint<sliceSize>(ap_uint<size>::range(lsb + sliceSize - 1, lsb)));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_uint<sliceSize> slc(ca_uint<lsbSize> lsb){
		 return ca_uint<sliceSize>(ap_uint<sliceSize>(ap_uint<size>::range(lsb + sliceSize - 1, lsb)));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_uint<sliceSize> slc(ca_int<lsbSize> lsb){
		 return ca_uint<sliceSize>(ap_uint<sliceSize>(ap_uint<size>::range(lsb + sliceSize - 1, lsb)));
	 }

	#define CTOR(TYPE) \
	INLINE ca_uint(TYPE val) : ap_uint<size>(val) {}
	CTOR(bool)
	CTOR(char)
	CTOR(signed char)
	CTOR(unsigned char)
	CTOR(short)
	CTOR(unsigned short)
	CTOR(int)
	CTOR(unsigned int)
	CTOR(long)
	CTOR(unsigned long)
	CTOR(ap_slong)
	CTOR(ap_ulong)
	#undef CTOR
	ca_uint(double val) : ap_uint<size>(val) {}
	ca_uint(float val) : ap_uint<size>(val) {}
	ca_uint(half val) : ap_uint<size>(val) {}


//	#define CTOR(TYPE) \
//		INLINE operator TYPE() const { return ap_uint<size>::operator TYPE(); }
//	CTOR(bool)
//	CTOR(char)
//	CTOR(signed char)
//	CTOR(unsigned char)
//	CTOR(short)
//	CTOR(unsigned short)
//	CTOR(int)
//	CTOR(unsigned int)
//	CTOR(long)
//	CTOR(unsigned long)
//	CTOR(ap_slong)
//	CTOR(ap_ulong)
//	#undef CTOR

 };





#else

//**************************************************************************
//**************************************************************************
//**************************************************************************



#include <ac_int.h>

//Placeholder for type ca_uint (used in set_slc function)
template<int size>
 struct ca_uint;

//**************************************************************************

//Struct definition for ca_int
template<int size>
 class ca_int{
 public:
	ac_int<size, true> value;

	 inline ca_int(ac_int<size, true> val) : value(val){}
	 inline operator ac_int<size, true>(){return value;};

	 ca_int() : value() {}


	 template <int sourceSize, bool Sign>
	 inline ca_int(ac_int<sourceSize, Sign> val) : value(val){}

	 template <int sourceSize>
	 inline ca_int(ac_int<sourceSize, false> val) : value(val){}

	 template <int sourceSize>
	 inline ca_int(ca_uint<sourceSize> val) : value(val.value) {};

	 template <int sourceSize>
	 inline ca_int(ca_int<sourceSize> val) : value(val.value) {};


	 template <int sliceSize>
	 void set_slc(int lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize>
	 void set_slc(int lsb, ca_uint<sliceSize> val){
		value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_uint<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_uint<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_uint<sliceSize> val){
		 value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_int<sliceSize> val){
		value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize, int lsbSize, bool S1, bool S2>
	 void set_slc(ac_int<lsbSize, S1> lsb, ac_int<sliceSize, S2> val){
		value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize>
	 ca_int<sliceSize> slc(const int lsb){
		 return ca_int<sliceSize>(value.slc<sliceSize>(lsb));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_int<sliceSize> slc(const ca_uint<lsbSize> lsb){
		 return ca_int<sliceSize>(value.slc<sliceSize>(lsb.value));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_int<sliceSize> slc(const ca_int<lsbSize> lsb){
		 return ca_int<sliceSize>(value.slc<sliceSize>(lsb.value));
	 }

	 //Would like to get rid of this one
	 template <int sliceSize, int lsbSize, bool S>
	 ca_int<sliceSize> slc(const ac_int<lsbSize, S> lsb){
		 return ca_int<sliceSize>(value.slc<sliceSize>(lsb));
	 }

	 // Assignation

	 ca_int<size> operator=(const long unsigned int val){
		 value = val;
		 return this;
	 }

	 ca_int<size> operator=(const long int val){
		 value = val;
		 return this;	 }

	 ca_int<size> operator=(const int val){
		 value = val;
		 return this;
	 }

	 ca_int<size> operator=(const unsigned int val){
		 value = val;
		 return this;
	 }

	 template <int otherSize>
	 ca_int<size> operator=(const ca_uint<otherSize> val){
		 value = val.value;
		 return this;
	 }

	 template <int otherSize>
	 ca_int<size> operator=(const ca_int<otherSize> val){
		 value = val.value;
		 return this;
	 }

	 template <int otherSize, bool sign>
	 ca_int<size> operator=(ac_int<otherSize, sign> val){
		 value = val;
		 return this;
	 }

	 //************** Operator Shifts

	ca_int<size> operator>>(int val){
		ca_int<size> result = value >> val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	ca_int<size> operator>>(ac_int<otherSize, sign> val){
		 ca_int<size> result = value >> val;
		 return result;
	 }

	 template <int otherSize>
	ca_int<size> operator>>(ca_int<otherSize> val){
		 ca_int<size> result = value >> val.value;
		 return result;
	 }

	 template <int otherSize>
	ca_int<size> operator>>(ca_uint<otherSize> val){
		 ca_int<size> result = value >> val.value;
		 return result;
	 }

	 //************** Operator Shifts <<

	ca_int<size> operator<<(int val){
		ca_int<size> result = value << val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	ca_int<size> operator<<(ac_int<otherSize, sign> val){
		 ca_int<size> result = value << val;
		 return result;
	 }

	 template <int otherSize>
	ca_int<size> operator<<(ca_int<otherSize> val){
		 ca_int<size> result = value << val.value;
		 return result;
	 }

	 template <int otherSize>
	ca_int<size> operator<<(ca_uint<otherSize> val){
		 ca_int<size> result = value << val.value;
		 return result;
	 }

	 //************** Operator +

	ca_int<size+1> operator+(int val){
		ca_int<size+1> result = value + val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	ca_int<(size > otherSize)? size + 1 : otherSize + 1> operator+(ac_int<otherSize, sign> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value + val;
		 return result;
	 }

	 template <int otherSize>
	ca_int<(size > otherSize)? size + 1 : otherSize + 1> operator+(ca_int<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value + val.value;
		 return result;
	 }

	 template <int otherSize>
	ca_int<(size > otherSize)? size + 1 : otherSize + 1> operator+(ca_uint<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value + val.value;
		 return result;
	 }

	 //************** Operator -

	ca_int<size+1> operator-(int val){
		ca_int<size+1> result = value - val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	ca_int<(size > otherSize)? size + 1 : otherSize + 1> operator-(ac_int<otherSize, sign> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value - val;
		 return result;
	 }

	 template <int otherSize>
	ca_int<(size > otherSize)? size + 1 : otherSize + 1> operator-(ca_int<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value - val.value;
		 return result;
	 }

	 template <int otherSize>
	ca_int<(size > otherSize)? size + 1 : otherSize + 1> operator-(ca_uint<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value - val.value;
		 return result;
	 }

	 //************** Operator *

	ca_int<size+32> operator*(int val){
		ca_int<size+32> result = value * val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	ca_int<size+otherSize> operator*(ac_int<otherSize, sign> val){
		 ca_int<size+otherSize> result = value * val;
		 return result;
	 }

	 template <int otherSize>
	ca_int<size+otherSize> operator*(ca_int<otherSize> val){
		 ca_int<size+otherSize> result = value * val.value;
		 return result;
	 }

	 template <int otherSize>
	ca_int<size+otherSize> operator*(ca_uint<otherSize> val){
		 ca_int<size+otherSize> result = value * val.value;
		 return result;
	 }

	 //============================ Operator ==

	 ca_int<1> operator==(int val){
		 ca_int<1> result = value == val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_int<1> operator==(ac_int<otherSize, sign> val){
		 ca_int<1> result = value == val;
		 return result;
	 }

	 template <int otherSize>
	 ca_int<1> operator==(ca_int<otherSize> val){
		 ca_int<1> result = value == val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_int<1> operator==(ca_uint<otherSize> val){
		 ca_int<1> result = value == val.value;
		 return result;
	 }

	 //********************************* Operator <

	 bool operator<(int val){
		 return value < val;
	 }

	 template <int otherSize, bool sign>
	 bool operator<(ac_int<otherSize, sign> val){
		 return value < val;
	 }

	 template <int otherSize>
	 bool operator<(ca_int<otherSize> val){
		 return value < val;
	 }

	 template <int otherSize>
	 bool operator<(ca_uint<otherSize> val){
		 return value < val;
	 }

	 //********************************* Operator >

	 bool operator>(int val){
		 return value > val;
	 }

	 template <int otherSize, bool sign>
	 bool operator>(ac_int<otherSize, sign> val){
		 return value > val;
	 }

	 template <int otherSize>
	 bool operator>(ca_int<otherSize> val){
		 return value > val;
	 }

	 template <int otherSize>
	 bool operator>(ca_uint<otherSize> val){
		 return value > val;
	 }

	 //********************************* Operator []

	 ca_int<1> operator[](int val){
		 return ca_int<1>(value.slc<1>(val));
	 }

	 //********************************* Operator ++ / --

	 ca_int<size> operator ++(int){
		 value++;
		 return this;
	 }

	 ca_int<size> operator --(int){
		 value--;
		 return this;
	 }

	 ca_int<size> operator +=(int val){
		 value += val;
		 return this;
	 }

	 ca_int<size> operator -=(int val){
		 value -= val;
		 return this;
	 }

	 //************************************ Implicit and explicit conversions

	 inline operator size_t() const {return value.to_uint64();}
	 inline unsigned int to_uint() const {return value.to_uint();}
	 inline int to_int() const {return value.to_int();}



	#define CTOR(TYPE) \
		inline ca_int(TYPE val) : value(val) {}
	CTOR(bool)
	CTOR(char)
	CTOR(signed char)
	CTOR(unsigned char)
	CTOR(short)
	CTOR(unsigned short)
	CTOR(int)
	CTOR(unsigned int)
	CTOR(long)
	CTOR(unsigned long)
	#undef CTOR
	ca_int(double val) : value(val) {}
	ca_int(float val) : value(val) {}

 };

//**************************************************************************
//Struct definition for ca_uint
template<int size>
 class ca_uint{
 public:
	ac_int<size, false> value;

	 inline ca_uint(ac_int<size, true> val) : value(val){}
	 inline operator ac_int<size, true>(){return value;};

	 ca_uint() : value(){}

	 template <int sourceSize, bool sign>
	 inline ca_uint(ac_int<sourceSize, sign> val) : value(val){}

	 template <int sourceSize>
	 inline ca_uint(ca_uint<sourceSize> val) : value(val.value) {};

	 template <int sourceSize>
	 inline ca_uint(ca_int<sourceSize> val) : value(val.value) {};

	 template <int sliceSize>
	 void set_slc(int lsb, ca_uint<sliceSize> val){
		 value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize>
	 void set_slc(int lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_uint<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_uint<sliceSize> val){
		 fprintf(stderr, "setting slice at %d\n", lsb.to_uint());
		 value.set_slc(lsb.value, val.value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_int<sliceSize> val){
		 value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_uint<sliceSize> val){
		 value.set_slc(lsb, val.value);
	 }

	 template <int sliceSize, int lsbSize, bool S1, bool S2>
	 void set_slc(ac_int<lsbSize, S1> lsb, ac_int<sliceSize, S2> val){
		 value.set_slc(lsb, val.value);
	 }

	 //*******************************************************************

	 template <int sliceSize>
	 ca_uint<sliceSize> slc(const int lsb){
		 return ca_uint<sliceSize>(value.slc<sliceSize>(lsb));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_uint<sliceSize> slc(const ca_uint<lsbSize> lsb){
		 return ca_uint<sliceSize>(value.slc<sliceSize>(lsb.value));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_uint<sliceSize> slc(const ca_int<lsbSize> lsb){
		 return ca_uint<sliceSize>(value.slc<sliceSize>(lsb.value));
	 }

	 //Would like to get rid of this one
	 template <int sliceSize, int lsbSize, bool S>
	 ca_uint<sliceSize> slc(const ac_int<lsbSize, S> lsb){
		 return ca_uint<sliceSize>(value.slc<sliceSize>(lsb.value));
	 }

	 // Assignation

	 ca_uint<size> operator=(const long unsigned int val){
		 value = val;
		 return this;
	 }

	 ca_uint<size> operator=(const long int val){
		 value = val;
		 return this;
	 }

	 ca_uint<size> operator=(const int val){
		 value = val;
		 return this;
	 }

	 ca_uint<size> operator=(const unsigned int val){
		 value = val;
		 return this;
	 }

	 template <int otherSize>
	 ca_uint<size> operator=(const ca_uint<otherSize> val){
		 value = val.value;
		 return this;
	 }

	 template <int otherSize>
	 ca_uint<size> operator=(const ca_int<otherSize> val){
		 value = val.value;
		 return this;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<size> operator=(ac_int<otherSize, sign> val){
		 value = val;
		 return this;
	 }


	 //************** Operator Shifts

	 ca_uint<size> operator>>(int val){
		 ca_int<size> result = value >> val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<size> operator>>(ac_int<otherSize, sign> val){
		 ca_int<size> result = value >> val;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<size> operator>>(ca_int<otherSize> val){
		 ca_int<size> result = value >> val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<size> operator>>(ca_uint<otherSize> val){
		 ca_int<size> result = value >> val.value;
		 return result;
	 }

	 //************** Operator Shifts <<

	 ca_uint<size> operator<<(int val){
		 ca_int<size> result = value << val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<size> operator<<(ac_int<otherSize, sign> val){
		 ca_int<size> result = value << val;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<size> operator<<(ca_int<otherSize> val){
		 ca_int<size> result = value << val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<size> operator<<(ca_uint<otherSize> val){
		 ca_int<size> result = value << val.value;
		 return result;
	 }

	 //************** Operator +

	 ca_uint<size+1> operator+(int val){
		 ca_int<size+1> result = value + val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<(size > otherSize)? size + 1 : otherSize + 1> operator+(ac_int<otherSize, sign> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value + val;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<(size > otherSize)? size + 1 : otherSize + 1> operator+(ca_int<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value + val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<(size > otherSize)? size + 1 : otherSize + 1> operator+(ca_uint<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value + val.value;
		 return result;
	 }

	 //************** Operator -

	 ca_uint<size+1> operator-(int val){
		 ca_int<size+1> result = value - val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<(size > otherSize)? size + 1 : otherSize + 1> operator-(ac_int<otherSize, sign> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value - val;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<(size > otherSize)? size + 1 : otherSize + 1> operator-(ca_int<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value - val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<(size > otherSize)? size + 1 : otherSize + 1> operator-(ca_uint<otherSize> val){
		 ca_int<(size > otherSize)? size + 1 : otherSize + 1> result = value - val.value;
		 return result;
	 }

	 //************** Operator *

	 ca_uint<size+32> operator*(int val){
		 ca_int<size+32> result = value * val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<size+otherSize> operator*(ac_int<otherSize, sign> val){
		 ca_int<size+otherSize> result = value * val;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<size+otherSize> operator*(ca_int<otherSize> val){
		 ca_int<size+otherSize> result = value * val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<size+otherSize> operator*(ca_uint<otherSize> val){
		 ca_int<size+otherSize> result = value * val.value;
		 return result;
	 }

	 //******************************** Operator ==

	 ca_uint<1> operator==(int val){
		 ca_int<1> result = value == val;
		 return result;
	 }

	 template <int otherSize, bool sign>
	 ca_uint<1> operator==(ac_int<otherSize, sign> val){
		 ca_int<1> result = value == val;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<1> operator==(ca_int<otherSize> val){
		 ca_int<1> result = value == val.value;
		 return result;
	 }

	 template <int otherSize>
	 ca_uint<1> operator==(ca_uint<otherSize> val){
		 ca_int<1> result = value == val.value;
		 return result;
	 }

	 //********************************* Operator <

	 template <int otherSize, bool sign>
	 bool operator<(ac_int<otherSize, sign> val){
		 return value < val;
	 }

	 template <int otherSize>
	 bool operator<(ca_int<otherSize> val){
		 return value < val;
	 }

	 template <int otherSize>
	 bool operator<(ca_uint<otherSize> val){
		 return value < val;
	 }

	 //********************************* Operator >

	 template <int otherSize, bool sign>
	 bool operator>(ac_int<otherSize, sign> val){
		 return value > val;
	 }

	 template <int otherSize>
	 bool operator>(ca_int<otherSize> val){
		 return value > val;
	 }

	 template <int otherSize>
	 bool operator>(ca_uint<otherSize> val){
		 return value > val;
	 }

	 //********************************* Operator []

	 ca_uint<1> operator[](int val){
		 return ca_uint<1>(value.slc<1>(val));
	 }

	 //********************************* Operator ++ and --

	 ca_uint<size+1> operator ++(int){
		 value++;
		 return this;
	 }

	 ca_uint<size+1> operator --(int){
		 value--;
		 return this;
	 }

	 ca_uint<size+1> operator +=(int val){
		 value += val;
		 return this;
	 }

	 ca_uint<size+1> operator -=(int val){
		 value -= val;
		 return this;
	 }

	 //*********************************** Implicit and explicit conversions

	 inline operator size_t() const {return value.to_uint64();}
	 inline unsigned int to_uint() const {return value.to_uint();}
	 inline int to_int() const {return value.to_int();}



	#define CTOR(TYPE) \
		inline ca_uint(TYPE val) : value(val) {}
	CTOR(bool)
	CTOR(char)
	CTOR(signed char)
	CTOR(unsigned char)
	CTOR(short)
	CTOR(unsigned short)
	CTOR(int)
	CTOR(unsigned int)
	CTOR(long)
	CTOR(unsigned long)
	#undef CTOR
	ca_uint(double val) : value(val) {}
	ca_uint(float val) : value(val) {}

 };


#endif




#endif /* INCLUDE_CA_INT_H_ */
