// Rearranging moves as a function of loading rate and target size
//

#include "stdafx.h"
#include <sstream>
#include <iostream>

class myStream : public std::stringstream {};
class classB {
	public:
		explicit classB (int num) { }
		friend myStream& operator<<(myStream& ss, const classB& objB);
};
myStream& operator<<(myStream& ss, const classB& objB){
	ss << "text from operator overload...\n";
	return ss;
}
int main (){
	myStream stream;
	classB objB(5);
	stream << objB; // Works!
	stream << "stringtest";
	stream << 5; // ERROR: More than one operator matches these operands...
	std::cout << stream.str ();
	return 0;
}


/*
class classB {
public:
	classB (int num) { }
};
class myStream : public std::stringstream {
public:
	using std::stringstream::operator<<;
	myStream& operator<<(const classB& objB)
	{
		std::stringstream::operator<<("test"); 
		return *this;
	}
};
int main () {
	myStream stream;
	classB objB (5);
	stream << "test"; // ERROR: More than one operator matches these operands...
	stream << objB; // Works!
	stream << 5; 
	return 0;
}*/