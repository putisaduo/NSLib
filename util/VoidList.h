#ifndef _NSLib_util_VoidList_
#define _NSLib_util_VoidList_

#include "StdHeader.h"
#include "util/Misc.h"

using namespace std;

namespace NSLib{ namespace util{

	
	template<typename _kt=void*> class VoidList{
	private:
		vector<_kt> keys;
		bool dv;
		int dvDelType;
	public:

		void setDoDelete(const int deleteType){
			dv = true;
			dvDelType = deleteType;
		}
		VoidList():
			dv(false),
			dvDelType(DELETE_TYPE_DELETE)
		{
		}
		virtual ~VoidList(){
			clear();
		}
		VoidList ( const bool deleteValue, const int valueDelType ):
			dv(deleteValue),
			dvDelType(valueDelType)
		{
		}

		void put( _kt k ){		
			keys.push_back(k);
		}
		uint size() const{
			return keys.size();
		}
		_kt at(int i){
			return keys[i];
		}
		_kt at(int i) const{
			return keys[i];
		}
		_kt operator[](int i){
			return keys[i];
		}
		_kt operator[](int i) const{
			return keys[i];
		}
		typename vector<_kt>::const_iterator begin() const{
			return keys.begin();
		}
		typename vector<_kt>::const_iterator end() const{
			return keys.end();
		}
		typename vector<_kt>::iterator begin(){
			return keys.begin();
		}
		typename vector<_kt>::iterator end(){
			return keys.end();
		}

		void pop_back(){
			if ( dv ){
				if ( dvDelType == DELETE_TYPE_DELETE )
					delete keys[keys.size()-1];
				else if ( dvDelType == DELETE_TYPE_DELETE_ARRAY )
					delete[] keys[keys.size()-1];
			}
			keys.pop_back();
		}
		void push_back( _kt k ){
			keys.push_back( k );
		}
		void push_front(_kt k){
			keys.insert(keys.begin(),k);
		}

		void clear(){
			if ( dv ){
				for ( uint i=0;i<keys.size();i++ ){
					if ( dvDelType == DELETE_TYPE_DELETE )
						delete keys[i];
					else if ( dvDelType == DELETE_TYPE_DELETE_ARRAY )
						delete[] keys[i];
				}

			}
			keys.clear();
		}

		void remove(int i){
			if ( dv ){
				if ( dvDelType == DELETE_TYPE_DELETE )
					delete keys[i];
				else if ( dvDelType == DELETE_TYPE_DELETE_ARRAY )
					delete[] keys[i];
			}
			keys.erase( keys.begin()+i );
		}
	};
}}
#endif
