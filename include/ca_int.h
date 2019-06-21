#ifndef INCLUDE_CA_INT_H_
#define INCLUDE_CA_INT_H_



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
 class ca_int : public ac_int<size, true>{
 public:

	 ca_int() : ac_int<size, true>(){}

	 template <int sourceSize>
	 inline ca_int(ac_int<sourceSize, true> val) : ac_int<size, true>(val){}

	 template <int sourceSize>
	 inline ca_int(ac_int<sourceSize, false> val) : ac_int<size, true>(val){}


	 template <int sliceSize>
	 void set_slc(int lsb, ca_int<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize>
	 void set_slc(int lsb, ca_uint<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_uint<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_int<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_uint<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_int<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_uint<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_int<sliceSize> value){
		 ac_int<size, true>::set_slc(lsb, value);
	 }

	 template <int sliceSize>
	 ca_int<sliceSize> slc(const int lsb){
		 return ca_int<sliceSize>(ac_int<size, true>::template slc<sliceSize>(lsb));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_int<sliceSize> slc(const ca_uint<lsbSize> lsb){
		 return ca_int<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_int<sliceSize> slc(const ca_int<lsbSize> lsb){
		 return ca_int<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 //Would like to get rid of this one
	 template <int sliceSize, int lsbSize, bool S>
	 ca_int<sliceSize> slc(const ac_int<lsbSize, S> lsb){
		 return ca_int<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 // Assignation

	 ca_int<size> operator=(const long unsigned int val){
		 return ac_int<size, true>::operator=(val);
	 }

	 ca_int<size> operator=(const long int val){
		 return ac_int<size, true>::operator=(val);
	 }

	 ca_int<size> operator=(const int val){
		 return ac_int<size, true>::operator=(val);
	 }

	 ca_int<size> operator=(const unsigned int val){
		 return ac_int<size, true>::operator=(val);
	 }

	 template <int otherSize>
	 ca_int<size> operator=(const ca_uint<otherSize> val){
		 return ac_int<size, true>::operator=(ac_int<otherSize, false>(val));
	 }

	 template <int otherSize>
	 ca_int<size> operator=(const ca_int<otherSize> val){
		 return ac_int<size, true>::operator=(ac_int<otherSize, true>(val));
	 }

	 template <int otherSize, bool sign>
	 ca_int<size> operator=(const ac_int<otherSize, sign> val){
		 return ac_int<size, true>::operator=(ac_int<otherSize, sign>(val));
	 }


	#define CTOR(TYPE) \
		inline ca_int(TYPE val) : ac_int<size, true>(val) {}
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
	ca_int(double val) : ac_int<size, true>(val) {}
	ca_int(float val) : ac_int<size, true>(val) {}

 };

//**************************************************************************
//Struct definition for ca_uint
template<int size>
 class ca_uint : public ac_int<size, false>{
 public:

	 ca_uint() : ac_int<size, false>(){}

	 template <int sourceSize>
	 inline ca_uint(ac_int<sourceSize, true> val) : ac_int<size, false>(val){}

	 template <int sourceSize>
	 inline ca_uint(ac_int<sourceSize, false> val) : ac_int<size, false>(val){}



	 template <int sliceSize>
	 void set_slc(int lsb, ca_uint<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize>
	 void set_slc(int lsb, ca_int<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_int<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_int<lsbSize> lsb, ca_uint<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_int<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize>
	 void set_slc(ca_uint<lsbSize> lsb, ca_uint<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_int<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }

	 template <int sliceSize, int lsbSize, bool S>
	 void set_slc(ac_int<lsbSize, S> lsb, ca_uint<sliceSize> value){
		 ac_int<size, false>::set_slc(lsb, value);
	 }


	 template <int sliceSize>
	 ca_uint<sliceSize> slc(const int lsb){
		 return ca_uint<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_uint<sliceSize> slc(const ca_uint<lsbSize> lsb){
		 return ca_uint<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 template <int sliceSize, int lsbSize>
	 ca_uint<sliceSize> slc(const ca_int<lsbSize> lsb){
		 return ca_uint<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 //Would like to get rid of this one
	 template <int sliceSize, int lsbSize, bool S>
	 ca_uint<sliceSize> slc(const ac_int<lsbSize, S> lsb){
		 return ca_uint<sliceSize>(ac_int<size, false>::template slc<sliceSize>(lsb));
	 }

	 // Assignation

	 ca_uint<size> operator=(const long unsigned int val){
		 return ac_int<size, false>::operator=(val);
	 }

	 ca_uint<size> operator=(const long int val){
		 return ac_int<size, false>::operator=(val);
	 }

	 ca_uint<size> operator=(const int val){
		 return ac_int<size, false>::operator=(val);
	 }

	 ca_uint<size> operator=(const unsigned int val){
		 return ac_int<size, false>::operator=(val);
	 }

	 template <int otherSize>
	 ca_uint<size> operator=(const ca_uint<otherSize> val){
		 return ac_int<size, false>::operator=(ac_int<otherSize, false>(val));
	 }

	 template <int otherSize>
	 ca_uint<size> operator=(const ca_int<otherSize> val){
		 return ac_int<size, false>::operator=(ac_int<otherSize, true>(val));
	 }

	 template <int otherSize, bool sign>
	 ca_uint<size> operator=(const ac_int<otherSize, sign> val){
		 return ac_int<size, false>::operator=(ac_int<otherSize, sign>(val));
	 }

	#define CTOR(TYPE) \
		inline ca_uint(TYPE val) : ac_int<size, false>(val) {}
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
	ca_uint(double val) : ac_int<size, false>(val) {}
	ca_uint(float val) : ac_int<size, false>(val) {}

 };

#endif




#endif /* INCLUDE_CA_INT_H_ */
