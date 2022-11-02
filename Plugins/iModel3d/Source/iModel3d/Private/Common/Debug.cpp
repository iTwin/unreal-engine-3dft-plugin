#include "Debug.h"

#include "CoreMinimal.h"

#include <iostream>
#include <sstream>

class LStream : public std::stringbuf {
protected:
	int sync() {
		UE_LOG(LogTemp, Log, TEXT("%s"), *FString(str().c_str()));
		str("");
		return std::stringbuf::sync();
	}
};

LStream _Stream;

void RedirectStdOutput()
{
	std::cout.rdbuf(&_Stream);
}
